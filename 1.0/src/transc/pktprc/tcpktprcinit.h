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

#ifndef TCPKTPRCINIT_H
#define TCPKTPRCINIT_H

#ifdef __cplusplus
extern "C" {
#endif

CCUR_PROTECTED(void)
tcPktPrcInitSiteCleanupRes(
        tc_pktprc_thread_ctxt_t *     pCntx);

CCUR_PROTECTED(tresult_t)
tcPktProcConfigInitLoadableRes(
        tc_pktprc_thread_ctxt_t*     pCntx,
        tc_ldcfg_conf_t*             pLdCfg);

CCUR_PROTECTED(BOOL)
tcPktPrcInitCkIntf(
        tc_pktprc_thread_ctxt_t*     pCntx,
        tc_ldcfg_conf_t*             pLdCfg);

CCUR_PROTECTED(tresult_t)
tcPktPrcInitIntf(
        tc_pktprc_thread_ctxt_t*     pCntx,
        BOOL                         bRxLoad);

CCUR_PROTECTED(tresult_t)
tcPktPrcCkIpBlacklist(
        tc_pktprc_thread_ctxt_t*        pCntx,
        tc_pktdesc_t*                   pPktDesc);

CCUR_PROTECTED(void)
tcPktProcInitLogDownStatusAndRetry(
        tc_pktprc_thread_ctxt_t*     pCntx);


#ifdef __cplusplus
}
#endif

#endif /* TCPKTPRCINIT_H */
