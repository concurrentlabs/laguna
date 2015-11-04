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

#include "tcinit.h"
#include "tcshared.h"

/* Runtime value shared between threads */
/* Mutex */
static pthread_mutex_t          g_tCommonMutex;
static pthread_mutex_t          g_tCompMutexTbl[tcTRCompTypeMax];
/* Init */
static tc_tr_sts_e         g_eTrStatusTbl[tcTRCompTypeMax];
static BOOL                     g_bCompInitTbl[tcTRCompTypeMax];
static BOOL                     g_bCompSyncTbl[tcTRCompTypeMax];
/* Config */
static tc_shared_cfgmsg_t       g_tConfigYaml;
static tc_shared_syscfgmsg_t    g_tSysYaml;
/* Process exit */
static BOOL                     g_bAppExit;
static BOOL                     g_Exit;
/* Health and wellness message passing */
static tc_shared_healthmsg_t    g_hlthToPktGenMsg;    /* Health -> pktgen */
static tc_shared_pktprcmsg_t    g_pktprcToHealthMsg;  /* Pkt Prc -> Health */
static tc_shared_pktgenmsg_t    g_pktgenToHealthMsg;  /* Pkt Gen -> Health */
/* Configuration loading flags */
static BOOL                     g_CfgYamlLoadTbl[tcTRCompTypeMax];
static BOOL                     g_SysYamlLoadTbl[tcTRCompTypeMax];

/***************************************************************************
 * function: Accessors
 *
 * description: Accessors to unprotected globally shared values.
 ***************************************************************************/
CCUR_PROTECTED(pthread_mutex_t*)            tcShDGetCommonMutex(){return(&g_tCommonMutex);}
CCUR_PROTECTED(tc_shared_cfgmsg_t*)         tcShDGetCfgYamlDesc(){return(&g_tConfigYaml);}
CCUR_PROTECTED(tc_shared_syscfgmsg_t*)      tcShDGetSysYamlDesc(){return(&g_tSysYaml);}
CCUR_PROTECTED(void)                        tcShDMsgBkgrndSetExitSts(BOOL sts){g_Exit=sts;}
CCUR_PROTECTED(BOOL)                        tcShDMsgBkgrndGetExitSts(){return(g_Exit);}
CCUR_PROTECTED(tc_shared_pktprcmsg_t*)      tcShDGetPktPrcMsg(){ return &g_pktprcToHealthMsg;}
CCUR_PROTECTED(tc_shared_pktgenmsg_t*)      tcShDGetPktGenMsg(){ return &g_pktgenToHealthMsg;}
CCUR_PROTECTED(tc_shared_healthmsg_t*)      tcShDGetPktGenHealthMsg(tc_tr_comptype_e comp){return &g_hlthToPktGenMsg;}

/***************************************************************************
 * function: tcShDGetCompMutex
 *
 * description: Get component mutex
 ***************************************************************************/
CCUR_PROTECTED(pthread_mutex_t*)
tcShDGetCompMutex(tc_tr_comptype_e comp)
{
    pthread_mutex_t* _pMtx;

    _pMtx = &g_tCommonMutex;
    if(comp < tcTRCompTypeMax)
        _pMtx = &(g_tCompMutexTbl[comp]);
    return _pMtx;
}

/***************************************************************************
 * function: tcShProtectedDSetAppExitSts
 *
 * description: Set application exit status. mtx protected.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcShProtectedDSetAppExitSts(BOOL sts)
{
    pthread_mutex_lock(&g_tCommonMutex);
    g_bAppExit = sts;
    pthread_mutex_unlock(&g_tCommonMutex);
}

/***************************************************************************
 * function: tcShProtectedDGetAppExitSts
 *
 * description: Get application exit status. mtx protected.
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcShProtectedDGetAppExitSts(BOOL lk)
{
    BOOL    _sts;
    if(lk)
        pthread_mutex_lock(&g_tCommonMutex);
    _sts = g_bAppExit;
    if(lk)
        pthread_mutex_unlock(&g_tCommonMutex);
    return _sts;
}

/***************************************************************************
 * function: tcShProtectedDGetCompSts
 *
 * description: Get component status. mtx protected.
 ***************************************************************************/
