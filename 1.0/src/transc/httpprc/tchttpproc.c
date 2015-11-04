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

#include "httpprc.h"
#include "tctemp.h"
/**************** PRIVATE Functions **********************/
/***************************************************************************
 * function: _tcHttpProcGetTimeDiff
 *
 * description: get delay time from pkt arrival to now time
 ***************************************************************************/
#if TRANSC_STRMINJTIME
CCUR_PRIVATE(void)
_tcHttpProcGetTimeDiff(
        tc_httpprc_thread_ctxt_t*        pCntx,
        tc_gd_time_t*                   pRxTime)
{
    tc_gd_time_t                _tNowTime;
    tc_gd_time_t                _tDiffTime;

    pCntx->nInjDelayCntr++;
    tUtilUTCTimeGet(&_tNowTime);
    tUtilUTCTimeDiff(&_tDiffTime,
                     &_tNowTime,
                     pRxTime);
    pCntx->tInjDelayDiffTime.nSeconds        += _tDiffTime.nSeconds;
    pCntx->tInjDelayDiffTime.nMicroseconds   += _tDiffTime.nMicroseconds;
    if(pCntx->tInjDelayDiffTime.nMicroseconds > 1000000)
    {
        /* convert to seconds */
        pCntx->tInjDelayDiffTime.nSeconds +=
                pCntx->tInjDelayDiffTime.nMicroseconds/1000000;
        pCntx->tInjDelayDiffTime.nMicroseconds %=1000000;
    }
}
#endif /* TRANSC_STRMINJTIME */

/***************************************************************************
 * function: _tcHttpProcLogStatsSummary
 *
 * description: dump statistics summary
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpProcLogStatsSummary(
        tc_httpprc_thread_ctxt_t* pCntx)
{
    tresult_t               _result;
    hlpr_httpsite_hndl      _j;
    U32                     _nInjDelayCntr;
    U32                     _nRunTimeSecs;
    U32                     _nTotalHttpPkts;
    tc_httpprc_stats_t*      _pHttpSiteRes;
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
    if(!_tDiffTime.nSeconds)
        _nRunTimeSecs          = 1;
    else
        _nRunTimeSecs          = _tDiffTime.nSeconds;
    _nInjDelayCntr = pCntx->nInjDelayCntr;
    if(!_nInjDelayCntr)
        _nInjDelayCntr = 1;

    _nTotalHttpPkts = pCntx->nHttpPktTotal;
    if(!_nTotalHttpPkts)
        _nTotalHttpPkts = 1;
    evLogTrace(
            pCntx->pQHttpProcToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "****Http Proc Summary TID#%x****\n"
            "Uptime:%s\n"
            "Total Running time:%ld mins\n"
            "Total HTTP GET processed/sec:%ld\n",
            pCntx->tid,
            _pTNowbuf,
            _tDiffTime.nSeconds/60,
            pCntx->nGetReqProcessed/_nRunTimeSecs
            );

    for(_j=1;_j<pCntx->nPPSitesTbl;_j++)
    {
        _pHttpSiteRes =
                tcHttpInitSiteGetRes(pCntx,_j);
        if(NULL == _pHttpSiteRes)
        {
            _result = EINVAL;
            break;
        }
        if(_pHttpSiteRes->nPktsToClients)
        {
            evLogTrace(
                pCntx->pQHttpProcToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "%s:\n"
                "    HTTP GET-Req/Total-pkts:%.06f\n"
                "    HTTP GET Video File requests/sec:%lu\n",
                pCntx->tPPSitesTbl[_j].strPktPrcArgSites,
                _pHttpSiteRes->nVideoFileReq/((float)(_nTotalHttpPkts-_pHttpSiteRes->nVideoFileReq)),
                _pHttpSiteRes->nVideoFileReq/_nRunTimeSecs);
        }
    }

    return(_result);
}

/***************************************************************************
 * function: _tcHttpProcLogStats
 *
 * description: dump statistics at current point in time
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpProcLogStats(
        tc_httpprc_thread_ctxt_t* pCntx)
{
    tresult_t               _result;
    hlpr_httpsite_hndl      _j;
    tc_httpprc_stats_t*      _pHttpSiteRes;
#if TRANSC_STRMINJTIME
    float                   _fInjDelayAvgSec;
    U32                     _nInjDelayCntr;
#endif /* TRANSC_STRMINJTIME */

    CCURASSERT(pCntx);

    _result = ESUCCESS;
    if (TRANSC_ZLOGCATEGORY_BITMAP_CK(pCntx->tLogDescSys,evLogLvlInfo))
        return _result;
#if TRANSC_STRMINJTIME
    _nInjDelayCntr = pCntx->nInjDelayCntr;
    if(!_nInjDelayCntr)
        _nInjDelayCntr = 1;
    /* Convert everything to micro seconds and
     * calcualte the delay in seconds */
    _fInjDelayAvgSec =
            ((pCntx->tInjDelayDiffTime.nSeconds*1000000)+
            pCntx->tInjDelayDiffTime.nMicroseconds)/1000000.0;
    _fInjDelayAvgSec /= (float)(_nInjDelayCntr);
