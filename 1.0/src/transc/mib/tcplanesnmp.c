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

#include "mib.h"

/***************************************************************************
 * function: _tcPlaneSnmpPktPrcReLoadConfigYaml
 *
 * description: loading new config.yaml and signals component thread
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcPlaneSnmpPktPrcReLoadConfigYaml(
        tc_tcplanesnmp_thread_ctxt_t*       pCntx,
        tc_ldcfg_conf_t*                    pTempNewCfg)
{
    tresult_t                _result;

    CCURASSERT(pCntx);

    do
    {
        _result = ESUCCESS;
        if('\0' == pTempNewCfg->strVer[0] ||
           !tcUtilsValidConfigVersion(pTempNewCfg->strVer))
        {
            evLogTrace(
                    pCntx->pQMibToBkgrnd,
                    evLogLvlFatal,
                    &(pCntx->tLogDescSys),
                    "Config file load failure, incompatible version!"
                    " executable is ver %s while config.yaml is"
                    " ver %s\n",TRANSC_MAIN_CONF_VER,pTempNewCfg->strVer);
            break;
        }
        evLogTrace(pCntx->pQMibToBkgrnd,
                   evLogLvlInfo,
                   &(pCntx->tLogDescSys),
                   "Reloading New config.yaml");
        /* TODO: reloading per component if
         * changes are made to the component
         * instead of reloading everything */
        /* Send config.yaml file */
        tcShProtectedDPushConfigYaml(pTempNewCfg,
                            pCntx->nSimSendWThreadsNum);
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcPlaneSnmpPktPrcReLoadSysYaml
 *
 * description: loading new sys.yaml and signals component thread
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcPlaneSnmpPktPrcReLoadSysYaml(
        tc_tcplanesnmp_thread_ctxt_t*  pCntx,
        tc_ldsyscfg_conf_t*            pTempNewCfg)
{
    tresult_t                _result;

    CCURASSERT(pCntx);

    do
    {
        _result = ESUCCESS;
#if 0
        /* Check version number on the new config */
        if(!tcUtilsValidConfigVersion(pTempNewCfg->strVer))
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQMibToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Config file load failure, incompatible version!",
                    " executable is ver %s while config.yaml is"
                    " ver %s\n",TRANSC_MAIN_CONF_VER,pTempNewCfg->strVer);
            break;
        }
#endif
        evLogTrace(
                    pCntx->pQMibToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Reloading New sys.yaml");
        tcShProtectedDPushSysYaml(pTempNewCfg,
                pCntx->nSimSendWThreadsNum);
    }while(FALSE);

    return _result;
}

/**************** PROTECTED Functions **********************/
CCUR_PROTECTED(void)
tcPlaneSnmpUpdateHealthSts(
        tc_tcplanesnmp_thread_ctxt_t*        pCntx)
{
    tc_shared_healthmsg_t*    _pHlthToMibMsg;
    pthread_mutex_lock(tcShDGetCompMutex(tcTRCompTypeMib));
    _pHlthToMibMsg = tcShDGetPktGenHealthMsg();
    memcpy(&(pCntx->tTrHealth),
            &(_pHlthToMibMsg->tTrHealth),
           sizeof(pCntx->tTrHealth));
    pthread_mutex_unlock(tcShDGetCompMutex(tcTRCompTypeMib));
}

/***************************************************************************
 * function: tcPlaneSnmpGetModeOfOperationTblSz
 *
 * description: Get mode of operation table size.
 ***************************************************************************/
CCUR_PROTECTED(U16)
tcPlaneSnmpGetModeOfOperationTblSz(tc_tcplanesnmp_thread_ctxt_t* pCntx)
{
    return pCntx->tTrHealth.nTotal;
}

