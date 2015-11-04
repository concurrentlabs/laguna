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

#ifndef TCPFRPCAP_H
#define TCPFRPCAP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRANSC_BUILDCFG_LIBPCAP

typedef struct pfring_pkthdr pfring_hdr_t;
#endif /* TRANSC_BUILDCFG_LIBPCAP */

CCUR_PROTECTED(tresult_t)
tcPfrPcapInitLibPfringRx(
        tc_pktprc_thread_ctxt_t * pCntx,
        U16                       nIntfIdx,
        U16                       nFilterIdx);

CCUR_PROTECTED(BOOL)
tcPfrPcapInitIsRxIntfLinkUp(
        tc_pktprc_thread_ctxt_t * pCntx);

CCUR_PROTECTED(tresult_t)
tcPfrPcapShutdownLibPfringRx(
        tc_pktprc_thread_ctxt_t* pCntx,
        tc_monintf_fltr_t* pMonIntffltr);

CCUR_PROTECTED(CHAR*)
tcPfrPcapGetStatPfringRx(
        tc_pktprc_thread_ctxt_t*     pCntx,
        CHAR*                        strBuff,
        U32                          nstrBuff);

CCUR_PROTECTED(tresult_t)
tcPfrPcapProcessLibPfring(
        tc_pktprc_thread_ctxt_t* pCntx );

#ifdef __cplusplus
}
#endif

#endif /* TCPFRPCAP_H */