#endif /* TRANSC_STRMINJTIME */
    evLogTrace(
            pCntx->pQHttpProcToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "****Http Proc Info TID#%x****\n"
            "       HTTP Pkts Processed:%lu\n"
            "           HTTP GET processed:%lu\n"
            "               HTTP Active Video Sess:%lu\n"
            "       HTTP Pkts Ignored:%lu\n"
            "           HTTP GET Parse Service unknown:%lu\n"
            "           HTTP GET Header Match ignored:%lu\n"
            "       HTTP Pkts Errors:%lu\n"
            "           HTTP GET process Err:%lu\n"
            "           No Q Memory HttpPrc->Sim Error:%lu\n"
#if TRANSC_STRMINJTIME
            "       Avg Injection Time:%f sec\n"
#endif
            ,
            pCntx->tid,
            pCntx->nHttpPktProcessed,
            pCntx->nGetReqProcessed,
            pCntx->nTcpStrmSessActive,
            pCntx->nHttpPktIgnored,
            pCntx->nIgnGetReqSvcUnknown,
            pCntx->nHdrMatchIgnore,
            pCntx->nHttpPktProcErr,
            pCntx->nGetReqErr,
            pCntx->nErrQNoMemHttpProcToSim
#if TRANSC_STRMINJTIME
            _fInjDelayAvgSec,
#endif
            );

    for(_j=1;_j<pCntx->nPPSitesTbl;_j++)
    {
        _pHttpSiteRes =
                tcHttpInitSiteGetRes(pCntx,_j);
        if(NULL == _pHttpSiteRes)
        {
            _result = EINVAL;
            break;
        }

        if(_pHttpSiteRes->nPktsToClients)
        {
            evLogTrace(
                pCntx->pQHttpProcToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "****Http Proc Site Info****\n"
                "%s:\n"
                "    HTTP GET Video File requests:%lu\n",
                pCntx->tPPSitesTbl[_j].strPktPrcArgSites,
                _pHttpSiteRes->nVideoFileReq);
        }
    }
    return(_result);
}


/***************************************************************************
 * function: _tcHttpProcWriteMibQRedir
 *
 * description: Write msg to Queue for mib thread
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpProcWriteMibQRedir(
        tc_httpprc_thread_ctxt_t*       pCntx,
        tc_httpparse_calldef_t*         pHttpMsg,
        tc_pktdesc_t*                   pPktDesc)
{
    tresult_t                               _result;
    CHAR                                    _tmpBuff[64];

    _result = ESUCCESS;
    if(pCntx->nRedirMsgOflw <= 0)
    {
        if(pCntx->tRedirMsg.nHttpMsgInfo <
           TRANSC_PPTOMIB_REDIRMSG_TABLE_SZ)
        {
            pCntx->tRedirMsg.bIsRedir       = TRUE;
            /* Domain name */
            ccur_strlcpy(pCntx->tRedirMsg.tHttpMsgInfo[pCntx->tRedirMsg.nHttpMsgInfo].strDomain,
                    pHttpMsg->tInjMsg.strHostName,
                    sizeof(pCntx->tRedirMsg.tHttpMsgInfo[pCntx->tRedirMsg.nHttpMsgInfo].strDomain));
            /* Client Name */
            memcpy(&(pCntx->tRedirMsg.tHttpMsgInfo[pCntx->tRedirMsg.nHttpMsgInfo].tIpAaddr),
                   &(pPktDesc->ipHdr.tSrcIP),sizeof(pPktDesc->ipHdr.tSrcIP));
            /* Service name */
            ccur_strlcpy(_tmpBuff,
                    pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].strPktPrcArgType,
                    sizeof(_tmpBuff));
            ccur_strlcat(_tmpBuff,"/",sizeof(_tmpBuff));
            ccur_strlcat(_tmpBuff,
                    pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].strPktPrcArgSites,
                    sizeof(_tmpBuff));
            ccur_strlcpy(pCntx->tRedirMsg.tHttpMsgInfo[pCntx->tRedirMsg.nHttpMsgInfo].strSvcName,
                    _tmpBuff,
                    sizeof(pCntx->tRedirMsg.tHttpMsgInfo[pCntx->tRedirMsg.nHttpMsgInfo].strSvcName));
            pCntx->tRedirMsg.nHttpMsgInfo++;
        }
        if(pCntx->tRedirMsg.nHttpMsgInfo ==
           TRANSC_PPTOMIB_REDIRMSG_TABLE_SZ)
        {
            tcHttpProcFlushMibTable(pCntx);
        }
    }
    else
        pCntx->nRedirMsgOflw--;

    return (_result);
}

