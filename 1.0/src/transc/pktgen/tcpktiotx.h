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

#ifndef TCPKTIOTX_H
#define TCPKTIOTX_H

#ifdef __cplusplus
extern "C" {
#endif

CCUR_PROTECTED(tresult_t)
tcPktIOTxOutIntfOpen(
        tc_pktgen_thread_ctxt_t* pCntx );

CCUR_PROTECTED(void)
tcPktIOTxOutIntfClose(
        tc_pktgen_thread_ctxt_t* pCntx );

CCUR_PROTECTED(CHAR*)
tcPktIOTxOutIntfGetStat(
        tc_pktgen_thread_ctxt_t*     pCntx,
        CHAR*                        strBuff,
        U32                          nstrBuff);

CCUR_PROTECTED(BOOL)
tcPktIOTxOutIntfIsLinkUp(
        tc_pktgen_thread_ctxt_t* pCntx );

CCUR_PROTECTED(I32)
tcPktIOTxOutIntfToWire(
        tc_pktgen_thread_ctxt_t* pCntx,
        tc_outintf_out_t*       pOutIntf,
        tc_pktgen_pktinj_t*      pkt);

#ifdef __cplusplus
}
#endif
#endif