CCUR_PROTECTED(tc_tr_sts_e)
tcShProtectedDGetCompSts(tc_tr_comptype_e comp)
{
    tc_tr_sts_e _eSts;

    _eSts = tcTrStsDown;
    pthread_mutex_lock(tcShDGetCompMutex(comp));
    if(comp < tcTRCompTypeMax)
        _eSts = g_eTrStatusTbl[comp];
    pthread_mutex_unlock(tcShDGetCompMutex(comp));
    return _eSts;
}

/***************************************************************************
 * function: tcShProtectedDSetCompSts
 *
 * description: Set component status. mtx protected.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcShProtectedDSetCompSts(tc_tr_comptype_e comp,
                tc_tr_sts_e sts)
{
    pthread_mutex_lock(tcShDGetCompMutex(comp));
    if(comp < tcTRCompTypeMax)
        g_eTrStatusTbl[comp] = sts;
    pthread_mutex_unlock(tcShDGetCompMutex(comp));
}

/***************************************************************************
 * function: tcShProtectedDMsgIsCompInitRdy
 *
 * description: Check if component is fully initialized. mtx protected.
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcShProtectedDMsgIsCompInitRdy(tc_tr_comptype_e comp)
{
    BOOL                _bInitSts;
    _bInitSts   = FALSE;
    pthread_mutex_lock(tcShDGetCompMutex(comp));
    if(comp < tcTRCompTypeMax)
        _bInitSts = g_bCompInitTbl[comp];
    pthread_mutex_unlock(tcShDGetCompMutex(comp));

    return _bInitSts;
}

/***************************************************************************
 * function: tcShProtectedDMsgSetCompInitRdy
 *
 * description: Set component initialization status to ready.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcShProtectedDMsgSetCompInitRdy(tc_tr_comptype_e comp, BOOL bInit)
{
    pthread_mutex_lock(tcShDGetCompMutex(comp));
    if(comp < tcTRCompTypeMax)
        g_bCompInitTbl[comp] = bInit;
    pthread_mutex_unlock(tcShDGetCompMutex(comp));
}

/***************************************************************************
 * function: tcShProtectedDMsgIsCompSyncRdy
 *
 * description: check if component synchronization flag. mtx protected.
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcShProtectedDMsgIsCompSyncRdy(tc_tr_comptype_e comp)
{
    BOOL                _bInitSts;
    _bInitSts   = FALSE;
    pthread_mutex_lock(tcShDGetCompMutex(comp));
    if(comp < tcTRCompTypeMax)
        _bInitSts = g_bCompSyncTbl[comp];
    pthread_mutex_unlock(tcShDGetCompMutex(comp));

    return _bInitSts;
}

/***************************************************************************
 * function: tcShProtectedDMsgSetCompSyncRdy
 *
 * description: set component synchronization flag. mtx protected.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcShProtectedDMsgSetCompSyncRdy(tc_tr_comptype_e comp, BOOL bSync)
{
    pthread_mutex_lock(tcShDGetCompMutex(comp));
    if(comp < tcTRCompTypeMax)
        g_bCompSyncTbl[comp] = bSync;
    pthread_mutex_unlock(tcShDGetCompMutex(comp));
}

/***************************************************************************
 * function: tcShProtectedGetSigCfgYamlLoadSts
 *
 * description: Get config.yaml status
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcShProtectedGetSigCfgYamlLoadSts(tc_tr_comptype_e comp)
{return tcUtilMemBarrierBOOLSetRead(&(g_CfgYamlLoadTbl[comp]));}

/***************************************************************************
 * function: tcShProtectedSetSigCfgYamlLoadSts
 *
 * description: Set config.yaml status
 ***************************************************************************/
CCUR_PROTECTED(void)
tcShProtectedSetSigCfgYamlLoadSts(tc_tr_comptype_e comp,BOOL bSts)
{tcUtilMemBarrierBOOLSetStore(&(g_CfgYamlLoadTbl[comp]),bSts);}

