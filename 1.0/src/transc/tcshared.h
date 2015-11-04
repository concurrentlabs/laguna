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

#ifndef TCSHARED_H
#define TCSHARED_H

#include "helper.h"
#include "tcmtxmsg.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Component config load types */
enum _tc_tcs_comptypecfg_e
{
    tcTRCompTypeCfgConfigYamlPktPrc,
    tcTRCompTypeCfgConfigYamlHttpPrc,
    tcTRCompTypeCfgConfigYamlPktGen,
    tcTRCompTypeCfgSysYamlPktGen,
    tcTRCompTypeCfgConfigYamlSimMgr,
    tcTRCompTypeCfgConfigYamlMib,
    tcTRCompTypeCfgConfigYamlLog,
    tcTRCompTypeCfgMax
};
typedef enum _tc_tcs_comptypecfg_e
             tc_tcs_comptypecfg_e;


#define TRANSC_LDCFG_SIMSND_MAX \
        tcTRCompTypeCfgMax+TRANSC_SIM_THD_MAX


struct _tc_shared_cfgmsg_s
{
    pthread_mutex_t           tCfgMutex;
    tc_ldcfg_conf_t           tNewConfig;
    BOOL                      bReloadCompTbl[tcTRCompTypeCfgMax];
    BOOL                      bReloadCompTblSimSndTbl[TRANSC_SIM_THD_MAX];
};
typedef struct _tc_shared_cfgmsg_s
               tc_shared_cfgmsg_t;

struct _tc_shared_syscfgmsg_s
{
    pthread_mutex_t           tCfgMutex;
    tc_ldsyscfg_conf_t        tNewConfig;
    BOOL                      bReloadCompTbl[tcTRCompTypeCfgMax];
    BOOL                      bReloadCompTblSimSndTbl[TRANSC_SIM_THD_MAX];
};
typedef struct _tc_shared_syscfgmsg_s
               tc_shared_syscfgmsg_t;

/****Shared Values locked ****/
struct _tc_shared_pktprcmsg_s
{
    tc_mtxtbl_ipptohlth_t   tMonIntf;
    BOOL                    bMonIntfUpdate;
};
typedef struct _tc_shared_pktprcmsg_s
               tc_shared_pktprcmsg_t;

/****Shared Values locked ****/
struct _tc_shared_pktgenmsg_s
{
    tc_mtxtbl_stspgentohlth_t   tSts;
    BOOL                        bStsUpdate;
    tc_mtxtbl_epgentohlth_t     tOutIntf;
    BOOL                        bOutIntfUpdate;
    tc_mtxtbl_mpgentohlth_t     tMapIntf;
};
typedef struct _tc_shared_pktgenmsg_s
               tc_shared_pktgenmsg_t;

/****Shared Values locked ****/
struct _tc_shared_healthmsg_s
{
    tc_mtxtbl_mhlthtopktgen_t   tPKtGenMapIntf;
    tc_mtxtbl_mhlthtomib_t      tTrHealth;
};
typedef struct _tc_shared_healthmsg_s
               tc_shared_healthmsg_t;

CCUR_PROTECTED(pthread_mutex_t*)                tcShDGetCompMutex(tc_tr_comptype_e comp);
CCUR_PROTECTED(tc_shared_healthmsg_t*)          tcShDGetPktGenHealthMsg();
CCUR_PROTECTED(tc_shared_pktprcmsg_t*)          tcShDGetPktPrcMsg();
CCUR_PROTECTED(tc_shared_pktgenmsg_t*)          tcShDGetPktGenMsg();
CCUR_PROTECTED(tc_tr_sts_e)                tcShProtectedDGetCompSts(tc_tr_comptype_e comp);
CCUR_PROTECTED(void)                            tcShProtectedDSetCompSts(tc_tr_comptype_e comp,tc_tr_sts_e sts);
CCUR_PROTECTED(BOOL)                            tcShProtectedDMsgIsCompInitRdy(tc_tr_comptype_e comp);
CCUR_PROTECTED(void)                            tcShProtectedDMsgSetCompInitRdy(tc_tr_comptype_e comp, BOOL bInit);
CCUR_PROTECTED(BOOL)                            tcShProtectedDMsgIsCompSyncRdy(tc_tr_comptype_e comp);
CCUR_PROTECTED(void)                            tcShProtectedDMsgSetCompSyncRdy(tc_tr_comptype_e comp, BOOL bSync);
CCUR_PROTECTED(pthread_mutex_t*)                tcShDGetCommonMutex();
CCUR_PROTECTED(void)                            tcShProtectedDSetAppExitSts(BOOL sts);
CCUR_PROTECTED(BOOL)                            tcShProtectedDGetAppExitSts(BOOL lk);
CCUR_PROTECTED(void)                            tcShDMsgBkgrndSetExitSts(BOOL sts);
CCUR_PROTECTED(BOOL)                            tcShDMsgBkgrndGetExitSts();
CCUR_PROTECTED(void)                            tcShProtectedDPushConfigYaml(
                                                            tc_ldcfg_conf_t* pTmpNewCfg,
                                                            U32  nSimSendWThreadsNum);
CCUR_PROTECTED(void)                            tcShProtectedDPushSysYaml(
                                                            tc_ldsyscfg_conf_t* pTmpNewCfg,
                                                            U32  nSimSendWThreadsNum);
CCUR_PROTECTED(tc_shared_cfgmsg_t*)            tcShDGetCfgYamlDesc();
CCUR_PROTECTED(tc_shared_syscfgmsg_t*)         tcShDGetSysYamlDesc();
CCUR_PROTECTED(U32)                             tcShDGetSimSndWThreadsNum();
CCUR_PROTECTED(void)                            tcShDSetSimSndWThreadsNum(U32 num);
CCUR_PROTECTED(BOOL)                            tcShProtectedGetSigCfgYamlLoadSts(tc_tr_comptype_e comp);
CCUR_PROTECTED(void)                            tcShProtectedSetSigCfgYamlLoadSts(tc_tr_comptype_e comp,BOOL bSts);
CCUR_PROTECTED(BOOL)                            tcShProtectedGetSigSysYamlLoadSts(tc_tr_comptype_e comp);
CCUR_PROTECTED(void)                            tcShProtectedSetSigSysYamlLoadSts(tc_tr_comptype_e comp,BOOL bSts);

#ifdef __cplusplus
}
#endif
#endif /* TCSHARED_H */