#if TRANSC_TCSIM
/***************************************************************************
 * function: _tcHttpProcWriteSimQHttpReq
 *
 * description: Write msg to Queue for Sim thread
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpProcWriteSimQHttpReq(
        tc_httpprc_thread_ctxt_t*       pCntx,
        tc_httpparse_calldef_t*         pHttpMsg,
        tc_pktdesc_t*                   pPktDesc)
{
    tresult_t                               _result;
    tc_qmsgtbl_pptosim_t*                   _pQMsg;

    do
    {
        _result = EFAILURE;
        if(pCntx->nSimMsgOflw <= 0)
        {
            /* Make sure the size fit into the msg buffer.
             * No point of processing broken data.
             * The ideal case is to copy the data into
             * sized memory buffer instead of wasteful operation
             * below. */
            if('\0' == pHttpMsg->tInjMsg.strCId[0])
                break;
            /* Must have range value */
            if('\0' == pHttpMsg->tInjMsg.strCRange[0])
                break;
            if(pHttpMsg->tInjMsg.nUrlLen > TRANSC_SIM_URLBUFF)
                break;
            if(strlen(pHttpMsg->tInjMsg.strCId) > TRANSC_SIM_CIDBUFF)
                break;
            if('\0' != pHttpMsg->tInjMsg.strCSimRange[0])
            {
                if(strlen(pHttpMsg->tInjMsg.strCSimRange) > TRANSC_SIM_RGBUFF)
                    break;
            }
            else
            {
                if(strlen(pHttpMsg->tInjMsg.strCRange) > TRANSC_SIM_RGBUFF)
                    break;
            }
            if(strlen(pHttpMsg->tInjMsg.strCMisc) > TRANSC_SIM_CMISCBUFF)
                break;
            _pQMsg = (tc_qmsgtbl_pptosim_t*)
                    lkfqMalloc(pCntx->pQHttpProcToSim);
            if(_pQMsg)
            {
                /* Perform other copy operations here ... */
                /* Write message */
                /* Whole lot of copying...*/
                ccur_strlcpy(_pQMsg->strCId,
                        pHttpMsg->tInjMsg.strCId,
                        sizeof(_pQMsg->strCId));
                if(pHttpMsg->tInjMsg.nUrlLen <
                        sizeof(_pQMsg->strUrl)-1)
                {
                    strncpy(_pQMsg->strUrl,
                            pHttpMsg->tInjMsg.pUrl,
                            pHttpMsg->tInjMsg.nUrlLen);
                    _pQMsg->strUrl[pHttpMsg->tInjMsg.nUrlLen] = '\0';
                }
                if('\0' != pHttpMsg->tInjMsg.strCSimRange[0])
                    _pQMsg->bIsHttpRgReq = TRUE;
                else
                    _pQMsg->bIsHttpRgReq = FALSE;
                ccur_strlcpy(_pQMsg->strCRange,
                        pHttpMsg->tInjMsg.strCRange,
                        sizeof(_pQMsg->strCRange));
                ccur_strlcpy(_pQMsg->strCMisc,
                        pHttpMsg->tInjMsg.strCMisc,
                        sizeof(_pQMsg->strCMisc));
                _pQMsg->tSrcIP = pPktDesc->ipHdr.tSrcIP;
                _pQMsg->tDstIP = pPktDesc->ipHdr.tDstIP;
                _pQMsg->nSrcPort = pPktDesc->tcpHdr.nSrcPort;
                _pQMsg->nDstPort = pPktDesc->tcpHdr.nDstPort;
                ccur_strlcpy(_pQMsg->strUAgent,
                        pHttpMsg->tInjMsg.strUAgent,
                        sizeof(_pQMsg->strUAgent));
                ccur_strlcpy(_pQMsg->strHostName,
                        pHttpMsg->tInjMsg.strHostName,
                        sizeof(_pQMsg->strHostName));
                ccur_strlcpy(_pQMsg->strRedirTarget,
                        pCntx->strHttpPrcRedirTarget,
                        sizeof(_pQMsg->strRedirTarget));
                /* Service type */
                ccur_strlcpy(_pQMsg->strSvcType,
                        pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].strPktPrcArgType,
                        sizeof(_pQMsg->strSvcType));
                /* Service name */
                ccur_strlcpy(_pQMsg->strSvcName,
                        pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].strPktPrcArgSites,
                        sizeof(_pQMsg->strSvcName));
                ccur_strlcpy(_pQMsg->strOptions,
                        pHttpMsg->tInjMsg.strOptions,
                        sizeof(_pQMsg->strOptions));
                lkfqWrite(pCntx->pQHttpProcToSim,
                        (lkfq_data_p)_pQMsg);
            }
            else
            {
                _result = ENOBUFS;
                evLogTrace(
                        pCntx->pQHttpProcToBkgrnd,
                        evLogLvlWarn,
                        &(pCntx->tLogDescSys),
                        "Sim bpool out of msg buffer, "
                        "stop queing msg for %d iterations",
                        TRANSC_HTTPPRC_RDROVFLW_CNT);
                pCntx->nSimMsgOflw =
                        TRANSC_HTTPPRC_RDROVFLW_CNT;
                pCntx->nErrQNoMemHttpProcToSim++;
                break;
            }
        }
        else
            pCntx->nSimMsgOflw--;
        _result = ESUCCESS;
    }while(FALSE);

    return (_result);
}
#endif /* TRANSC_TCSIM */

