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
#include <unistd.h>
#include "helper.h"
#include "lkfq.h"
#include "helper.h"
#include "tcpktparse.h"
#include "tcutil.h"

#define MISC_LKFQ_STS_FAIL      0
#define MISC_LKFQ_STS_SUCCESS   1

CCUR_PUBLIC(tresult_t)
lkfqCrQueue(
        lkfq_tc_t* pQ,
        U16        nBlockSize,
        U32        nQueue,
        U16        nChunkBlock,
        U16        nPages,
        BOOL       bLock)
{
    tresult_t  _result;

    CCURASSERT(pQ);

    do
    {
        _result = lfds611_queue_new(
                        &(pQ->tQAlloc.pRbQs), nQueue);
        if(!_result )
        {
            _result = ENOMEM;
            break;
        }
        _result = lfds611_queue_new(
                        &(pQ->tQFree.pRbQs), nQueue);
        if(!_result )
        {
            _result = ENOMEM;
            break;
        }
        if(bLock)
            pQ->bLock = TRUE;
        _result = pthread_mutex_init (
                &(pQ->tQAlloc.tLock), NULL);
        if(ESUCCESS != _result)
        {
            _result = EFAILURE;
            break;
        }
        _result = pthread_mutex_init (
                &(pQ->tQFree.tLock), NULL);
        if(ESUCCESS != _result)
        {
            _result = EFAILURE;
            break;
        }

        pQ->pMp =
            cp_mempool_create_by_option(CP_SHARED_MEMPOOL_TYPE_1, nBlockSize, 100);
        if(NULL == pQ->pMp)
        {
            _result = ENOMEM;
            pQ->pMp = NULL;
            break;
        }
        _result = ESUCCESS;
    }while(FALSE);

    if(ESUCCESS != _result)
    {
        lkfqDestQueue(pQ);
    }
    return _result;
}

CCUR_PUBLIC(void)
lkfqSyncQ(lkfq_tc_t * pQ)
{
    pthread_mutex_lock(&(pQ->tQAlloc.tLock));
    lfds611_queue_use( pQ->tQAlloc.pRbQs );
    pQ->tQAlloc.bInit = TRUE;
    pthread_mutex_unlock(&(pQ->tQAlloc.tLock));

    pthread_mutex_lock(&(pQ->tQFree.tLock));
    lfds611_queue_use( pQ->tQFree.pRbQs );
    pQ->tQFree.bInit = TRUE;
    pthread_mutex_unlock(&(pQ->tQFree.tLock));
}

CCUR_PUBLIC(void)
lkfqDestQueue(lkfq_tc_t * pQ)
{
    CCURASSERT(pQ);

    if(pQ->tQFree.pRbQs)
    {
        lkfqFlushFreeList(pQ);
        lfds611_queue_delete(pQ->tQFree.pRbQs, NULL, NULL );
        pQ->tQFree.pRbQs = NULL;
        pthread_mutex_destroy(&(pQ->tQFree.tLock));
    }
    if(pQ->tQAlloc.pRbQs)
    {
        lkfqFlushAllocList(pQ);
        lfds611_queue_delete(pQ->tQAlloc.pRbQs, NULL, NULL );
        pQ->tQAlloc.pRbQs = NULL;
        pthread_mutex_destroy(&(pQ->tQAlloc.tLock));
    }
    if(pQ->pMp)
        cp_mempool_destroy(pQ->pMp);
}

CCUR_PUBLIC(lkfq_data_p)
lkfqMalloc(lkfq_tc_t * pQ)
{
    lkfq_data_p _pData;

    CCURASSERT(pQ);

    lkfqFlushFreeList(pQ);
    _pData = cp_mempool_alloc(pQ->pMp);
    return _pData;
}

CCUR_PUBLIC(void)
lkfqFree(lkfq_tc_t * pQ, lkfq_data_p pD)
{
    CCURASSERT(pQ);
    CCURASSERT(pD);

    if(pQ && pD)
        cp_mempool_free(pQ->pMp, pD);
}

