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

#ifndef MSEM_H
#define MSEM_H

#ifdef __cplusplus
extern "C" {
#endif

struct _msemattr_s
{
    S32                 nCurValue;
    S32                 nMaxValue;
};

typedef struct _msemattr_s msemattr_t;

struct _msem_s
{
    msemattr_t          tAttributes;
    pthread_mutex_t     hMutex;
    pthread_cond_t      tConditional;
    U32                 nWaitingCount;
};
typedef struct _msem_s msem_t;


CCUR_PUBLIC(tresult_t)
mSemCondVarSemCreate(
    msem_t*             pSem,
    CHAR*               strErrBuff,
    U32                 nErrBuff);

CCUR_PUBLIC(tresult_t)
mSemCondVarSemWaitTimed(
    msem_t*             pSem,
    U32                 nTimeoutMs);

CCUR_PROTECTED(tresult_t)
mSemCondVarSemDestroy(
    msem_t*             pSem);

CCUR_PUBLIC(tresult_t)
mSemCondVarSemPost(
    msem_t*         pSem,
    U32*            pnPostValue);

#ifdef __cplusplus
extern "C" {
#endif
#endif /* MSEM_H */
