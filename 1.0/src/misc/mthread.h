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

#ifndef MTHREAD_H
#define MTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#define MTHREAD_DEFAULT_PRIORITY     (0xFFFF)
#define MTHREAD_DEFAULT_STACK_SIZE   (-1)

typedef int mthread_result_t;

typedef mthread_result_t \
        (*mthreadentry_t) (void *);

/* for SCHED_RR and SCHED_FIFO */
typedef enum
{
    mThreadRtPriBackGrnd                = 0,
    mThreadRtPriLowest                  = 20,
    mThreadRtPriBelowNormal             = 40,
    mThreadRtPriNormal                  = 50,
    mThreadRtPriAboveNormal             = 60,
    mThreadRtPriHigh                    = 80,
    mThreadRtPriSuperDuperHigh          = 100
} mthreadpri_t;

struct _mthreadcreateattr_s
{
    S32                    nStackSize;
    S32                    nPriority;
    I32                    nPolicy;
    U64                    nCPUMask;
};

typedef struct _mthreadcreateattr_s
               mthreadcreateattr_t;

struct _mthread_s
{
    pthread_t               hThread;
    pthread_t               hParentThread;
    mthread_result_t        result;
};

typedef struct _mthread_s mthread_t;

struct _mthreadargs_s
{
    mthread_t*              pThread;
    mthreadentry_t          pfActualEntry;
    mthread_result_t*       pResult;
    S32                     nPriority;
    I32                     nPolicy;
    void*                   pActualArgs;
};

typedef struct _mthreadargs_s mthreadargs_t;

CCUR_PUBLIC(tresult_t)
mthreadSetPriority(
    mthread_t*         pThread,
    S32                nSchedPrio,
    I32                nPolicy);

CCUR_PUBLIC(tresult_t)
mthreadGetPriority(
    mthread_t*          pThread,
    S32*                pnPriority);

CCUR_PUBLIC(tresult_t)
mthreadWaitExit(
    mthread_t*          pThread,
    mthread_result_t*   pExitCode);

CCUR_PUBLIC(tresult_t)
mthreadCreateHelper(
    mthread_t*              pThread,
    mthreadentry_t       pfEntryRoutine,
    void*                   pThreadArgs,
    mthreadcreateattr_t*    pThreadAttribs);

CCUR_PUBLIC(tresult_t)
mthreadCreate(
    mthread_t*          phThread,
    mthreadentry_t      pfEntryRoutine,
    I32                 nStackSize,
    S32                 nSchedPrio,
    I32                 nSchedPolicy,
    void*               pThreadArgs);

#ifdef __cplusplus
extern "C" {
#endif
#endif /* MTHREAD_H */
