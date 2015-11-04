/*
  Copyright 2015 Concurrent Computer Corporation

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "helper.h"
#include "msem.h"

/* Thin layer over POSIX Cond var & Semaphore */

CCUR_PROTECTED(tresult_t)
mSemCondVarSemCreate(
    msem_t*             pSem,
    CHAR*               strErrBuff,
    U32                 nErrBuff)
{
    tresult_t           _result;

    CCURASSERT(pSem);
    CCURASSERT(strErrBuff);

    do
    {
        _result = ESUCCESS;
        if (NULL == pSem)
        {
            ccur_strlcpy(strErrBuff,
                    "Invalid (NULL) semaphore.\n",
                    nErrBuff);
            _result = EFAILURE;
            break;
        }
        if(NULL == strErrBuff)
        {
            ccur_strlcpy(strErrBuff,
                    "Invalid (NULL) str err buffer.\n",
                    nErrBuff);
            _result = EFAILURE;
            break;
        }
        ccur_memclear(pSem, sizeof(msem_t));
        pSem->tAttributes.nCurValue    = 0;
        pSem->tAttributes.nMaxValue    = 1;
        /*
         * Initialize the mutex and condition variable.
         */
        pthread_mutex_init(&(pSem->hMutex), NULL);
        pthread_cond_init(&(pSem->tConditional), NULL);
    }while(FALSE);

    return _result;
}


CCUR_PROTECTED(tresult_t)
mSemCondVarSemDestroy(
        msem_t*   pSem)
{
    U32         _nLoopCount;
    tresult_t   _result;

    do
    {
        _result = EBUSY;
        /*
         *  Simple retry of spamming signals to all the threads.
         */
        for (_nLoopCount = 0; _nLoopCount < 2; _nLoopCount++)
        {
            if (EBUSY == pthread_mutex_trylock(&pSem->hMutex))
            {
                usleep(10);
            }
            else
            {
                pSem->tAttributes.nCurValue = pSem->tAttributes.nMaxValue;
                pthread_cond_broadcast(&pSem->tConditional);
                _nLoopCount = 0;
                _result = ESUCCESS;
                while ((pSem->nWaitingCount > 0) &&
                        (_nLoopCount++ < 3))
                {
                    _result = EBUSY;
                    pthread_mutex_unlock(&pSem->hMutex);
                    usleep(10);
                    pthread_mutex_lock(&pSem->hMutex);
                    if (0 == pSem->nWaitingCount)
                    {
                        _result = ESUCCESS;
                        break;
                    }
                }
                pthread_mutex_unlock(&pSem->hMutex);
                break;
            }
        }
        if (ESUCCESS != _result)
            break;
        pthread_cond_destroy(&pSem->tConditional);
        pthread_mutex_destroy(&pSem->hMutex);
    }while(FALSE);

    return _result;
}


CCUR_PROTECTED(tresult_t)
mSemCondVarSemWaitTimed(
    msem_t*             pSem,
    U32                 nTimeoutMs)
{
    tresult_t       _result;
    struct timeval  _tCurTime;
    struct timespec _tWaitExpTime;

    CCURASSERT(pSem);
    /*
     * Verify the input parameters.
     */

    _result = ETIMEDOUT;
    gettimeofday(&_tCurTime, NULL);
    _tWaitExpTime.tv_sec   = _tCurTime.tv_sec;
    _tWaitExpTime.tv_nsec  = _tCurTime.tv_usec * 1000;
    while (nTimeoutMs >= 1000)
    {
        _tWaitExpTime.tv_sec++;
        nTimeoutMs -= 1000;
    }
    _tWaitExpTime.tv_nsec += (nTimeoutMs * 1000000);
    _tWaitExpTime.tv_sec  += (_tWaitExpTime.tv_nsec / 1000000000);
    _tWaitExpTime.tv_nsec  = (_tWaitExpTime.tv_nsec % 1000000000);

    pthread_mutex_lock(&pSem->hMutex);
    pSem->nWaitingCount++;
    if (0 == pSem->tAttributes.nCurValue)
    {
        _result = pthread_cond_timedwait(
                    &pSem->tConditional,
                    &pSem->hMutex,
                    &_tWaitExpTime
                    );
    }
    else
        _result = ESUCCESS;
    if (ESUCCESS == _result)
    {
        if(pSem->tAttributes.nCurValue > 0)
            pSem->tAttributes.nCurValue--;
    }
    pSem->nWaitingCount--;
    pthread_mutex_unlock(&pSem->hMutex);
    return _result;
}

CCUR_PROTECTED(tresult_t)
mSemCondVarSemPost(
    msem_t*         pSem,
    U32*            pnPostValue)
{

    CCURASSERT(pSem);

    if (pSem)
    {
        pthread_mutex_lock(&pSem->hMutex);
        if (NULL == pnPostValue)
            pSem->tAttributes.nCurValue++;
        else
            pSem->tAttributes.nCurValue += *pnPostValue;
        if ((pSem->tAttributes.nMaxValue > 0) &&
            (pSem->tAttributes.nCurValue > pSem->tAttributes.nMaxValue))
            pSem->tAttributes.nCurValue = pSem->tAttributes.nMaxValue;
        if (pSem->tAttributes.nCurValue > 0)
            pthread_cond_signal(&pSem->tConditional);
        pthread_mutex_unlock(&pSem->hMutex);
    }
    return ESUCCESS;
}