/***************************************************************************
 * function: tcPlaneSnmpGetModeOfOperationEntry
 *
 * description: Get mode of operation Entry given index size and string.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPlaneSnmpGetModeOfOperationEntry(tc_tcplanesnmp_thread_ctxt_t* pCntx,
        U16 idx,CHAR* strMsg,U32 nstrMsg)
{
    if(idx < pCntx->tTrHealth.nTotal)
        ccur_strlcpy(strMsg,pCntx->tTrHealth.tIntfMapActvTbl[idx],nstrMsg);
    else
        strMsg[0] = '\0';
}

/***************************************************************************
 * function: tcPlaneSnmpGetThreadExit
 *
 * description: Get thread exit status.
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcPlaneSnmpGetThreadExit(tc_tcplanesnmp_thread_ctxt_t*        pCntx)
{return pCntx->bExit;}

/***************************************************************************
 * function: tcPlaneSnmpLogString
 *
 * description: dumps string into log file
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPlaneSnmpLogString(
        tc_tcplanesnmp_thread_ctxt_t* pCntx,
        evlog_loglvl_e                lvl,
        CHAR*                         msg)
{
    evLogTrace(
            pCntx->pQMibToBkgrnd,
            lvl,
            &(pCntx->tLogDescSys),
            msg);
}

/***************************************************************************
 * function: tcPlaneSnmpReLoadCfgYaml
 *
 * description: reloads config.yaml config file
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPlaneSnmpReLoadCfgYaml(
        tc_tcplanesnmp_thread_ctxt_t*        pCntx)
{
    tresult_t               _result;
    tc_ldcfg_conf_t*        _pOldCfg;
    tc_ldcfg_conf_t*        _pTempNewCfg;

    CCURASSERT(pCntx);

    do
    {
        _result = EFAILURE;
        _pTempNewCfg = (tc_ldcfg_conf_t*)
                calloc(1,sizeof(tc_ldcfg_conf_t));
        if(NULL == _pTempNewCfg)
            break;
        _result = tcLoadUnmarshallConfigYaml(
                pCntx->pQMibToBkgrnd,
                &(pCntx->tLogDescSys),
                pCntx->tConfig.strCmdArgConfigYamlLoc,
                _pTempNewCfg,
                &(pCntx->tConfig.tConfigYamlParser));
        if(ESUCCESS != _result)
        {
            evLogTrace(
                    pCntx->pQMibToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                   "Failure Reading config.yaml file\n");
            break;
        }
        /* Generic reload config for all
         * components. */
        _pOldCfg = &(pCntx->tConfig.tConfigYamlLdCfg);
        /* Compare old and new config and see if
         * there are any changes. */
        if(0 == memcmp(_pOldCfg,_pTempNewCfg,
                sizeof(tc_ldcfg_conf_t)))
        {
            evLogTrace(
                    pCntx->pQMibToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                   "No changes in the config file:%s",
                   pCntx->tConfig.strCmdArgConfigYamlLoc);
            break;
        }
        _result = _tcPlaneSnmpPktPrcReLoadConfigYaml(
                pCntx,_pTempNewCfg);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                    pCntx->pQMibToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                   "Failure Reloading config.yaml file\n");
            break;
        }
        memcpy(_pOldCfg,_pTempNewCfg,
                sizeof(tc_ldcfg_conf_t));
    }while(FALSE);

    if(_pTempNewCfg)
        free(_pTempNewCfg);

    return _result;
}

/***************************************************************************
 * function: tcPlaneSnmpReLoadSysCfg
 *
 * description: reloading sys.yaml config file.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPlaneSnmpReLoadSysYaml(
        tc_tcplanesnmp_thread_ctxt_t*        pCntx)
{
    tresult_t                  _result;
    tc_ldsyscfg_conf_t*        _pOldCfg;
    tc_ldsyscfg_conf_t*        _pTempNewCfg;

    CCURASSERT(pCntx);

    do
    {
        _result = EFAILURE;
        _pTempNewCfg = (tc_ldsyscfg_conf_t*)
                calloc(1,sizeof(tc_ldsyscfg_conf_t));
        if(NULL == _pTempNewCfg)
            break;
        _result = tcLoadUnmarshallSysYaml(
                pCntx->pQMibToBkgrnd,
                &(pCntx->tLogDescSys),
                pCntx->tConfig.strCmdArgSysYamlLoc,
                _pTempNewCfg,
                &(pCntx->tConfig.tSysYamlParser));
        if(ESUCCESS != _result)
        {
            evLogTrace(
                    pCntx->pQMibToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                   "Failure Reading sys.yaml file\n");
            break;
        }
        /* Generic reload config for all
         * components. */
        _pOldCfg = &(pCntx->tConfig.tSysYamlLdCfg);
        /* Compare old and new config and see if
         * there are any changes. */
        if(0 == memcmp(_pOldCfg,_pTempNewCfg,
                sizeof(tc_ldsyscfg_conf_t)))
        {
            evLogTrace(
                    pCntx->pQMibToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                   "No changes in the config file:%s",
                   pCntx->tConfig.strCmdArgSysYamlLoc);
            break;
        }
        _result = _tcPlaneSnmpPktPrcReLoadSysYaml(
                pCntx,_pTempNewCfg);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                    pCntx->pQMibToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                   "Failure Reloading config file\n");
            break;
        }
        memcpy(_pOldCfg,_pTempNewCfg,
                sizeof(tc_ldsyscfg_conf_t));
    }while(FALSE);

    if(_pTempNewCfg)
        free(_pTempNewCfg);

    return _result;
}

