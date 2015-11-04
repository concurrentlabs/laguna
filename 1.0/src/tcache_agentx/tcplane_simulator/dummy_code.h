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

#ifndef DUMMY_CODE_H
#define DUMMY_CODE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define evLogLvlDebug 1

#define ESUCCESS 1

typedef void * tc_gd_thread_ctxt_t;

int tcBkgrndReLoadCfg(tc_gd_thread_ctxt_t * pCntx);

int tcBkgrndGetTrSts(tc_gd_thread_ctxt_t * pCntx);

char * tcBkgrndGetRedirAddr(tc_gd_thread_ctxt_t * pCntx, char * ip, size_t size);

void evLogTraceSys(int level, const char * fmt, const char * msg);

void tcBkgrndSetRedirNode(tc_gd_thread_ctxt_t * _pCntx, int state);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif
