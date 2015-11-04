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

#include <sys/time.h>
#include "helper.h"
#include "mthread.h"
#include "pthread.h"

/* Thin layer over POSIX threads */

CCUR_PUBLIC(tresult_t)
mthreadSetPriority(
    mthread_t*         pThread,
    S32                nSchedPrio,
    I32                nPolicy)
{
    tresult_t               _result;
    pthread_t               _tid;
    struct sched_param      _tSchedParam;
    S32                     _nMinPri;
    S32                     _nMaxPri;
    S32                     _nSchedPrio;
    S32                     _nPriority;

    do
    {
        _result = ESUCCESS;
        if (NULL ==  pThread)
            _tid = pthread_self();
        else
            _tid = pThread->hThread;
        _nMinPri = sched_get_priority_min(nPolicy);
        _nMaxPri = sched_get_priority_max(nPolicy);
        if(nSchedPrio < _nMinPri)
            _nSchedPrio = _nMinPri;
        else if(nSchedPrio > _nMaxPri)
            _nSchedPrio = _nMaxPri;
        else
            _nSchedPrio  = nSchedPrio;
        switch(nPolicy)
        {
            case SCHED_RR:
            case SCHED_FIFO:
                _nPriority =
                        ((_nSchedPrio * (_nMaxPri - _nMinPri))/
                                mThreadRtPriSuperDuperHigh) + _nMinPri;
                break;
            case SCHED_OTHER:
#ifdef SCHED_NORMAL
            case SCHED_NORMAL:
#endif
#ifdef SCHED_BATCH
            case SCHED_BATCH:
#endif
#ifdef SCHED_IDLE
            case SCHED_IDLE:
#endif
                _nPriority = _nSchedPrio;
                break;
            default:
                _result = EFAILURE;
                break;
        }
        if(ESUCCESS != _result)
            break;
        ccur_memclear(&_tSchedParam,sizeof(_tSchedParam));
        _tSchedParam.sched_priority = _nPriority;
        if(ESUCCESS != pthread_setschedparam(
                _tid, nPolicy, &_tSchedParam))
            break;
    }while(FALSE);

    return _result;
}

CCUR_PUBLIC(tresult_t)
mthreadGetPriority(
    mthread_t*          pThread,
    S32*                pnPriority)
{
    tresult_t           _result;
    I32                 _nPolicy;
    struct sched_param  _tSchedParams;

    CCURASSERT(pThread);
    CCURASSERT(pnPriority);

    do
    {
        _result =
            (tresult_t)pthread_getschedparam(
                pThread->hThread,
                &_nPolicy,
                &_tSchedParams);
        if (ESUCCESS != _result)
            break;
        if (pnPriority)
            *pnPriority = _tSchedParams.sched_priority;
    }while(FALSE);

    return _result;
}

CCUR_THREAD_ENTRY(void *)
mthreadEntryPriv(
    void* pThreadArgs)
{
    U8                      _nPriority;
    void*                   _pActualArgs;
    sigset_t                _tSigSet;
    I32                     _nPolicy;
    mthread_t*              _pThreadInfo;
    mthread_result_t*       _presult;
    mthread_result_t        _result;
    mthreadentry_t          _pfActualEntry;
    mthreadargs_t*          _pThreadArgs =
                            (mthreadargs_t*)pThreadArgs;

    CCURASSERT(pThreadArgs);

    /* Another wrapper,
     * Add whatever you need to init thread here,
     * same old, same old method...
     */
    /*
     * Retrieve the thread control
     * information from the thread arguments.
     */
    _presult       = _pThreadArgs->pResult;
    _pActualArgs   = _pThreadArgs->pActualArgs;
    _pfActualEntry = _pThreadArgs->pfActualEntry;
    _pThreadInfo   = _pThreadArgs->pThread;
    _nPriority     = _pThreadArgs->nPriority;
    _nPolicy       = _pThreadArgs->nPolicy;
    free(_pThreadArgs);

    /*
     * Setup the thread to block all interesting signals. This
     *   way we can correctly dispatch termination signals under
     *   both a pure threading and a tasking environment.
     */
    sigfillset(&_tSigSet);
    sigdelset(&_tSigSet, SIGHUP);
    sigdelset(&_tSigSet, SIGINT);
    sigdelset(&_tSigSet, SIGQUIT);
    sigdelset(&_tSigSet, SIGFPE);
    sigdelset(&_tSigSet, SIGSEGV);
    sigdelset(&_tSigSet, SIGPIPE);
    sigdelset(&_tSigSet, SIGALRM);
    sigdelset(&_tSigSet, SIGTERM);
    sigdelset(&_tSigSet, SIGUSR1);
    sigdelset(&_tSigSet, SIGUSR2);
    sigdelset(&_tSigSet, SIGCHLD);
    pthread_sigmask(SIG_SETMASK, &_tSigSet, NULL);

    /* Set real-time thread priority */
    _result = mthreadSetPriority(
                        _pThreadInfo,
                        _nPriority,
                        _nPolicy);
    /* Init glib for each thread for version < 2.32 */
    if( ! g_thread_supported() )
        g_thread_init( NULL );
    if(ESUCCESS == _result)
        _result = _pfActualEntry(_pActualArgs);
    *_presult = _result;
    return ((void*)_presult);
}

