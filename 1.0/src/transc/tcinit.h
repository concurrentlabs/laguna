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

#ifndef  TCINIT_H
#define  TCINIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "httpprc/httpprc.h"
#include "pktprc/pktprc.h"
#include "pktgen/pktgen.h"
#include "bkgrnd/bkgrnd.h"
#include "simprc/simprc.h"
#include "health/health.h"
#include "mib/mib.h"
#include "tcqmsg.h"
#include "tcutils.h"
#include "tctemp.h"

struct _tc_gd_thread_ctxt_s
{
    /* Common Mutex for all threads */
    U32                         nSimSendWThreadsNum;
    BOOL                        bIsThdStart;
    /*----------- Communication Queues ------------------ */
    /* Shared Communication Queues between threads */
    /* Communication Queues components -> Logging thread */
    evlog_t                     tQLogTbl[TRANSC_BKGRNDLOG_QUEUE_MAX];
    /* Communication Queues components -> Mib thread */
    lkfq_tc_t                   tQMibTbl[TRANSC_MIB_QUEUE_MAX];
    /* Communication Queues Pkt Proc -> Http Proc */
    lkfq_tc_t                   tQPPktProcToHttpProc;
    /* Communication Queues Http Proc -> Sim */
    lkfq_tc_t                   tQHttpProcToSim;
    /* Communication Queues Sim -> SimSend workers */
    lkfq_tc_t                   tQSimToSimSendWTbl[TRANSC_SIM_THD_MAX];
    /* Communication Queues SimSend -> sim  workers */
    lkfq_tc_t                   tQSimSendWToSimTbl[TRANSC_SIM_THD_MAX];
    /*----------- Contexts ------------------ */
    /* Back Ground (logging) thread */
    tc_bkgrnd_thread_ctxt_t     tBkGnThd;
    /* Mib thread */
    tc_tcplanesnmp_thread_ctxt_t tMibThd;
    /* Health thread */
    tc_health_thread_ctxt_t     tHealthThd;
    /* Pkt proc thread */
    tc_pktprc_thread_ctxt_t     tPktPrcThd;
    /* Http proc thread */
    tc_httpprc_thread_ctxt_t    tHttpPrcThd;
    /* Pkt Gen  */
    tc_pktgen_thread_ctxt_t     tPktGenThd;
    /* Right now Http Proc and Pkt gen are combined
     * Together as one thread.*/
    tc_temp_thread_ctxt_t     tHttpProcPktGen;
    /* Sim thread */
    tc_sim_thread_ctxt_t        tSimThd;
    /* Sim/Simsnd thread */
    tc_simsnd_thread_ctxt_t     tSimSndThdTbl[TRANSC_SIM_THD_MAX];
};
typedef struct _tc_gd_thread_ctxt_s
               tc_gd_thread_ctxt_t;

CCUR_PROTECTED(tresult_t)
tcInitRes(
        tc_gd_thread_ctxt_t*             pCntx);

CCUR_PROTECTED(tresult_t)
tcInitRunThreads(
        tc_gd_thread_ctxt_t*             pCntx);

CCUR_PROTECTED(tresult_t)
tcInitDaemonize(BOOL bDaemonize);

CCUR_PROTECTED(tresult_t)
tcInitReadFromConsole(
        tc_ldcfg_t* pCfg,
        S32 argc, CHAR*  argv[]);

CCUR_PROTECTED(tresult_t)
tcInitLoadConfigFiles(tc_gd_thread_ctxt_t* pCntx);

CCUR_PROTECTED(void)
tcInitUsageMsgPrintBanner();

CCUR_PROTECTED(void)
tcInitIsSwitchDaemonMode(BOOL bDaemonize);

CCUR_PROTECTED(void)
tcInitCleanupRes(
        mthread_result_t*       pExitCode,
        tc_gd_thread_ctxt_t*    pCntx);

CCUR_PROTECTED(tresult_t)
tcInitEventLog(
        tc_gd_thread_ctxt_t*    pCntx);

CCUR_PROTECTED(tc_gd_thread_ctxt_t*)
tcInitGetGlobalThdContext();

#ifdef __cplusplus
}
#endif
#endif