/***************************************************************************
 * function: _tcHttpProcProcessGetRequest
 *
 * description: Process HTTP GET request
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpProcProcessGetRequest(
        tc_httpprc_thread_ctxt_t*       pHttpPrcCntx,
        tc_pktgen_thread_ctxt_t*        pPktGenCntx,
        tc_g_svcdesc_t*                 pSvcType,
        tc_pktdesc_t*                   pPktDesc)
{
    tresult_t                   _result;
    tc_httpparse_calldef_t      _tHttpMsg;
    tc_httpprc_stats_t*         _pHttpSiteRes;
    S32                         _nParsed;

    CCURASSERT(pHttpPrcCntx);
    CCURASSERT(pPktDesc);

    do
    {
        _result = ESUCCESS;
        _tHttpMsg.bParseErr         = FALSE;
        _tHttpMsg.pCntx             = pHttpPrcCntx;
        _tHttpMsg.eHttpHdrMatchType = tcHttpParseHdrMatchCheck;
        _tHttpMsg.nHttpHdrMatchIdx  = 0;
        _tHttpMsg.eSiteHost         = tcHttpParseSiteTypeUnknown;
        _tHttpMsg.ePossibleSiteHost = pSvcType->ePossibleSvcHostIdx;
        _tHttpMsg.eFieldType        = tcHttpReqFieldypeNone;
        _tHttpMsg.tInjMsg.nUrlLen   = 0;
        _tHttpMsg.tInjMsg.strCId[0]         = '\0';
        _tHttpMsg.tInjMsg.strCSimRange[0]   = '\0';
        _tHttpMsg.tInjMsg.strSvcName[0]     = '\0';
        _tHttpMsg.tInjMsg.strCRange[0]      = '\0';
        _tHttpMsg.tInjMsg.strCOrigin[0]     = '\0';
        _tHttpMsg.tInjMsg.strCMisc[0]       = '\0';
        _tHttpMsg.tInjMsg.strHostName[0]    = '\0';
        _tHttpMsg.tInjMsg.strUAgent[0]      = '\0';
        ccur_strlcpy(_tHttpMsg.tInjMsg.strOutIntfName,
                pHttpPrcCntx->tPktDescMsg.strOutIntfName,
                sizeof(_tHttpMsg.tInjMsg.strOutIntfName));
        pHttpPrcCntx->tParser.data = (void*)&(_tHttpMsg);
        /* Parse GET message */
        /* why nginx parser need to be reset all the time?
         * if this is too slow, I will write my own parser.
         * In the meantime, use theirs since I don't know
         * how much I need to parse.
         */
        tcHttpParserInit(
                &(pHttpPrcCntx->tParser),
                HTTP_REQUEST);
        _nParsed = tcHttpParserExecute(
                &(pHttpPrcCntx->tParser),
                &(pHttpPrcCntx->tSettings),
                (CHAR*)pPktDesc->tcpHdr.pPyld,
                (size_t)pPktDesc->tcpHdr.nPyldLen);
        if(tcHttpParseSiteTypeUnknown ==
                _tHttpMsg.eSiteHost)
        {
            pHttpPrcCntx->nIgnGetReqSvcUnknown++;
            _result = EIGNORE;
            break;
        }
        if(tcHttpParseHdrMatchIgnore ==
                _tHttpMsg.eHttpHdrMatchType)
        {
            pHttpPrcCntx->nHdrMatchIgnore++;
            _result = EIGNORE;
            break;
        }
        if (pHttpPrcCntx->tParser.upgrade)
        {
            pHttpPrcCntx->nGetReqErr++;
            _result = EFAILURE;
            break;
        }
        if (_nParsed != pPktDesc->tcpHdr.nPyldLen)
        {
            pHttpPrcCntx->nGetReqErr++;
            _result = EFAILURE;
            break;
        }
        if(_tHttpMsg.bParseErr)
        {
            pHttpPrcCntx->nGetReqErr++;
            _result = EFAILURE;
            break;
        }
        if(tcHttpParseSvcTypeUnknown == pSvcType->svcType)
        {
            pHttpPrcCntx->nGetReqErr++;
            _result = EFAILURE;
            break;
        }
        /* Host name can't be null, if not
         * provided, get it from ip address.
         * does not take into account proxy
         * X-HOST: <>.
         */
        if('\0' == _tHttpMsg.tInjMsg.strHostName[0])
        {
            tcUtilIPAddrtoAscii(
                &(pPktDesc->ipHdr.tDstIP),
                _tHttpMsg.tInjMsg.strHostName,
                sizeof(_tHttpMsg.tInjMsg.strHostName));
        }
        switch(pSvcType->svcType)
        {
            case tcHttpParseSvcTypeUrl:
                if(pSvcType->nSvcTypeIdx >= TRANSC_LDCFG_SITE_MAXURITYPE_LST)
                {
                    pHttpPrcCntx->nGetReqErr++;
                    _result = EFAILURE;
                }
                break;
            case tcHttpParseSvcTypeSess:
                if(pSvcType->nSvcTypeIdx >= TRANSC_LDCFG_SITE_MAXSESSTYPE_LST)
                {
                    pHttpPrcCntx->nGetReqErr++;
                    _result = EFAILURE;
                }
                break;
            default:
                pHttpPrcCntx->nGetReqErr++;
                _result = EFAILURE;
                break;
        }
        if(ESUCCESS != _result)
            break;
        _pHttpSiteRes = tcHttpInitSiteGetRes(
                        pHttpPrcCntx,_tHttpMsg.eSiteHost);
        if(NULL == _pHttpSiteRes)
        {
            _result = EFAILURE;
            pHttpPrcCntx->nGetReqErr++;
            break;
        }
        /* _tHttpMsg.eSiteHost is
         * being checked within tcSiteGetRes() */
        ccur_strlcpy(_tHttpMsg.tInjMsg.strOptions,
                pHttpPrcCntx->tPPSitesTbl[_tHttpMsg.eSiteHost].strPktPrcArgOpt,
                sizeof(_tHttpMsg.tInjMsg.strOptions));
        ccur_strlcpy(_tHttpMsg.tInjMsg.strSvcName,
                pHttpPrcCntx->tPPSitesTbl[_tHttpMsg.eSiteHost].strPktPrcArgType,
                sizeof(_tHttpMsg.tInjMsg.strSvcName));
        ccur_strlcat(_tHttpMsg.tInjMsg.strSvcName,"/",sizeof(_tHttpMsg.tInjMsg.strSvcName));
        ccur_strlcat(_tHttpMsg.tInjMsg.strSvcName,
                pHttpPrcCntx->tPPSitesTbl[_tHttpMsg.eSiteHost].strPktPrcArgSites,
                sizeof(_tHttpMsg.tInjMsg.strSvcName));
        _pHttpSiteRes->nVideoFileReq++;
        _pHttpSiteRes->nPktsToClients++;

        /* It is possible cache key id exists on the http header,
         * then skip trying to pull it from url or building session
         * relationships.
         */
        /* _tHttpMsg.eSiteHost is validated by tcSiteGetRes() */
        /* Check if session and video relationship is required or not
         * for this service */
        if(tcHttpParseSvcTypeSess ==
                        pSvcType->svcType)
        {
            _result =
                    tcTCPStreamSessGetSessCacheKeyRel(
                            pHttpPrcCntx,
                    &_tHttpMsg,
                    pSvcType);
            if(ESUCCESS != _result)
                break;
        }
        else
        {
            if(pHttpPrcCntx->tPPSitesTbl[_tHttpMsg.eSiteHost].
                    tHttpURI[pSvcType->nSvcTypeIdx].bSessRel)
            {
                _result =
                        tcTCPStreamSessGetSessCacheKeyRel(
                        pHttpPrcCntx,
                        &_tHttpMsg,
                        pSvcType);
                if(ESUCCESS != _result)
                    break;
            }
            else
            {
                _result =
                        tcHttpParserGetAllUrlCacheKeyId(
                                pHttpPrcCntx,
                                pSvcType,
                                &_tHttpMsg);
                if(ESUCCESS != _result)
                    break;
            }
        }
        if(ESUCCESS == _result &&
           (tcHttpParseSvcTypeUrl ==
           pSvcType->svcType))
        {
            tcPktGenProcessPktGen(
                    pPktGenCntx,
                    &(_tHttpMsg.tInjMsg),
                    pPktDesc,
                    _tHttpMsg.eSiteHost);
#if TRANSC_STRMINJTIME
            /* First pkt seems to be higher so discard
             * this anomaly and starts from second pkt.
             */
            if(_pHttpSiteRes->nVideoFileReq > 1)
            {
                _tcHttpProcGetTimeDiff(
                        pHttpPrcCntx,&(pPktDesc->tRxTime));
            }
#endif /* TRANSC_STRMINJTIME */
            _tcHttpProcWriteMibQRedir(
                    pHttpPrcCntx,&_tHttpMsg,pPktDesc);
#if TRANSC_TCSIM
            if(pHttpPrcCntx->bBwSimMode &&
               strcasecmp("no-cache",
               _tHttpMsg.tInjMsg.strOptions))
            {
                /* If range exists in http header,
                 * then use it otherwise use URL range */
                if('\0' != _tHttpMsg.tInjMsg.strCSimRange[0])
                {
                    ccur_strlcpy(_tHttpMsg.tInjMsg.strCRange,
                            _tHttpMsg.tInjMsg.strCSimRange,
                            sizeof(_tHttpMsg.tInjMsg.strCRange));
                }
                if('\0' == _tHttpMsg.tInjMsg.strCRange[0])
                {
                    _tHttpMsg.tInjMsg.strCRange[0] = '0';
                    _tHttpMsg.tInjMsg.strCRange[1] = '-';
                    _tHttpMsg.tInjMsg.strCRange[2] = '\0';
                }
                if('\0' == _tHttpMsg.tInjMsg.strCMisc[0])
                {
                    _tHttpMsg.tInjMsg.strCMisc[0] = '0';
                    _tHttpMsg.tInjMsg.strCMisc[1] = '\0';
                }
                /* Write http session info to Sim thread */
                _result = _tcHttpProcWriteSimQHttpReq(
                           pHttpPrcCntx,&_tHttpMsg,pPktDesc);
                if(ESUCCESS != _result)
                    break;
            }
#endif /* TRANSC_TCSIM */
        }
        pHttpPrcCntx->nGetReqProcessed++;
    }while(FALSE);

    if(ESUCCESS == _result)
        pHttpPrcCntx->nHttpPktProcessed++;
    else if(EIGNORE == _result)
    {
        pHttpPrcCntx->nHttpPktIgnored++;
        tcUtilsLogW3cFormat(pHttpPrcCntx->tid,
                evLogLvlNotice,
                &(_tHttpMsg.tInjMsg),pPktDesc,
                pHttpPrcCntx->pQHttpProcToBkgrnd,
                &(pHttpPrcCntx->tLogDescSys));
    }
    else
    {
        pHttpPrcCntx->nHttpPktProcErr++;
        tcUtilsLogW3cFormat(pHttpPrcCntx->tid,
                evLogLvlWarn,
                &(_tHttpMsg.tInjMsg),pPktDesc,
                pHttpPrcCntx->pQHttpProcToBkgrnd,
                &(pHttpPrcCntx->tLogDescSys));
    }

    return ( _result );
}

