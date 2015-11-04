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

#ifndef  TCTEMP_H
#define  TCTEMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pktprc/pktprc.h"
#include "pktgen/pktgen.h"

/* TODO: Split this into each contexts of http proc and pktgen and
 * delete this file. */
struct _tc_temp_thread_ctxt_s
{
    /* Http proc thread */
    tc_httpprc_thread_ctxt_t*   pHttpPrcThd;
    /* Pkt Gen  */
    tc_pktgen_thread_ctxt_t*    pPktGenThd;
};
typedef struct _tc_temp_thread_ctxt_s
                tc_temp_thread_ctxt_t;

#ifdef __cplusplus
}
#endif
#endif
