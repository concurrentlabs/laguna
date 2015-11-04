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

#include "pktgen.h"

/**************** PRIVATE Functions **********************/

/**************** PROTECTED Functions **********************/
CCUR_PROTECTED(tresult_t)
tcPktGenLogStatsSummary(
        tc_pktgen_thread_ctxt_t* pCntx)
{
    tc_gd_time_t            _tNow;
    tc_gd_time_t            _tDiffTime;
    CHAR                    _pTNowbuf[128];

    CCURASSERT(pCntx);

    strftime(_pTNowbuf, sizeof(_pTNowbuf),
            "%a, %d %b %Y %H:%M:%S %Z",
            pCntx->tGMUptime);
    tUtilUTCTimeGet(&_tNow);
    tUtilUTCTimeDiff(&_tDiffTime,
                     &_tNow,
                     &(pCntx->tUptime));
    evLogTrace(
            pCntx->pQPktGenToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "****Pkt Gen Summary TID#%x****\n"
            "Uptime:%s\n"
            "Total Running time:%ld mins\n",
            pCntx->tid,
            _pTNowbuf,
            _tDiffTime.nSeconds/60);

    return ESUCCESS;
}

/***************************************************************************
 * function: tcPktGenUpdateSharedValues
 *
 * description: Update shared values.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktGenUpdateSharedValues(tc_pktgen_thread_ctxt_t* pCntx, BOOL bRead)
{
    U32                         _i;
    tc_shared_healthmsg_t*      _pHlthToPktGenMsg;
    tc_shared_pktgenmsg_t*      _pPktGenToHlthMsg;

    if(bRead)
    {
        pthread_mutex_lock(tcShDGetCompMutex(tcTRCompTypePktGen));
        _pHlthToPktGenMsg = tcShDGetPktGenHealthMsg();
        memcpy(&(pCntx->tTmpHlthToPKtGenMapIntf),
               &(_pHlthToPktGenMsg->tPKtGenMapIntf),
               sizeof(pCntx->tTmpHlthToPKtGenMapIntf));
        pthread_mutex_unlock(tcShDGetCompMutex(tcTRCompTypePktGen));
        pCntx->tIntfX.nIntfMapTblTotal =
                pCntx->tTmpPktGenToHlthMapIntf.nTotal;
        for(_i=0;_i<pCntx->tIntfX.nIntfMapTblTotal;_i++)
        {
            pCntx->tIntfX.tIntfMapTbl[_i].bIsModeActv =
                    pCntx->tTmpHlthToPKtGenMapIntf.tIntfMapActvTbl[_i];
        }
    }
    else
    {
        if(memcmp(&(pCntx->tOldTmpPktGenToHlthSts),
           &(pCntx->tTmpPktGenToHlthSts),
           sizeof(pCntx->tOldTmpPktGenToHlthSts)))
        {
            pthread_mutex_lock(tcShDGetCompMutex(tcTRCompTypePktGen));
            _pPktGenToHlthMsg = tcShDGetPktGenMsg();
            _pPktGenToHlthMsg->bStsUpdate = TRUE;
            memcpy(&(_pPktGenToHlthMsg->tSts),
                   &(pCntx->tTmpPktGenToHlthSts),
                   sizeof(_pPktGenToHlthMsg->tSts));
            pthread_mutex_unlock(tcShDGetCompMutex(tcTRCompTypePktGen));
            /* Update the old value */
            memcpy(&(pCntx->tOldTmpPktGenToHlthSts),
                   &(pCntx->tTmpPktGenToHlthSts),
                   sizeof(pCntx->tOldTmpPktGenToHlthSts));
        }
        if(pCntx->tIntfX.bIntfCfgChanged)
        {
            pthread_mutex_lock(tcShDGetCompMutex(tcTRCompTypePktGen));
            _pPktGenToHlthMsg = tcShDGetPktGenMsg();
            _pPktGenToHlthMsg->bOutIntfUpdate = TRUE;
            memcpy(&(_pPktGenToHlthMsg->tOutIntf),
                   &(pCntx->tTmpPktGenToHlthOutIntf),
                   sizeof(_pPktGenToHlthMsg->tOutIntf));
            memcpy(&(_pPktGenToHlthMsg->tMapIntf),
                   &(pCntx->tTmpPktGenToHlthMapIntf),
                   sizeof(_pPktGenToHlthMsg->tMapIntf));
            pthread_mutex_unlock(tcShDGetCompMutex(tcTRCompTypePktGen));
            pCntx->tIntfX.bIntfCfgChanged = FALSE;
        }
    }
}

