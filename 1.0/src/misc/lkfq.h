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

#ifndef LKFQ_H
#define LKFQ_H

#include "helper.h"
/***** external includes *****/
#include <liblfds611.h>
#include <cprops/mempool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void*               lkfq_data_p;
struct _lkfq_lfds_s
{
    pthread_mutex_t                         tLock;
    struct lfds611_queue_state*             pRbQs;
    BOOL                                    bInit;
    U32                                     errCnt;
};
typedef struct _lkfq_lfds_s lkfq_lfds_t;

struct _lkfq_tc_s
{
    lkfq_lfds_t             tQAlloc;
    lkfq_lfds_t             tQFree;
    cp_mempool *            pMp;
    BOOL                    bLock;
};
typedef struct _lkfq_tc_s
               lkfq_tc_t;

CCUR_PUBLIC(tresult_t)
lkfqCrQueue(
        lkfq_tc_t*            pQ,
        U16                   nBlockSize,
        U32                   nQueue,
        U16                   nChunkBlock,
        U16                   nPages,
        BOOL                  bLock);

CCUR_PUBLIC(void)
lkfqSyncQ(
        lkfq_tc_t*          pQ);

CCUR_PUBLIC(void)
lkfqDestQueue(
        lkfq_tc_t*       pQ);

CCUR_PUBLIC(lkfq_data_p)
lkfqMalloc(
        lkfq_tc_t*              pQ);

CCUR_PUBLIC(void)
lkfqFree(
        lkfq_tc_t*              pQ,
        lkfq_data_p             pD);

CCUR_PUBLIC(void)
lkfqFlushAllocList(
        lkfq_tc_t*              pQ);

CCUR_PUBLIC(void)
lkfqFlushFreeList(
        lkfq_tc_t*              pQ);

CCUR_PUBLIC(void)
lkfqWrite(
        lkfq_tc_t*            pQ,
        lkfq_data_p           pD);

CCUR_PUBLIC(lkfq_data_p*)
lkfqRead(
        lkfq_tc_t*          pQ);

CCUR_PUBLIC(void)
lkfqReadRelease(
        lkfq_tc_t*                              pQ,
        lkfq_data_p                             pData);

#ifdef __cplusplus
}
#endif
#endif /* LKFQ_H */
