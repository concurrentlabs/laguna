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

#ifndef TCHEALTH_H
#define TCHEALTH_H

#ifdef __cplusplus
extern "C" {
#endif

#define TRANSC_HEALTH_WAITTIMEOUT_MS    100
#define TRANSC_HEALTH_MOOMSG_LEN        128
#define TRANSC_HEALTH_PING_INTERVAL     30

struct _tc_health_thread_ctxt_s
{
    /****** Thread Init ****************/
    U32                             tid;
    BOOL                            bExit;
    msem_t                          tCondVarSem;
    BOOL                            bCondVarSet;
    mthread_t                       tThd;
    struct tm*                      tGMUptime;
    tc_gd_time_t                    tUptime;
    tc_gd_time_t                    tDowntime;
    /****** config and init Flags ****************/
    tc_ldcfg_t                      tConfig;
    BOOL                            bNoRRPolling;
    BOOL                            bMonIntfUpdate;
    BOOL                            bOutIntfUpdate;
    /****** Health internal dstruct  ****************/
    tc_health_monactv_t             tMonActvTbl[TRANSC_INTERFACE_MAX];
    U16                             nMonActvTbl;
    tc_tr_sts_e                     eTrStatusTbl[tcTRCompTypeMax];
    /****** QUEUES Management ****************/
    /* PP -> Bkgrnd thread */
    evlog_desc_t                    tLogDescSys;
    evlog_t*                        pQHealthToBkgrnd;
    /****** temporary variables Management */
    tc_mtxtbl_ipptohlth_t           tMtxPPToHealthMonIntfMsg;
    tc_mtxtbl_stspgentohlth_t       tMtxPktGenToHealthStsMsg;
    tc_mtxtbl_epgentohlth_t         tMtxPktGenToHealthOutIntfMsg;
    tc_mtxtbl_mpgentohlth_t         tMtxPktGenToHealthMapIntfMsg;
    tc_mtxtbl_mhlthtopktgen_t       tMtxHealthToPktGenActvMsg;
    tc_mtxtbl_mhlthtomib_t          tMtxHealthToMibMsg;

};
typedef struct _tc_health_thread_ctxt_s
               tc_health_thread_ctxt_t;

CCUR_PROTECTED(mthread_result_t)
tcHealthThreadEntry(void* pthdArg);

#ifdef __cplusplus
}
#endif
#endif /* TCHEALTH_H */