/***************************************************************************
 * function: _tcPktProcWriteHealthReport
 *
 * description: Write health report message for health thread.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcPktGenWriteHealthReport(tc_pktgen_thread_ctxt_t* pCntx)
{
    U32 _i;

    /* Copy Status message */
    pCntx->tTmpPktGenToHlthSts.bActiveMode =
            pCntx->tIntfCfg.bActiveMode;
    pCntx->tTmpPktGenToHlthSts.bRedirCapReached =
            pCntx->bRedirCapReached;
    if(pCntx->tIntfX.bIntfCfgChanged)
    {
        /* Copy all the values here */
        /* Out interface */
        pCntx->tTmpPktGenToHlthOutIntf.nOpen =
                pCntx->tIntfX.nOutIntfOpen;
        pCntx->tTmpPktGenToHlthOutIntf.nTotal =
                pCntx->tIntfX.nOutIntfTotal;
        for(_i=0;_i<pCntx->tIntfX.nOutIntfTotal;_i++)
        {
            ccur_strlcpy(pCntx->tTmpPktGenToHlthOutIntf.tIntfETbl[_i].strIntfName,
                    pCntx->tIntfX.tOutIntfTbl[_i].tIntf.strIntfName,
                    sizeof(pCntx->tTmpPktGenToHlthOutIntf.tIntfETbl[_i].strIntfName));
            pCntx->tTmpPktGenToHlthOutIntf.tIntfETbl[_i].bIntfRdy =
                    pCntx->tIntfX.tOutIntfTbl[_i].tIntf.bIntfRdy;
        }
        /* map interface */
        pCntx->tTmpPktGenToHlthMapIntf.nTotal =
                pCntx->tIntfX.nIntfMapTblTotal;
        for(_i=0;_i<pCntx->tIntfX.nIntfMapTblTotal;_i++)
        {
            if(pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH)
            {
                ccur_strlcpy(pCntx->tTmpPktGenToHlthMapIntf.tIntfMapTbl[_i].strMonIntfName,
                        pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->tLinkIntf.strIntfName,
                        sizeof(pCntx->tTmpPktGenToHlthMapIntf.tIntfMapTbl[_i].strMonIntfName));
            }
            pCntx->tTmpPktGenToHlthMapIntf.tIntfMapTbl[_i].bEgressIdxSet =
                    FALSE;
            if(pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf)
            {
                pCntx->tTmpPktGenToHlthMapIntf.tIntfMapTbl[_i].nEgressIdx =
                        pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.nIntfIdx;
                if(pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.bIntfIdxSet)
                    pCntx->tTmpPktGenToHlthMapIntf.tIntfMapTbl[_i].bEgressIdxSet =
                            TRUE;
            }
            pCntx->tTmpPktGenToHlthMapIntf.tIntfMapTbl[_i].bLinked =
                    pCntx->tIntfX.tIntfMapTbl[_i].bLinked;
            pCntx->tTmpPktGenToHlthMapIntf.tIntfMapTbl[_i].pMonActv = NULL;
        }
        pCntx->bSendHealthReport = TRUE;
    }
}