/***************************************************************************
 * function: _tcSimLogStats
 *
 * description: dump statistics based on time specified.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHttpProcStatsDump(
        tc_httpprc_thread_ctxt_t*  pCntx,
        tc_gd_time_t*              pUpdDiff)
{
    pCntx->tStatsDumpCkTime.nSeconds += pUpdDiff->nSeconds;
    /* get miliseconds value */
    /*pCntx->tStatsDumpCkTime.reserved += (pUpdDiff->nMicroseconds/1000);*/
    if( pCntx->tStatsDumpCkTime.nSeconds >=
            TRANSC_HTTPPRC_DUMPSTATS_TIME_SEC )
    {
        ccur_memclear(&(pCntx->tStatsDumpCkTime),
                sizeof(pCntx->tStatsDumpCkTime));
        _tcHttpProcLogStats(pCntx);
    }
}

/***************************************************************************
 * function: _tcHttpProcReadQueueMsg
 *
 * description: Read message from pkt processing thread through Queue
 * mechanism
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpProcReadQueueMsg(
        tc_httpprc_thread_ctxt_t*       pHttpPrcCntx,
        tc_pktgen_thread_ctxt_t*        pPktGenCntx)
{
    tresult_t                               _result;
    tc_g_qmsgpptohp_t*                      _pPktDescQMsg;
    tc_gd_time_t                            _tOldTime;
    tc_gd_time_t                            _tNowTime;
    BOOL                                    _bProcPkt;
    tc_gd_time_t                            _tUpdDiff;
    U32                                     _nReqCnt;
    U32                                     _nHunger = 0;

    tUtilUTCTimeGet(&_tOldTime);
    _tNowTime = _tOldTime;
    _nReqCnt  = 0;
    ccur_memclear(&_tUpdDiff,sizeof(_tUpdDiff));
    while(!pHttpPrcCntx->bExit)
    {
        /* Read config to see if there are any changes */
        tcHttpProcReadCfg(pHttpPrcCntx,FALSE);
        tcPktGenReadCfg(pPktGenCntx,FALSE);
        /* Wait until both monitoring and
         * output interfaces are up and running
         * then continue.
         */
        if(tcTrStsDown == pPktGenCntx->bTrSts)
            tcPktGenLogDownStatusAndRetry(pPktGenCntx);
        else if(tcTrStsDown == pHttpPrcCntx->bTrSts)
        {
            evLogTrace(
                  pHttpPrcCntx->pQHttpProcToBkgrnd,
                  evLogLvlError,
                  &(pHttpPrcCntx->tLogDescSys),
                  "Error, http proc configuration loading failure!");
            sleep(1);
        }
        else
        {
            _pPktDescQMsg   = NULL;
            _result         = ENODATA;
            _pPktDescQMsg = (tc_g_qmsgpptohp_t*)lkfqRead(
                    pHttpPrcCntx->pQPPktProcToHttpProc);
            if(_pPktDescQMsg)
            {
                _nHunger = 0;
                _bProcPkt = FALSE;
                if(_pPktDescQMsg->pktDesc.tcpHdr.nPyldLen < (S32)
                   sizeof(pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.pPyldBuf)-1)
                {
                    pHttpPrcCntx->tPktDescMsg.nHttpPktTotal =
                            _pPktDescQMsg->nHttpPktTotal;
                    pHttpPrcCntx->tPktDescMsg.svcType =
                            _pPktDescQMsg->svcType;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.nCaplen   =
                            _pPktDescQMsg->pktDesc.nCaplen;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.nWireLen  =
                            _pPktDescQMsg->pktDesc.nWireLen;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.pktHash   =
                            _pPktDescQMsg->pktDesc.pktHash;
                    /* Copy L2 */
                    pHttpPrcCntx->tPktDescMsg.pktDesc.l2Hdr     =
                            _pPktDescQMsg->pktDesc.l2Hdr;
                    /* Copy L3 */
                    pHttpPrcCntx->tPktDescMsg.pktDesc.ipHdr     =
                            _pPktDescQMsg->pktDesc.ipHdr;
                    /* Copy L4 */
                    pHttpPrcCntx->tPktDescMsg.pktDesc.pMsgStrt = NULL;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.nDstPort   =
                            _pPktDescQMsg->pktDesc.tcpHdr.nDstPort;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.nPyldLen   =
                            _pPktDescQMsg->pktDesc.tcpHdr.nPyldLen;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.nSrcPort   =
                            _pPktDescQMsg->pktDesc.tcpHdr.nSrcPort;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.nTcpAck    =
                            _pPktDescQMsg->pktDesc.tcpHdr.nTcpAck;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.nTcpFlags  =
                            _pPktDescQMsg->pktDesc.tcpHdr.nTcpFlags;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.nTcpSeq    =
                            _pPktDescQMsg->pktDesc.tcpHdr.nTcpSeq;
                    /* Copy Eth type and offsets */
                    pHttpPrcCntx->tPktDescMsg.pktDesc.ethType =
                            _pPktDescQMsg->pktDesc.ethType;
                    pHttpPrcCntx->tPktDescMsg.pktDesc.tOffsets =
                            _pPktDescQMsg->pktDesc.tOffsets;
                    memcpy(pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.pPyldBuf,
                           _pPktDescQMsg->pktDesc.tcpHdr.pPyldBuf,
                           _pPktDescQMsg->pktDesc.tcpHdr.nPyldLen);
                    pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.pPyld =
                            pHttpPrcCntx->tPktDescMsg.pktDesc.tcpHdr.pPyldBuf;
                    ccur_strlcpy(pHttpPrcCntx->tPktDescMsg.strOutIntfName,
                                _pPktDescQMsg->strOutIntfName,
                                sizeof(pHttpPrcCntx->tPktDescMsg.strOutIntfName));
                    _bProcPkt = TRUE;
                }
                lkfqReadRelease(
                        pHttpPrcCntx->pQPPktProcToHttpProc,
                        (lkfq_data_p)_pPktDescQMsg);
                if(_bProcPkt)
                {
                    _tcHttpProcProcessGetRequest(
                                pHttpPrcCntx,
                                pPktGenCntx,
                                &(pHttpPrcCntx->tPktDescMsg.svcType),
                                &(pHttpPrcCntx->tPktDescMsg.pktDesc));
                }
            }
            else
            {
                _nHunger++;
                if(_nHunger > TRANSC_HTTPPRC_HUNGER_CNT)
                {
                    /* Flush all free list */
                    lkfqFlushFreeList(&(pHttpPrcCntx->pQHttpProcToBkgrnd->tLkfq));
                    lkfqFlushFreeList(pHttpPrcCntx->pQHttpProcToMib);
                    lkfqFlushFreeList(pHttpPrcCntx->pQHttpProcToSim);
                    usleep(1000);
                    tcHttpProcFlushMibTable(pHttpPrcCntx);
                    _nHunger = 0;
                }
            }
        }
        tcPktGenCheckRedirPerSecMax(
                pPktGenCntx,&_tUpdDiff,&_tNowTime,&_tOldTime,&_nReqCnt);
        tcPktGenStatsDump(pPktGenCntx,&_tUpdDiff);
        tcPktGenTimedUpdateSharedValues(pPktGenCntx,&_tUpdDiff);
        tcTCPStreamSessHtableTimeoutCheck(pHttpPrcCntx,&_tUpdDiff);
        _tcHttpProcStatsDump(pHttpPrcCntx,&_tUpdDiff);
    }

    return _result;
}

