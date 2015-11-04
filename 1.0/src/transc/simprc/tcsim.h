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

#ifndef TCSIM_H
#define TCSIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cprops/mempool.h>

struct _tc_sim_ckeymap_s
{
    /* cdll_node_t must be on top of struct.
     * as circular linklist node */
    cdll_node_t             tNode;
    U32                     nDynHashId;
    U32                     nBin;
    tc_gd_time_t            tLastUpdateTime;
    U32                     nTimoutSec;
    tc_qmsgtbl_pptosim_t    tPktInfo;
};
typedef struct _tc_sim_ckeymap_s
               tc_sim_ckeymap_t;

struct _tc_sim_thread_ctxt_s
{
    /****** Thread Init ****************/
    U32                     tid;
    BOOL                    bExit;
    msem_t                  tCondVarSem;
    BOOL                    bCondVarSet;
    mthread_t               tThd;
    /****** Periodic time  ****************/
    struct tm*              tGMUptime;
    tc_gd_time_t            tUptime;
    tc_gd_time_t            tDowntime;
    /****** config and init Flags ****************/
    BOOL                    bBwSimMode;
    CHAR                    strSimBwOutIntf[TRANSC_LDCFG_BWSIM_OUTINTFLEN];
    BOOL                    bLoadSimSndCfgTbl[TRANSC_SIM_THD_MAX];
    /****** Statistics counters ****************/
    U32                     nErrNoMemSimToSimSnd;
    U32                     nErrCount;
    U32                     nCurlReqErr;
    U32                     nBadRespErr;
    U32                     nIgnCount;
    U32                     nHttpReq;
    U32                     nHttpGETReq;
    U32                     nHttpHEADReq;
    U32                     nSimSendWThreadsNum;
    U32                     nRRQSimToSimSnd;
    U16                     nSimMsgOflw;
    /****** Ckey mapping ****************/
    cdll_hdnode_t           shCkeyMapHashTable[TRANSC_SIMCKEY_DFLTBIN_CNT];
    U32                     nCkeyEntryTimeoutCnt;
    U32                     nResCkeyMapBinMax;
    U32                     nCkeyMapHtblBin;
    cp_mempool *            pCkeyMapMpool;
    U32                     nCkeyActive;
    /****** QUEUES Management ****************/
    /*Sim -> Bkgrnd */
    evlog_desc_t            tLogDescSys;
    evlog_desc_t            tLogDescSvc;
    evlog_t*                pQSimToBkgrnd;
    /* Http proc -> Sim */
    lkfq_tc_t*              pQHttpProcToSim;
    /* Sim To Sim/Send */
    lkfq_tc_t*              pQSimToSimSendWTbl[TRANSC_SIM_THD_MAX];
    /* Sim/Send To Sim */
    lkfq_tc_t*              pQSimSendWToSimTbl[TRANSC_SIM_THD_MAX];
    U32                     nQSimSendWToSimWrCnt;
    /****** temporary variables Management */
};
typedef struct _tc_sim_thread_ctxt_s
               tc_sim_thread_ctxt_t;

CCUR_PROTECTED(mthread_result_t)
tcSimProcThreadEntry(void* pthdArg);

CCUR_PROTECTED(tresult_t)
tcSimSndThreadInitRes(
        tc_simsnd_thread_ctxt_t*     pCntx,
        tc_ldcfg_conf_t*             pLdCfg);

CCUR_PROTECTED(tresult_t)
tcSimProcInitLoadableRes(
        tc_sim_thread_ctxt_t*        pCntx,
        tc_ldcfg_conf_t*             pLdCfg);

CCUR_PROTECTED(tresult_t)
tcSimCkeyMapMpoolCreate(
        tc_sim_thread_ctxt_t*        pCntx);

CCUR_PROTECTED(void)
tcSimCkeyMapMpoolDestroy(
        tc_sim_thread_ctxt_t*        pCntx);

CCUR_PROTECTED(void)
tcSimHtableTimeoutCheck(
        tc_sim_thread_ctxt_t*       pCntx);

#ifdef __cplusplus
}
#endif
#endif /* TCSIM_H */