/***************************************************************************
 * function: tcHttpProcReadCfg
 *
 * description: Check to see if flag is set and reload the config file.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktGenReadCfg(tc_pktgen_thread_ctxt_t*  pCntx, BOOL bForceUpdShVal)
{
    tresult_t                   _result;
    tc_ldcfg_conf_t*            _pNewConfigYamlCfg;
    tc_shared_cfgmsg_t*         _pConfigYamlCfgDesc;
    tc_ldsyscfg_conf_t*         _pNewSysYamlCfg;
    tc_shared_syscfgmsg_t*      _pSysYamlCfgDesc;
    BOOL                        _bLoad = FALSE;

    _pConfigYamlCfgDesc = tcShDGetCfgYamlDesc();
    if(tcShProtectedGetSigCfgYamlLoadSts(tcTRCompTypePktGen))
    {
        pthread_mutex_lock(&(_pConfigYamlCfgDesc->tCfgMutex));
        if(_pConfigYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgConfigYamlPktGen])
        {
            _bLoad = TRUE;
            _pNewConfigYamlCfg    = &(_pConfigYamlCfgDesc->tNewConfig);
            _result = tcPktGenConfigInitLoadableRes(pCntx,_pNewConfigYamlCfg);
            if(ESUCCESS == _result)
            {
                evLogTrace(
                        pCntx->pQPktGenToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "New config.yaml config loaded successfully by Pkt Gen thread TID#%x",pCntx->tid);
            }
            else
            {
                evLogTrace(pCntx->pQPktGenToBkgrnd,
                        evLogLvlError,
                        &(pCntx->tLogDescSys),
                        "New config.yaml config loading failure by Pkt Gen thread TID#%x",pCntx->tid);
            }
            if(pCntx->bIntfCfgLoadSuccess)
            {
                /* Overloads Yaml MAC address with values from config.yaml if exists */
                tcOutIntfConfigYamlInitMACAddresses(
                        &(pCntx->tIntfX),&(pCntx->tIntfCfg),
                        _pNewConfigYamlCfg);
            }
            _pConfigYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgConfigYamlPktGen] = FALSE;
        }
        pthread_mutex_unlock(&(_pConfigYamlCfgDesc->tCfgMutex));
        tcShProtectedSetSigCfgYamlLoadSts(tcTRCompTypePktGen,FALSE);
    }
    _pSysYamlCfgDesc = tcShDGetSysYamlDesc();
    if(tcShProtectedGetSigSysYamlLoadSts(tcTRCompTypePktGen))
    {
        if(pCntx->bIntfCfgLoadSuccess)
        {
            _bLoad = TRUE;
            pthread_mutex_lock(&(_pSysYamlCfgDesc->tCfgMutex));
            _pNewSysYamlCfg    = &(_pSysYamlCfgDesc->tNewConfig);
            tcOutIntfSysYamlInitMACAddresses(&(pCntx->tIntfX),
                    &(pCntx->tIntfCfg),_pNewSysYamlCfg);
            if(_pSysYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgSysYamlPktGen])
            {
                evLogTrace(
                        pCntx->pQPktGenToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "New sys.yaml config loaded by pkt gen thread TID#%x",pCntx->tid);
                _pSysYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgSysYamlPktGen] = FALSE;
            }
            pthread_mutex_unlock(&(_pSysYamlCfgDesc->tCfgMutex));
            pthread_mutex_lock(&(_pSysYamlCfgDesc->tCfgMutex));
            _pNewConfigYamlCfg    = &(_pConfigYamlCfgDesc->tNewConfig);
            /* Overloads Yaml MAC address with values from config.yaml if exists */
            tcOutIntfConfigYamlInitMACAddresses(
                    &(pCntx->tIntfX),&(pCntx->tIntfCfg),
                    _pNewConfigYamlCfg);
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "New sys.yaml config loaded successfully by Pkt Prc thread TID#%x",pCntx->tid);
            pthread_mutex_unlock(&(_pSysYamlCfgDesc->tCfgMutex));
            tcShProtectedSetSigSysYamlLoadSts(tcTRCompTypePktGen,FALSE);
        }
    }
    if(_bLoad || bForceUpdShVal)
    {
        _tcPktGenWriteHealthReport(pCntx);
        tcPktGenUpdateSharedValues(pCntx,FALSE);
        tcPktGenInitLogOutCfgLoadStatus(pCntx);
    }
}

