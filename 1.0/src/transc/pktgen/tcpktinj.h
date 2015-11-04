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

#ifndef TCPKTINJ_H
#define TCPKTINJ_H

#ifdef __cplusplus
extern "C" {
#endif


CCUR_PROTECTED(tresult_t)
tcPktInjInjectReq302Pkt(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_g_qmsghptopg_t*          pHttpMsg,
        tc_pktdesc_t*               pPktDesc,
        hlpr_httpsite_hndl          eSite,
        tc_outintf_out_t*           pOutIntf,
        tc_outintf_mon_t*           pMonIntf);

CCUR_PROTECTED(tresult_t)
tcPktInjInjectReqFinPkt(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_pktdesc_t*               pPktDesc,
        hlpr_httpsite_hndl          eSite,
        tc_pktgen_tcpflgproto_e     eFlagProto,
        tc_outintf_out_t*          pOutIntf);

#ifdef __cplusplus
}
#endif
#endif
