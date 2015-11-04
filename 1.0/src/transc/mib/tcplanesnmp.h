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

#ifndef  TCPLANESNMP_H_
#define  TCPLANESNMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TRANSC_SNMPPLANE_WAITTIMEOUT_MS     100

enum _tc_snmpplane_qmsgtype_e
{
    tcSnmpPlaneQMsgAvail = 0x0,
    tcSnmpPlaneQMsgNotAvail,
    tcSnmpPlaneQMsgError
};
typedef enum _tc_snmpplane_qmsgtype_e
             tc_snmpplane_qmsgtype_e;


struct _tc_snmpplane_monactv_s
{
    CHAR                        strRedirAddr[64];
    BOOL                        bIsRedirUp;
    BOOL                        bOldRedirIsUp;
};
typedef struct _tc_snmpplane_monactv_s
               tc_snmpplane_monactv_t;

struct _tc_tcplanesnmp_thread_ctxt_s
{
    /****** Thread Init ****************/
    U32                       tid;
    struct tm*                tGMUptime;
    tc_gd_time_t              tUptime;
    tc_gd_time_t              tDowntime;
    BOOL                      bExit;
    msem_t                    tCondVarSem;
    BOOL                      bCondVarSet;
    pthread_t                 tcplane_snmp_thread;
    /****** config and init Flags ****************/
    tc_ldcfg_t                tConfig;
    U32                       nSimSendWThreadsNum;
    BOOL                      bOldRedirIsUp;
    /****** QUEUES Management ****************/
    /* Mib -> Bkgrnd thread */
    evlog_desc_t              tLogDescSys;
    evlog_t*                  pQMibToBkgrnd;
    /* Other threads -> Mib thread */
    lkfq_tc_t*                pQMib[TRANSC_MIB_QUEUE_MAX];
    /****** Statistics counters ****************/
    /* N/A */
    /****** temporary variables Management */
    /* N/A */
    tc_mtxtbl_mhlthtomib_t    tTrHealth;
};
typedef struct _tc_tcplanesnmp_thread_ctxt_s
               tc_tcplanesnmp_thread_ctxt_t;

CCUR_PROTECTED(tc_tr_sts_e)
tcPlaneSnmpGetTrSts(tc_tcplanesnmp_thread_ctxt_t*        pCntx);

CCUR_PROTECTED(tresult_t)
tcPlaneSnmpReLoadCfgYaml(
        tc_tcplanesnmp_thread_ctxt_t*        pCntx);

CCUR_PROTECTED(tresult_t)
tcPlaneSnmpReLoadSysYaml(
        tc_tcplanesnmp_thread_ctxt_t*        pCntx);

tc_snmpplane_qmsgtype_e
tcPlaneSnmpReadPPToMibQ(tc_tcplanesnmp_thread_ctxt_t* context,
                     tc_qmsgtbl_comptomib_t*   pMsg);

tc_snmpplane_qmsgtype_e
tcPlaneSnmpReadHPToMibQ(tc_tcplanesnmp_thread_ctxt_t* context,
                  tc_qmsgtbl_comptomib_t*   pMsg);

tc_snmpplane_qmsgtype_e
tcPlaneSnmpReadPGToMibQ(tc_tcplanesnmp_thread_ctxt_t* context,
                  tc_qmsgtbl_comptomib_t*   pMsg);

CCUR_PROTECTED(void)
tcPlaneSnmpLogString(tc_tcplanesnmp_thread_ctxt_t* pCntx,
                     evlog_loglvl_e                lvl,
                     CHAR*                         msg);

CCUR_PROTECTED(BOOL)
tcPlaneSnmpGetThreadExit(tc_tcplanesnmp_thread_ctxt_t* pCntx);

#ifdef __cplusplus
}
#endif
#endif