CCUR_PROTECTED(tc_outintf_map_t*)
tcPktGenFindMapIntf(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_g_qmsghptopg_t*          pInjMsg,
        U32*                        pMapIdx)
{
    U32                 _i;
    tc_outintf_map_t*   _pMap;

    _pMap    = NULL;
    for(_i=0;_i<pCntx->tIntfX.nIntfMapTblTotal;_i++)
    {
        if(pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf)
        {
            if(!strcmp(
                    pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.strIntfName,
                    pInjMsg->strOutIntfName))
            {
                *pMapIdx = _i;
                _pMap = &(pCntx->tIntfX.tIntfMapTbl[_i]);
                break;
            }
        }
    }

    return _pMap;
}

CCUR_PROTECTED(void)
tcPktGenProcessPktGen(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_g_qmsghptopg_t*          pInjMsg,
        tc_pktdesc_t*               pPktDesc,
        hlpr_httpsite_hndl          eSite)
{
    tresult_t                   _result;
    BOOL                        _bInject;
    tc_pktgen_tcpflgproto_e     _eTcpFlg;
    U32                         _nMapIdx;
    tc_outintf_map_t*           _pMap;
    BOOL                        _bActive;

    do
    {
        _result = ESUCCESS;
        /* Active mode means we are tracking and injecting */
        _pMap = tcPktGenFindMapIntf(pCntx,pInjMsg,&_nMapIdx);
        if(_pMap)
        {
            _bActive = FALSE;
            if(_nMapIdx < TRANSC_INTERFACE_MAX)
                _bActive =
                        pCntx->tTmpHlthToPKtGenMapIntf.tIntfMapActvTbl[_nMapIdx];
            if(_bActive)
            {
                _bInject = TRUE;
                /* Don't inject if
                 * not redirecting CORS request */
                if(('\0' != pInjMsg->strCOrigin[0]) &&
                  (FALSE == pCntx->bRedirectCORSReq))
                    _bInject = FALSE;
            }
            else
            {
                _bInject = FALSE;
                if(pCntx->tIntfCfg.bActiveMode)
                {
                    _result = EFAILURE;
                    pCntx->nOutIntfMapInjectionError++;
                }
            }
        }
        else
        {
            _result = EFAILURE;
            _bInject = FALSE;
        }
        if(_bInject)
        {
            if('\0' == pInjMsg->strCRange[0])
            {
                pInjMsg->strCRange[0] = '0';
                pInjMsg->strCRange[1] = '-';
                pInjMsg->strCRange[2] = '\0';
            }
            if('\0' == pInjMsg->strCMisc[0])
            {
                pInjMsg->strCMisc[0] = '0';
                pInjMsg->strCMisc[1] = '\0';
            }
            if(!pCntx->bBlockTraffic)
            {
                /* Inject 302 packet to client */
                _result = tcPktInjInjectReq302Pkt(
                        pCntx,
                        pInjMsg,
                        pPktDesc,
                        eSite,
                        _pMap->pOutIntf,
                        _pMap->pMonIntfH);
                if(ESUCCESS != _result)
                {
                    pCntx->nPktRedirErr++;
                    break;
                }
                pCntx->nTCPRedir++;
            }
            _eTcpFlg = tcTcpFlagProtoRstAck;
            if(!TCUTIL_IS_URLABSPATH(pInjMsg->pUrl))
            {
                if(TCUTIL_IS_URLHTTPSTRING(pInjMsg->pUrl))
                    _eTcpFlg = tcTcpFlagProtoFinAck;
            }
            _result = tcPktInjInjectReqFinPkt(
                        pCntx,
                       pPktDesc,
                       eSite,
                       _eTcpFlg,
                       _pMap->pOutIntf);
            if(ESUCCESS != _result)
            {
                pCntx->nPktRedirErr++;
                break;
            }
            pCntx->nTCPRst++;
            pCntx->nTotalActvPktRedirected++;
        }
        else
                pCntx->nTotalMonPktRedirected++;
        pCntx->nRedirReqCapCntr++;
        pCntx->nTotalRedirCntr++;
    }while(FALSE);

    if(ESUCCESS == _result)
    {
        tcUtilsLogW3cFormat(
                pCntx->tid,
                evLogLvlInfo,
                pInjMsg,
                pPktDesc,
                pCntx->pQPktGenToBkgrnd,
                &(pCntx->tLogDescSvc));
    }
    else
    {
        tcUtilsLogW3cFormat(
                pCntx->tid,
                evLogLvlWarn,
                pInjMsg,
                pPktDesc,
                pCntx->pQPktGenToBkgrnd,
                &(pCntx->tLogDescSvc));
    }
}

