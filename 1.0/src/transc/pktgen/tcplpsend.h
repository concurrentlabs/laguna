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

#ifndef _TCPLPSEND_
#define _TCPLPSEND_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TRANSC_BUILDCFG_LIBPCAP

CCUR_PROTECTED(tresult_t)
tcPlPsendInitLibPcapTx(
        tc_pktgen_thread_ctxt_t * pCntx);

CCUR_PROTECTED(tresult_t)
tcPlPsendShutdownLibPcapTx(
        tc_pktgen_thread_ctxt_t * pCntx);

CCUR_PROTECTED(BOOL)
tcPlPsendInitIsTxIntfLinkUp(
        tc_pktgen_thread_ctxt_t * pCntx);

CCUR_PROTECTED(I32)
tcPlPsendLibPcapTx(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_pktgen_t*                pPktGen,
        tc_pktgen_pktinj_t*         pkt);


CCUR_PROTECTED(CHAR*)
tcPlPsendGetStatLibPcapTx(
        tc_pktgen_thread_ctxt_t*     pCntx,
        CHAR*                        strBuff,
        U32                          nstrBuff);
#endif

#ifdef __cplusplus
}
#endif
#endif