CCUR_PUBLIC(tresult_t)
mthreadWaitExit(
    mthread_t*        pThread,
    mthread_result_t* pExitCode)
{
    tresult_t         _result;
    mthread_result_t* _exitCode;

    _result = pthread_join(
            pThread->hThread,
            (void*)(&_exitCode));
    if(ESUCCESS == _result)
    {
        if(pExitCode)
            *pExitCode = *_exitCode;
    }

    return _result;
}

CCUR_PUBLIC(tresult_t)
mthreadCreateHelper(
    mthread_t*              pThread,
    mthreadentry_t          pfEntryRoutine,
    void*                   pThreadArgs,
    mthreadcreateattr_t*    pThreadAttribs)
{
    tresult_t               _result;
    pthread_attr_t          _tThreadAttr;
    S32                     _nSchedPrio;
    S32                     _nPriority;
    S32                     _nMinPri;
    S32                     _nMaxPri;
    mthreadargs_t*          _pArgs = NULL;
#if 0
    struct sched_param _tSchedParams;
#endif /* 0 */

    CCURASSERT(pThread);
    CCURASSERT(pThreadArgs);
    CCURASSERT(pThreadAttribs);

    do
    {
        _result = ESUCCESS;
        _pArgs = (mthreadargs_t*)malloc(sizeof(mthreadargs_t));
        if (NULL == _pArgs)
        {
            _result = ENOMEM;
            break;
        }
        _nMinPri = sched_get_priority_min(pThreadAttribs->nPolicy);
        _nMaxPri = sched_get_priority_max(pThreadAttribs->nPolicy);
        if(pThreadAttribs->nPriority < _nMinPri)
            _nSchedPrio = _nMinPri;
        else if(pThreadAttribs->nPriority > _nMaxPri)
            _nSchedPrio = _nMaxPri;
        else
            _nSchedPrio  = pThreadAttribs->nPriority;
        switch(pThreadAttribs->nPolicy)
        {
            case SCHED_RR:
            case SCHED_FIFO:
                _nPriority =
                        ((_nSchedPrio * (_nMaxPri - _nMinPri))/
                                mThreadRtPriSuperDuperHigh) + _nMinPri;
                break;
            case SCHED_OTHER:
#ifdef SCHED_NORMAL
            case SCHED_NORMAL:
#endif
#ifdef SCHED_BATCH
            case SCHED_BATCH:
#endif
#ifdef SCHED_IDLE
            case SCHED_IDLE:
#endif
                _nPriority = _nSchedPrio;
                break;
            default:
                _result = EFAILURE;
                break;
        }
        if(ESUCCESS != _result)
            break;
        /* Thread args init */
        _pArgs->pThread             = pThread;
        _pArgs->pResult             = &(pThread->result);
        _pArgs->pActualArgs         = pThreadArgs;
        _pArgs->pfActualEntry       = pfEntryRoutine;
        _pArgs->nPriority           = _nPriority;
        _pArgs->nPolicy             = pThreadAttribs->nPolicy;

        /* Thread attributes init */
        pThread->hParentThread = pthread_self();
        pthread_attr_init(&_tThreadAttr);
        pthread_attr_setscope(&_tThreadAttr,
                              PTHREAD_SCOPE_SYSTEM);
        /* overload stack size */
        if (pThreadAttribs->nStackSize > 0)
        {
            pthread_attr_setstacksize(
                &_tThreadAttr,
                pThreadAttribs->nStackSize
                );
        }
#if 0
        pthread_attr_setinheritsched(&_tThreadAttr, PTHREAD_INHERIT_SCHED);
        pthread_attr_getschedparam(&_tThreadAttr, &_tSchedParams);
        _tSchedParams.sched_priority = pThreadAttribs->nPriority;
        pthread_attr_setschedparam(&_tThreadAttr, &_tSchedParam);
#endif
        _result = (tresult_t)pthread_create(
                                        &pThread->hThread,
                                        &_tThreadAttr,
                                        mthreadEntryPriv,
                                        (void*)_pArgs);
        if (ESUCCESS != _result)
            break;
        pthread_attr_destroy(&_tThreadAttr);
#if 0
        /* Set CPU affinity mask, if specified */
        if(pThreadAttribs && pThreadAttribs->nCPUMask)
        {
            mthreadSetAffinity(pThread, pThreadAttribs->nCPUMask);
        }
#endif

    }while(FALSE);

    if (ESUCCESS != _result)
        free(_pArgs);

    return _result;
}

CCUR_PUBLIC(tresult_t)
mthreadCreate(
    mthread_t*          phThread,
    mthreadentry_t      pfEntryRoutine,
    I32                 nStackSize,
    S32                 nSchedPrio,
    I32                 nSchedPolicy,
    void*               pThreadArgs)
{
    tresult_t                   _result;
    mthreadcreateattr_t         _tThreadAttribs;

    CCURASSERT(phThread);
    CCURASSERT(pThreadArgs);

    do
    {
        _tThreadAttribs.nStackSize      = nStackSize;
        _tThreadAttribs.nPriority       = nSchedPrio;
        _tThreadAttribs.nPolicy         = nSchedPolicy;
        _tThreadAttribs.nCPUMask        = 0;
        _result = mthreadCreateHelper(
                phThread,
                pfEntryRoutine,
                pThreadArgs,
                &_tThreadAttribs
                );
    }while(FALSE);

    return _result;
}