CCUR_PUBLIC(void)
lkfqFlushAllocList(lkfq_tc_t * pQ)
{
    lkfq_data_p  pD = NULL;
    BOOL         _bElemAvail = TRUE;

    CCURASSERT(pQ);

    if(pQ)
    {
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQFree Mutex here */
        if(pQ->bLock)
            pthread_mutex_lock(&(pQ->tQAlloc.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
        while(_bElemAvail)
        {
            if(MISC_LKFQ_STS_FAIL ==
                    lfds611_queue_dequeue(pQ->tQAlloc.pRbQs, &pD ))
            {
                _bElemAvail = FALSE;
            }
            else
            {
                cp_mempool_free(pQ->pMp, pD);
                _bElemAvail = TRUE;
            }
        }
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQFree Mutex here */
        if(pQ->bLock)
            pthread_mutex_unlock(&(pQ->tQFree.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
    }
}


CCUR_PUBLIC(void)
lkfqFlushFreeList(lkfq_tc_t * pQ)
{
    lkfq_data_p  _pD;
    BOOL         _bElemAvail = TRUE;

    CCURASSERT(pQ);

    if(pQ)
    {
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQFree Mutex here */
        if(pQ->bLock)
            pthread_mutex_lock(&(pQ->tQFree.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
        while(_bElemAvail)
        {
            if(MISC_LKFQ_STS_FAIL ==
                    lfds611_queue_dequeue( pQ->tQFree.pRbQs, &_pD ))
            {
                _bElemAvail = FALSE;
            }
            else
            {
                cp_mempool_free(pQ->pMp, _pD);
                _bElemAvail = TRUE;
            }
        }
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQFree Mutex here */
        if(pQ->bLock)
            pthread_mutex_unlock(&(pQ->tQFree.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
    }
}

CCUR_PUBLIC(void)
lkfqWrite(lkfq_tc_t* pQ, lkfq_data_p pD)
{
    CCURASSERT(pQ);
    CCURASSERT(pD);

    if(pQ && pD)
    {
        /* Flush the freelist before writing. */
        lkfqFlushFreeList(pQ);
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQAlloc Mutex here */
        if(pQ->bLock)
            pthread_mutex_lock(&(pQ->tQAlloc.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
        if(MISC_LKFQ_STS_FAIL ==
                lfds611_queue_enqueue(pQ->tQAlloc.pRbQs,pD))
        {
            cp_mempool_free(pQ->pMp, pD);
        }
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQAlloc Mutex here */
        if(pQ->bLock)
            pthread_mutex_unlock(&(pQ->tQAlloc.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
    }
}

CCUR_PUBLIC(lkfq_data_p*)
lkfqRead(lkfq_tc_t * pQ)
{
    lkfq_data_p  _pD = NULL;

    CCURASSERT(pQ);

    if(pQ)
    {
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQAlloc Mutex here */
        if(pQ->bLock)
            pthread_mutex_lock(&(pQ->tQAlloc.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
        if(MISC_LKFQ_STS_FAIL ==
                lfds611_queue_dequeue( pQ->tQAlloc.pRbQs, &_pD ))
        {
            _pD = NULL;
        }
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQAlloc Mutex here */
        if(pQ->bLock)
            pthread_mutex_unlock(&(pQ->tQAlloc.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
    }

    return _pD;
}

CCUR_PUBLIC(void)
lkfqReadRelease(lkfq_tc_t * pQ, lkfq_data_p pD)
{
    CCURASSERT(pQ);
    CCURASSERT(pD);

    if(pQ && pD)
    {
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQFree Mutex here */
        if(pQ->bLock)
            pthread_mutex_lock(&(pQ->tQFree.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
        while(1)
        {
            if(MISC_LKFQ_STS_FAIL ==
                    lfds611_queue_enqueue(pQ->tQFree.pRbQs,pD))
                usleep(10);
            else
                break;
        }
#if TRANSC_LKFQ_LOCKCK
        /* pQ->tQFree Mutex here */
        if(pQ->bLock)
            pthread_mutex_unlock(&(pQ->tQFree.tLock));
#endif /* TRANSC_LKFQ_LOCKCK */
    }
}