/**************** PROTECTED Functions **********************/

/***************************************************************************
 * function: tcHttpProcLogFlushMibTable
 *
 * description: Flush MIB table.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcHttpProcFlushMibTable(
        tc_httpprc_thread_ctxt_t*        pCntx)
{
    tresult_t                               _result;
    tc_qmsgtbl_comptomib_t*                 _pQMsg;

    _result = ESUCCESS;
    if(pCntx->tRedirMsg.nHttpMsgInfo)
    {
        _pQMsg = (tc_qmsgtbl_comptomib_t*)
                lkfqMalloc(pCntx->pQHttpProcToMib);
        if(_pQMsg)
        {
            memcpy(_pQMsg,&(pCntx->tRedirMsg),
                   sizeof(tc_qmsgtbl_comptomib_t));
            /* Write message */
            lkfqWrite(pCntx->pQHttpProcToMib,
                    (lkfq_data_p)_pQMsg);
        }
        else
        {
            _result = ENOBUFS;
            evLogTrace(
                    pCntx->pQHttpProcToBkgrnd,
                    evLogLvlWarn,
                    &(pCntx->tLogDescSys),
                    "http proc Mib redir bpool out of msg buffer, "
                    "stop queing msg for %d iterations",
                    TRANSC_HTTPPRC_RDROVFLW_CNT);
            pCntx->nRedirMsgOflw =
                    TRANSC_HTTPPRC_RDROVFLW_CNT;
        }
        /* just reset if failure to send, the table entry
         * will be stale anyways.
         */
        pCntx->tRedirMsg.nHttpMsgInfo = 0;
    }

    return _result;
}