/***************************************************************************
 * function: tcPktGenCheckRedirPerSecMax
 *
 * description: Check maximum redirection max every (rate cap/2) requests
 ***************************************************************************/
CCUR_PROTECTED(tc_gd_time_t*)
tcPktGenCheckRedirPerSecMax(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_gd_time_t*               pUpdDiff,
        tc_gd_time_t*               pNowTime,
        tc_gd_time_t*               pOldTime,
        U32*                        nReqCnt)
{
    tc_gd_time_t    _tUpdDiff;

    pUpdDiff->nSeconds = 0;
    pUpdDiff->nMicroseconds = 0;

    /* Get time every when we hit the redirection cap or
     * request sample count */
    if(*nReqCnt >= pCntx->nReqSampleCnt ||
       pCntx->nRedirReqCapCntr >= pCntx->nRedirReqRateCap)
    {
        tUtilUTCTimeGet(pNowTime);
        /* Calculate time interval between n-th requests */
        if( pNowTime->nSeconds <= pOldTime->nSeconds)
            pUpdDiff->nSeconds = 0;
        else
        {
            tUtilUTCTimeDiff(
                    pUpdDiff,
                    pNowTime,
                    pOldTime
                    );
        }
        *nReqCnt = 0;
        if(0 == pUpdDiff->nSeconds)
        {
            if(pCntx->nRedirReqCapCntr >= pCntx->nRedirReqRateCap)
            {
                ccur_memclear(&(pCntx->tRedirReqDownTime),
                        sizeof(pCntx->tRedirReqDownTime));
                evLogTrace(pCntx->pQPktGenToBkgrnd,
                        evLogLvlWarn,
                        &(pCntx->tLogDescSys),
                       "Warning! max redirection rate %d (pkts/sec) reached!",
                       pCntx->nRedirReqRateCap);
                if(FALSE == pCntx->bRedirCapReached)
                {
                    pCntx->tRedirReqDownTime = *pNowTime;
                    pCntx->bRedirCapReached = TRUE;
                    /* Half the rate cap  sample */
                    pCntx->nReqSampleCnt = (pCntx->nRedirReqRateCap >> 1);
                    if (0 == pCntx->nReqSampleCnt)
                        pCntx->nReqSampleCnt =  TRANSC_REDIRRATE_MAX;
                    evLogTrace(pCntx->pQPktGenToBkgrnd,
                            evLogLvlWarn,
                            &(pCntx->tLogDescSys),
                            "Redirection sample rate (req/sec):%lu",
                            pCntx->nReqSampleCnt);
                }
                pCntx->nRedirReqCapCntr    = 0;
            }
        }
        /* interval > 1 sec */
        else
        {
            if(pUpdDiff->nSeconds)
                *pOldTime           = *pNowTime;
            if(pCntx->bRedirCapReached)
            {
                /* Output the down time in milliseconds */
                if(pCntx->tRedirReqDownTime.nMicroseconds)
                {
                    tUtilUTCTimeDiff(
                            &_tUpdDiff,
                            pNowTime,
                            &(pCntx->tRedirReqDownTime)
                            );
                    evLogTrace(pCntx->pQPktGenToBkgrnd,
                            evLogLvlWarn,
                            &(pCntx->tLogDescSys),
                           "cap redirection req down time: %lu msecs",
                           (_tUpdDiff.nSeconds*1000)+(_tUpdDiff.nMicroseconds/1000));
                }
                pCntx->bRedirCapReached = TRUE;
            }
            /* set the sampling rate back to cap value*/
            pCntx->nReqSampleCnt = (pCntx->nRedirReqRateCap);
            if (0 == pCntx->nReqSampleCnt)
                pCntx->nReqSampleCnt =  TRANSC_REDIRRATE_MAX;
            pCntx->nRedirReqCapCntr    = 0;
        }
    }
    (*nReqCnt)++;

    return pUpdDiff;
}

