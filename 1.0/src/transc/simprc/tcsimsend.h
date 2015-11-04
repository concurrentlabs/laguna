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

#ifndef TCSIMSEND_H
#define TCSIMSEND_H

#ifdef __cplusplus
extern "C" {
#endif

#define TRANSC_SIMSND_CONTLENBUFF  TRANSC_SIM_CONTLENBUFF

struct _tc_simsnd_thread_ctxt_s
{
    /****** Thread Init ****************/
    U32                     tid;
    U32                     cfgId;
    BOOL                    bExit;
    msem_t                  tCondVarSem;
    BOOL                    bCondVarSet;
    mthread_t               tThd;
    /****** Periodic time  ****************/
    struct tm*              tGMUptime;
    tc_gd_time_t            tUptime;
    tc_gd_time_t            tDowntime;
    /****** config and init Flags ****************/
    CHAR                    strSimBwOutIntf[TRANSC_LDCFG_BWSIM_OUTINTFLEN];
    BOOL                    bBwSimMode;
    BOOL                    bLdCfgYaml;
    /****** Statistics counters ****************/
    U32                     nMsgOflw;
    U32                     nErrNoQMemSimSndToSim;
    tc_simutil_curlerr_t    tCurlErr;
    U32                     nErrCount;
    U32                     nCurlReqErr;
    U32                     nBadRespErr;
    U32                     nIgnCount;
    U32                     nHttpReq;
    U32                     nHttpSuccessHEADreq;
    U32                     nHttpSuccessGETreq;
    /****** QUEUES Management ****************/
    /*Sim -> Bkgrnd */
    evlog_desc_t            tLogDescSys;
    evlog_desc_t            tLogDescSvc;
    evlog_t*                pQSimSendWToBkgrnd;
    /* Sim To Sim/Send */
    lkfq_tc_t*              pQSimToSimSendW;
    /* Sim/Send To Sim */
    lkfq_tc_t*              pQSimSendWToSim;
    /****** temporary variables Management */
    U8                      strBodyPyload[TRANSC_LDCFG_BWSIM_BODYPYLDLEN];
};
typedef struct _tc_simsnd_thread_ctxt_s
               tc_simsnd_thread_ctxt_t;

CCUR_PROTECTED(mthread_result_t)
tcSimSendProcThreadEntry(void* pthdArg);

CCUR_PROTECTED(tresult_t)
tcSimSendProcInitLoadableRes(
        tc_simsnd_thread_ctxt_t*        pCntx,
        tc_ldcfg_conf_t*                pLdCfg);

#ifdef __cplusplus
}
#endif
#endif /* tcsimsnd_H */