/***************************************************************************
 * function: tcHttpProcReadCfg
 *
 * description: Check to see if flag is set and reload the config file.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcHttpProcReadCfg(tc_httpprc_thread_ctxt_t* pCntx,BOOL bForcedLdShVal)
{
    tresult_t                 _result;
    tc_ldcfg_conf_t*          _pNewConfigYamlCfg;
    tc_shared_cfgmsg_t*      _pConfigYamlCfgDesc;

    _pConfigYamlCfgDesc = tcShDGetCfgYamlDesc();
    if(tcShProtectedGetSigCfgYamlLoadSts(tcTRCompTypeHttpPrc))
    {
        pthread_mutex_lock(&(_pConfigYamlCfgDesc->tCfgMutex));
        if(_pConfigYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgConfigYamlHttpPrc])
        {
            _pNewConfigYamlCfg    = &(_pConfigYamlCfgDesc->tNewConfig);
            _result = tcHttpInitLoadableRes(pCntx,_pNewConfigYamlCfg);
            if(ESUCCESS == _result)
            {
                evLogTrace(
                        pCntx->pQHttpProcToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "New config.yaml config loaded successfully by Http Prc thread TID#%x",pCntx->tid);
            }
            else
            {
                evLogTrace(pCntx->pQHttpProcToBkgrnd,
                        evLogLvlError,
                        &(pCntx->tLogDescSys),
                        "New config.yaml config loading failure by Http Prc thread TID#%x",pCntx->tid);
            }
            _pConfigYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgConfigYamlHttpPrc] = FALSE;
        }
        pthread_mutex_unlock(&(_pConfigYamlCfgDesc->tCfgMutex));
        tcShProtectedSetSigCfgYamlLoadSts(tcTRCompTypeHttpPrc,FALSE);
    }
}

/***************************************************************************
 * function: tcHttpProcThreadEntry
 *
 * description: HTTP processing thread entry point.
 * Note: currently now Http Proc and Pkt gen are combined
 * together as one thread.
 ***************************************************************************/