/***************************************************************************
 * function: tcPktGenLogStats
 *
 * description: dump statistics at current point in time
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktGenLogStats(
        tc_pktgen_thread_ctxt_t* pCntx)
{
    tresult_t               _result;
    U32                     _i;
    CHAR                    _strBuff0[256];

    CCURASSERT(pCntx);

    _result = ESUCCESS;
    if (TRANSC_ZLOGCATEGORY_BITMAP_CK(pCntx->tLogDescSys,evLogLvlInfo))
        return _result;
    /*pCntx->tStatsDumpCkTime.reserved /=1000;
    pCntx->nRedirRate   = pCntx->nTotalRedirCntr/
            (pCntx->tStatsDumpCkTime.nSeconds+pCntx->tStatsDumpCkTime.reserved);*/
    if(0 == pCntx->tStatsDumpCkTime.nSeconds)
        pCntx->tStatsDumpCkTime.nSeconds = 1;
    pCntx->nRedirRate   = pCntx->nTotalRedirCntr/
            (pCntx->tStatsDumpCkTime.nSeconds);
    pCntx->nTotalRedirCntr          = 0;
    evLogTrace(
            pCntx->pQPktGenToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "*****Pkt Gen TID#%x Info****\n"
            "    Redirection Rate Max (redirReq/sec):%lu\n"
            "    Redirection sample Rate (req/sec):%lu\n"
            "    Redirection Rate (pkts/sec):%lu \n"
            "    Total monitor redirected:%lu\n"
            "    Total active redirected:%lu\n"
            "    Total Out intf map injection error:%lu\n"
            "    Injections:\n"
            "       TCP RST injections:%lu\n"
            "       TCP Fin injections:%lu\n"
            "       TCP 302 injections:%lu\n"
            "%s\n"
            ,
            pCntx->tid,
            pCntx->nRedirReqRateCap,
            pCntx->nReqSampleCnt,
            pCntx->nRedirRate,
            pCntx->nTotalMonPktRedirected,
            pCntx->nTotalActvPktRedirected,
            pCntx->nOutIntfMapInjectionError,
            pCntx->nTCPRst,
            pCntx->nTCPFin,
            pCntx->nTCPRedir,
            tcPktIOTxOutIntfGetStat(pCntx,_strBuff0,sizeof(_strBuff0))
            );
    for(_i=0;_i<pCntx->tIntfX.nIntfMapTblTotal;_i++)
    {
        if(pCntx->tIntfCfg.bActiveMode)
        {
            if(pCntx->tIntfX.tIntfMapTbl[_i].bIsModeActv)
            {
                if(pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->tLinkIntf.nRefCnt &&
                   pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf)
                {
                    evLogTrace(
                          pCntx->pQPktGenToBkgrnd,
                          evLogLvlInfo,
                          &(pCntx->tLogDescSys),
                          "pktgen set to: active | "
                          "actual mode of operation Link:%d/%s/<%s:N/A>-<%s:%s:%d>/active",
                          _i,
                          pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->strRedirAddr,
                          pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->tLinkIntf.strIntfName,
                          pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.strIntfName,
                          pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.strIntfVal,
                          pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.bIntfRdy);
                }
            }
            else
            {
                if(pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->tLinkIntf.nRefCnt &&
                   pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf)
                {
                    evLogTrace(
                          pCntx->pQPktGenToBkgrnd,
                          evLogLvlInfo,
                          &(pCntx->tLogDescSys),
                          "pktgen set to: active | "
                          "actual mode of operation Link:%d/%s/<%s:N/A>-<%s:%s:%d>/monitor",
                          _i,
                          pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->strRedirAddr,
                          pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->tLinkIntf.strIntfName,
                          pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.strIntfName,
                          pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.strIntfVal,
                          pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.bIntfRdy);
                }
            }
        }
        else
        {
            if(pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->tLinkIntf.nRefCnt &&
               pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf)
            {
                evLogTrace(
                      pCntx->pQPktGenToBkgrnd,
                      evLogLvlInfo,
                      &(pCntx->tLogDescSys),
                      "pktgen set to: monitor | "
                      "actual mode of operation Link:%d/%s/<%s:N/A>-<%s:%s:%d>/monitor",
                      _i,
                      pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->strRedirAddr,
                      pCntx->tIntfX.tIntfMapTbl[_i].pMonIntfH->tLinkIntf.strIntfName,
                      pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.strIntfName,
                      pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.strIntfVal,
                      pCntx->tIntfX.tIntfMapTbl[_i].pOutIntf->tIntf.bIntfRdy);
            }
        }
    }
    if(pCntx->bRedirCapReached)
    {
        evLogTrace(pCntx->pQPktGenToBkgrnd,
                evLogLvlError,
                &(pCntx->tLogDescSys),
                "Warning! max redirection rate %d (pkts/sec) reached!",
                pCntx->nRedirReqRateCap);
    }
    evLogTrace(pCntx->pQPktGenToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
           "Redirection Rate Max (redirReq/sec):%lu",
           pCntx->nRedirReqRateCap);
    evLogTrace(pCntx->pQPktGenToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
           "Redirection sample Rate (req/sec):%lu",
           pCntx->nReqSampleCnt);

    return(_result);
}