/***************************************************************************
 *  function: tcPlaneSnmpGetVersion
 *
 *  description: Get control plane version.
 ***************************************************************************/
CCUR_PROTECTED(CHAR*)
tcPlaneSnmpGetVersion(
        tc_tcplanesnmp_thread_ctxt_t*       pCntx,
        CHAR*                       strBuff,
        I32                         nBuff)
{
    CCURASSERT(pCntx);
    CCURASSERT(strBuff);

    ccur_strlcpy(strBuff,pCntx->tConfig.tConfigYamlLdCfg.strVer,nBuff);

    return strBuff;
}

/***************************************************************************
 * function: tcPlaneSnmpGetTrSts
 *
 * description: Get the program status by checking pkt proc, http and pkt gen
 * thread status.
 ***************************************************************************/
CCUR_PROTECTED(tc_tr_sts_e)
tcPlaneSnmpGetTrSts(tc_tcplanesnmp_thread_ctxt_t*        pCntx)
{
    return pCntx->tTrHealth.eTrSts;
}

/***************************************************************************
 * function: tcPlaneSnmpReadPPToMibQ
 *
 * description: Read stats msg Queue from Packet proc thread
 ***************************************************************************/
tc_snmpplane_qmsgtype_e
tcPlaneSnmpReadPPToMibQ(tc_tcplanesnmp_thread_ctxt_t* context,
                  tc_qmsgtbl_comptomib_t*   pMsg)
{
    tc_snmpplane_qmsgtype_e             _result;
    tc_qmsgtbl_comptomib_t*             _pQMsg;

    _result = tcSnmpPlaneQMsgNotAvail;
    if(context->pQMib[tcTRPlaneSnmpCompTypePktPrc])
    {
        _pQMsg = (tc_qmsgtbl_comptomib_t*)lkfqRead(
                context->pQMib[tcTRPlaneSnmpCompTypePktPrc]);
        if(_pQMsg)
        {
            memcpy(pMsg,_pQMsg,sizeof(tc_qmsgtbl_comptomib_t));
            lkfqReadRelease(
                context->pQMib[tcTRPlaneSnmpCompTypePktPrc],(lkfq_data_p)_pQMsg);
            _result = tcSnmpPlaneQMsgAvail;
        }
    }

    return _result;
}

/***************************************************************************
 * function: tcPlaneSnmpReadHPToMibQ
 *
 * description: Read stats msg Queue from Http proc thread
 ***************************************************************************/
tc_snmpplane_qmsgtype_e
tcPlaneSnmpReadHPToMibQ(tc_tcplanesnmp_thread_ctxt_t*   context,
                  tc_qmsgtbl_comptomib_t*               pMsg)
{
    tc_snmpplane_qmsgtype_e             _result;
    tc_qmsgtbl_comptomib_t*             _pQMsg;

    _result = tcSnmpPlaneQMsgNotAvail;
    if(context->pQMib[tcTRPlaneSnmpCompTypeHttpPrc])
    {
        _pQMsg = (tc_qmsgtbl_comptomib_t*)lkfqRead(
                context->pQMib[tcTRPlaneSnmpCompTypeHttpPrc]);
        if(_pQMsg)
        {
            memcpy(pMsg,_pQMsg,sizeof(tc_qmsgtbl_comptomib_t));
            lkfqReadRelease(
                context->pQMib[tcTRPlaneSnmpCompTypeHttpPrc],(lkfq_data_p)_pQMsg);
            _result = tcSnmpPlaneQMsgAvail;
        }
    }

    return _result;
}