CCUR_PROTECTED(mthread_result_t)
tcHttpProcThreadEntry(void* pthdArg)
{
    U32                                 _retry;
    tresult_t                           _result;
    time_t                              _tnow;
    tc_temp_thread_ctxt_t*            _pHttpProcPktGen = NULL;
    tc_httpprc_thread_ctxt_t*           _pHttpPrcCntx = NULL;
    tc_pktgen_thread_ctxt_t*            _pPktGenCntx = NULL;
    _pHttpProcPktGen = (tc_temp_thread_ctxt_t*)pthdArg;

    CCURASSERT(_pHttpProcPktGen);

    _pHttpPrcCntx   = _pHttpProcPktGen->pHttpPrcThd;
    _pPktGenCntx    = _pHttpProcPktGen->pPktGenThd;
    /****** 1. Thread Common Init ********/
    /* Http proc */
    _tnow = time(0);
    _pHttpPrcCntx->tGMUptime =
            gmtime(&_tnow);
    tUtilUTCTimeGet(&(_pHttpPrcCntx->tUptime));
    _pHttpPrcCntx->tid = (U32)pthread_self();
    _tnow = time(0);
    _pPktGenCntx->tGMUptime =
            gmtime(&_tnow);
    tUtilUTCTimeGet(&(_pPktGenCntx->tUptime));
    _pPktGenCntx->tid  = pthread_self();
    /* Sync Queues */
    lkfqSyncQ(&(_pHttpPrcCntx->pQHttpProcToBkgrnd->tLkfq));
    lkfqSyncQ(_pHttpPrcCntx->pQPPktProcToHttpProc);
    lkfqSyncQ(_pHttpPrcCntx->pQHttpProcToMib);
    lkfqSyncQ(_pHttpPrcCntx->pQHttpProcToSim);
    /****** 2. Thread Resource Init  ********/
    /* Pkt Gen */
    _pPktGenCntx->tIntfCfg.pEvLog =
            _pPktGenCntx->pQPktGenToBkgrnd;
    _pPktGenCntx->tIntfCfg.pLogDescSysX =
            &(_pPktGenCntx->tLogDescSys);
    /* Http proc */
    /* init nginx parser */
    _pHttpPrcCntx->tSettings.on_header_field =
            tcHttpParseReqHdrFieldCb;
    _pHttpPrcCntx->tSettings.on_header_value =
            tcHttpParseReqHdrValueCb;
    _pHttpPrcCntx->tSettings.on_url =
            tcHttpParseReqUrlCb;
    /* TCP Stream config */
    _pHttpPrcCntx->nResTcpStrmSessBinMax =
            TRANSC_HTTPPRC_TCPSTRM_DFLTBIN_CNT;
    _pHttpPrcCntx->nResTcpStrmSessMax =
            TRANSC_HTTPPRC_TCPSTRM_DLFT_CNT;
    _pHttpPrcCntx->nResTcpStrmSessGrowSz =
            TRANSC_HTTPPRC_TCPSTRM_GROW_CNT;
    _result = tcTCPStreamSessEntryMpoolCreate(_pHttpPrcCntx);
    if(ESUCCESS != _result)
    {
        /* Set this thread to exit,which will cause
         * the entire application to exit. */
        _pHttpPrcCntx->bExit = TRUE;
        evLogTrace(
                _pHttpPrcCntx->pQHttpProcToBkgrnd,
                evLogLvlError,
                &(_pHttpPrcCntx->tLogDescSys),
                "Http Proc TID#%x Initialization Error, unable to allocate memory!",
                _pHttpPrcCntx->tid);
    }
    else
    {
        /* Http Proc */
        tcHttpProcReadCfg(_pHttpPrcCntx,TRUE);
        _tcHttpProcLogStats(_pHttpPrcCntx);
        tcShProtectedDMsgSetCompInitRdy(
                tcTRCompTypeHttpPrc,TRUE);
        /****** 3. Thread Synchronization  ********/
        _retry = 0;
        while (!(_pPktGenCntx->bExit) &&
               !(_pHttpPrcCntx->bExit))
        {
            if(tcShProtectedDMsgIsCompInitRdy(
                    tcTRCompTypeHealth))
                break;
            else
            {
                if(_retry >= 5)
                    break;
                tcPktGenReadCfg(_pPktGenCntx,TRUE);
                tcShProtectedDMsgSetCompSyncRdy(
                        tcTRCompTypePktGen,TRUE);
                sleep(1);
                _retry++;
            }
        }
        tcPktGenUpdateSharedValues(
                _pPktGenCntx,TRUE);
        tcPktGenLogStats(_pPktGenCntx);
        tcPktGenInitLogOutCfgLoadStatus(_pPktGenCntx);
        tcShProtectedDMsgSetCompInitRdy(
                tcTRCompTypePktGen,TRUE);
    }
    /****** 4. Thread Process ********/
    /* Block here until signaled by main thread,
     * This will allow other thread, Queue, Resource
     * to be initialized. */
    while (!(_pHttpPrcCntx->bExit))
    {
        if (ESUCCESS == mSemCondVarSemWaitTimed(
                      &(_pHttpPrcCntx->tCondVarSem),
                      TRANSC_HTTPPRC_WAITTIMEOUT_MS))
            break;
    }
    if(FALSE == _pHttpPrcCntx->bExit &&
       FALSE == _pPktGenCntx->bExit)
    {
        evLogTrace(
                _pHttpPrcCntx->pQHttpProcToBkgrnd,
                evLogLvlInfo,
                &(_pHttpPrcCntx->tLogDescSys),
                "Http Proc Thd is running with TID#%x",_pHttpPrcCntx->tid);
        evLogTrace(
                _pPktGenCntx->pQPktGenToBkgrnd,
                evLogLvlInfo,
                &(_pHttpPrcCntx->tLogDescSys),
                "Pkt Gen Thd is running with TID#%x",_pPktGenCntx->tid);
        _result =
                _tcHttpProcReadQueueMsg(_pHttpPrcCntx,
                                        _pPktGenCntx);
        /* Http proc */
        tUtilUTCTimeGet(&(_pHttpPrcCntx->tDowntime));
        /* Dump temporary stats */
        _tcHttpProcLogStats(_pHttpPrcCntx);
        /* Dump summary stats */
        _tcHttpProcLogStatsSummary(_pHttpPrcCntx);
        /* Pkt Gen */
        tUtilUTCTimeGet(&(_pPktGenCntx->tDowntime));
        /* Dump temporary stats */
        tcPktGenLogStats(_pPktGenCntx);
        /* Dump summary stats */
        tcPktGenLogStatsSummary(_pPktGenCntx);
    }
    tcShProtectedDSetCompSts(tcTRCompTypeHttpPrc,tcTrStsDown);
    tcShProtectedDSetCompSts(tcTRCompTypePktGen,tcTrStsDown);
    /****** 5. Thread Resource Destroy ********/
    /* Http proc */
    tcHttpInitSiteCleanupRes(_pHttpPrcCntx);
    tcTCPStreamSessEntryMpoolDestroy(_pHttpPrcCntx);
    /* Pkt Gen */
    if(_pPktGenCntx->tIntfX.bOutIntfOpen)
        tcPktIOTxOutIntfClose(_pPktGenCntx);
    /****** 6. Exit application ********/
    /* Http proc */
    if(_pHttpPrcCntx->bExit)
        tcShProtectedDSetAppExitSts(_pHttpPrcCntx->bExit);
    /* Pkt Gen */
    if(_pPktGenCntx->bExit)
        tcShProtectedDSetAppExitSts(_pPktGenCntx->bExit);

    return _result;
}