/***************************************************************************
 * function: tcShProtectedGetSigSysYamlLoadSts
 *
 * description: Get config.yaml status
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcShProtectedGetSigSysYamlLoadSts(tc_tr_comptype_e comp)
{return tcUtilMemBarrierBOOLSetRead(&(g_SysYamlLoadTbl[comp]));}

/***************************************************************************
 * function: tcShProtectedSetSigSysYamlLoadSts
 *
 * description: Set config.yaml status
 ***************************************************************************/
CCUR_PROTECTED(void)
tcShProtectedSetSigSysYamlLoadSts(tc_tr_comptype_e comp,BOOL bSts)
{tcUtilMemBarrierBOOLSetStore(&(g_SysYamlLoadTbl[comp]),bSts);}

/***************************************************************************
 * function: tcShProtectedDPushConfigYaml
 *
 * description: loading new config.yaml configuration
 ***************************************************************************/
CCUR_PROTECTED(void)
tcShProtectedDPushConfigYaml(
        tc_ldcfg_conf_t*           pTmpNewCfg,
        U32                        nSimSendWThreadsNum)
{
    U32     _i;

    pthread_mutex_lock(&(g_tConfigYaml.tCfgMutex));
    ccur_memclear(&(g_tConfigYaml.tNewConfig),sizeof(tc_ldcfg_conf_t));
    memcpy(&(g_tConfigYaml.tNewConfig),pTmpNewCfg,sizeof(tc_ldcfg_conf_t));
    g_tConfigYaml.bReloadCompTbl[tcTRCompTypeCfgConfigYamlPktPrc]    = TRUE;
    g_tConfigYaml.bReloadCompTbl[tcTRCompTypeCfgConfigYamlHttpPrc]   = TRUE;
    g_tConfigYaml.bReloadCompTbl[tcTRCompTypeCfgConfigYamlPktGen]    = TRUE;
    g_tConfigYaml.bReloadCompTbl[tcTRCompTypeCfgConfigYamlSimMgr]    = TRUE;
    for(_i=0;_i<nSimSendWThreadsNum;_i++)
    {
        if(_i < TRANSC_SIM_THD_MAX)
            g_tConfigYaml.bReloadCompTblSimSndTbl[_i] = TRUE;
    }
    pthread_mutex_unlock(&(g_tConfigYaml.tCfgMutex));
    /* Protected by memory barrier */
    tcShProtectedSetSigCfgYamlLoadSts(tcTRCompTypePktPrc,TRUE);
    tcShProtectedSetSigCfgYamlLoadSts(tcTRCompTypeHttpPrc,TRUE);
    tcShProtectedSetSigCfgYamlLoadSts(tcTRCompTypePktGen,TRUE);
    tcShProtectedSetSigCfgYamlLoadSts(tcTRCompTypeSimMgr,TRUE);
    /* Sim manager will tell the workers to update */
}

/***************************************************************************
 * function: tcShProtectedDPushSysYaml
 *
 * description: loading new sys.yaml configuration
 ***************************************************************************/
CCUR_PROTECTED(void)
tcShProtectedDPushSysYaml(
        tc_ldsyscfg_conf_t*           pTmpNewCfg,
        U32                           nSimSendWThreadsNum)
{
    pthread_mutex_lock(&(g_tSysYaml.tCfgMutex));
    ccur_memclear(&(g_tSysYaml.tNewConfig),sizeof(tc_ldsyscfg_conf_t));
    memcpy(&(g_tSysYaml.tNewConfig),pTmpNewCfg,sizeof(tc_ldsyscfg_conf_t));
    g_tSysYaml.bReloadCompTbl[tcTRCompTypeCfgSysYamlPktGen]       = TRUE;
    pthread_mutex_unlock(&(g_tSysYaml.tCfgMutex));
    /* Protected by memory barrier */
    tcShProtectedSetSigSysYamlLoadSts(tcTRCompTypePktGen,TRUE);
}