/***************************************************************************
 * function: tcPktGenTimedUpdateSharedValues
 *
 * description: Update Shared values. Read/write values from health thread.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktGenTimedUpdateSharedValues(
        tc_pktgen_thread_ctxt_t*   pCntx,
        tc_gd_time_t*              pUpdDiff)
{

    CCURASSERT(pCntx);
    CCURASSERT(pUpdDiff);

    pCntx->tStatsUpdShValCkTime.nSeconds += pUpdDiff->nSeconds;
    if( pCntx->tStatsUpdShValCkTime.nSeconds >=
            TRANSC_PKTGEN_SHVALUPD_TIME_SEC)
    {
        tcPktGenUpdateSharedValues(pCntx,TRUE);
        _tcPktGenWriteHealthReport(pCntx);
        tcPktGenUpdateSharedValues(pCntx,FALSE);
        pCntx->tStatsUpdShValCkTime.nSeconds = 0;
    }
}

/***************************************************************************
 * function: tcPktGenStatsDump
 *
 * description: dump statistics based on time specified.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktGenStatsDump(
        tc_pktgen_thread_ctxt_t*   pCntx,
        tc_gd_time_t*              pUpdDiff)
{
    CCURASSERT(pCntx);
    CCURASSERT(pUpdDiff);

    pCntx->tStatsDumpCkTime.nSeconds += pUpdDiff->nSeconds;
    /* get miliseconds value */
    /*pCntx->tStatsDumpCkTime.reserved += (pUpdDiff->nMicroseconds/1000);*/
    if( pCntx->tStatsDumpCkTime.nSeconds >=
            TRANSC_PKTGEN_DUMPSTATS_TIME_SEC )
    {
        ccur_memclear(&(pCntx->tStatsDumpCkTime),
                sizeof(pCntx->tStatsDumpCkTime));
        tcPktGenLogStats(pCntx);
    }
}