/***************************************************************************
 * function: tcPlaneSnmpReadPGToMibQ
 *
 * description: Read stats msg Queue from Pkt Gen thread
 ***************************************************************************/
tc_snmpplane_qmsgtype_e
tcPlaneSnmpReadPGToMibQ(tc_tcplanesnmp_thread_ctxt_t* context,
                  tc_qmsgtbl_comptomib_t*             pMsg)
{
    return tcSnmpPlaneQMsgNotAvail;
}

/***************************************************************************
 * function: tcPlaneSnmpInitRes
 *
 * description: Snmp plane resource C lang initialization
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPlaneSnmpInitRes(tc_tcplanesnmp_thread_ctxt_t* pCntx)
{
    U32                             _retry;
    U16                             _i;
    time_t                          _tnow;
    CHAR                            _strMsg[128];
    U16                             _nTblSz;
    /****** 1. Thread Common Init ********/
    _tnow = time(0);
    pCntx->tGMUptime =
            gmtime(&_tnow);
    tUtilUTCTimeGet(&(pCntx->tUptime));
    pCntx->tid = (U32)pthread_self();
    /* Sync Queues */
    lkfqSyncQ(pCntx->pQMib[tcTRPlaneSnmpCompTypePktPrc]);
    lkfqSyncQ(pCntx->pQMib[tcTRPlaneSnmpCompTypeHttpPrc]);
    lkfqSyncQ(&(pCntx->pQMibToBkgrnd->tLkfq));

    /****** 2. Thread Resource Init  ********/
    /* TODO: Move Queue init here */
    /* ... */
    /****** 3. Thread Synchronization  ********/
    _retry = 0;
    while (!(pCntx->bExit))
    {
        if(tcShProtectedDMsgIsCompInitRdy(
                tcTRCompTypeHealth))
            break;
        else
        {
            if(_retry >= 5)
                break;
            tcPlaneSnmpUpdateHealthSts(pCntx);
            tcShProtectedDMsgSetCompSyncRdy(
                    tcTRCompTypeMib,TRUE);
            sleep(1);
            _retry++;
        }
    }
    _nTblSz = tcPlaneSnmpGetModeOfOperationTblSz(pCntx);
    for(_i=0;_i<_nTblSz;_i++)
    {
        tcPlaneSnmpGetModeOfOperationEntry(pCntx,
                _i,_strMsg,sizeof(_strMsg));
        evLogTrace(
                pCntx->pQMibToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                _strMsg);
    }
    tcShProtectedDMsgSetCompInitRdy(
            tcTRCompTypeMib,TRUE);
    tcShProtectedDSetCompSts(tcTRCompTypeMib,tcTrStsUp);
    /****** 4. Thread Process ********/
    /* Block here until signaled by main thread,
     * This will allow other thread, Queue, Resource
     * to be initialized. */
    while (!(pCntx->bExit))
    {
        if (ESUCCESS == mSemCondVarSemWaitTimed(
                      &(pCntx->tCondVarSem),
                      TRANSC_SNMPPLANE_WAITTIMEOUT_MS))
            break;
    }
    evLogTrace(
            pCntx->pQMibToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "Snmp Plane Thd is running with TID#%x",(U32)pthread_self());
}

/***************************************************************************
 * function: tcPlaneSnmpExit
 *
 * description: Snmp plane resource C lang exit
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPlaneSnmpExit(tc_tcplanesnmp_thread_ctxt_t* pCntx)
{
    /****** 5. Thread Resource Destroy ********/
    /* ... */
    /****** 6. Exit application ********/
    tcShProtectedDSetCompSts(tcTRCompTypeMib,tcTrStsDown);
    tcShProtectedDSetAppExitSts(pCntx->bExit);
}
