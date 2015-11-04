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

#ifndef TCTCPSTRM_H
#define TCTCPSTRM_H

CCUR_PROTECTED(tresult_t)
tcTCPStreamSessGetSessCacheKeyRel(
        tc_httpprc_thread_ctxt_t*   pCntx,
        tc_httpparse_calldef_t*     pHttpMsg,
        tc_g_svcdesc_t*             pSvcType);

CCUR_PROTECTED(tresult_t)
tcTCPStreamSessGetSessCacheKeySess(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_httpparse_calldef_t*     pHttpMsg,
        tc_g_svcdesc_t*             pSvcType);

CCUR_PROTECTED(void)
tcTCPStreamSessHtableTimeoutCheck(
        tc_httpprc_thread_ctxt_t*   pCntx,
        tc_gd_time_t*               pTimeNow);

CCUR_PROTECTED(tresult_t)
tcTCPStreamSessEntryMpoolCreate(
        tc_httpprc_thread_ctxt_t*        pCntx);

CCUR_PROTECTED(void)
tcTCPStreamSessEntryMpoolDestroy(
        tc_httpprc_thread_ctxt_t*        pCntx);

CCUR_PROTECTED(void)
tcTCPStreamSessEntryMpoolGetStats(
        tc_httpprc_thread_ctxt_t*        pCntx);

#endif /* TCPSTRM_H */
