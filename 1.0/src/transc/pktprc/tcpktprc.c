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

#include "pktprc.h"

//*************** external functions **********************
extern void print_blacklist(void);

extern int blacklisted(uint32_t ip);

extern const char * str_ip(const uint32_t ip);

/**************** PRIVATE Functions **********************/
/***************************************************************************
 * function: _tcPktProcLogOutCfgLoadStatus
 *
 * description: dumps subset of pkt proc thread status.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcPktProcLogOutCfgLoadStatus(
        tc_pktprc_thread_ctxt_t*      pCntx)
{
    U32                     _i;
    tc_monintf_intfd_t*     _pIntfd;
    tc_intf_config_t*       _pIntfdCfg;
    tc_monintf_mon_t*       _pMonIntf;

    _pIntfd       = &(pCntx->tIntfX);
    _pIntfdCfg    = &(pCntx->tIntfCfg);
    for(_i=0;_i<_pIntfd->nMonIntfTotal;_i++)
    {
        _pMonIntf = &(_pIntfd->tMonIntfTbl[_i]);
        if(_pMonIntf)
        {
            evLogTrace(pCntx->pQPktProcToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Ingress stats:\n"
                    "intfName/RedirAddr/TotalNum/OpenNum/Sts/IsUp\n"
                    "%s/%s/%s/%d/%d/%d/%d",
                    _pMonIntf->tIntf.strIntfName,
                    _pMonIntf->strRedirAddr,
                    _pMonIntf->tIntf.strIntfVal,
                    _pIntfd->nMonIntfTotal-0,
                    _pIntfd->nMonIntfOpen-0,
                    _pIntfd->bMonIntfOpen,
                    _pIntfd->bMonIntfUp);
        }
    }
    if(_pIntfdCfg->bActiveMode)
    {
        evLogTrace(pCntx->pQPktProcToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
               "Mode Of Operation Mapped Entries:%lu/%lu",
               _pIntfd->nIntfMapTblTotal,TRANSC_INTERFACE_MAX);
        for(_i=0;_i<_pIntfd->nIntfMapTblTotal;_i++)
        {
            if(_pIntfd->tIntfMapTbl[_i].pOutIntfH)
            {
                evLogTrace(pCntx->pQPktProcToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                       "pkt proc Mode Of Operation: %s/%s->%s",
                       _pIntfd->tMonIntfTbl[_i].strRedirAddr,
                       _pIntfd->tMonIntfTbl[_i].tIntf.strIntfName,
                       _pIntfd->tIntfMapTbl[_i].pOutIntfH->tLinkIntf.strIntfName);
            }
        }
    }
}

/***************************************************************************
 * function: _tcPktPrcIsValidsite
 *
 * description: Check if GET request match with the list of provided valid
 * sites from config file.
 ***************************************************************************/
CCUR_PRIVATE(BOOL)
_tcPktPrcIsValidsite(
        tc_g_svcdesc_t*                   pSvcType,
        tc_pktprc_thread_ctxt_t*          pCntx,
        CHAR*                             pPyld,
        U32                               nPyldLen)
{
    S32                         _i;
    U32                         _j;
    U32                         _nMatchSz;
    BOOL                        _bValid;
    I32                         _rc;
    tc_regex_t*                 _pRe;
    I32                         _subStrVec[TRANSC_PKTPRC_LOAD_REGEXSUBEX_NUM];

    CCURASSERT(pSvcType);
    CCURASSERT(pPyld);
    CCURASSERT(pCntx);

    _bValid           = FALSE;
    pSvcType->svcType = tcHttpParseSvcTypeUnknown;
    for(_i=1;_i<pCntx->nPPSitesTbl;_i++)
    {
        for(_j=0;_j<pCntx->tPPSitesTbl[_i].nHttpURITbl;_j++)
        {
            _nMatchSz =
                    pCntx->tPPSitesTbl[_i].tHttpURITbl[_j].nMatchSz;
            if(_nMatchSz > nPyldLen)
                _nMatchSz = nPyldLen;
            _pRe = &(pCntx->tPPSitesTbl[_i].tHttpURITbl[_j].tSig.tReCkey);
            if(NULL == _pRe->code)
                break;
            _rc = tcRegexExec(
                    _pRe,
                    pPyld,_nMatchSz,
                    _subStrVec,TRANSC_PKTPRC_LOAD_REGEXSUBEX_NUM);
            if(_rc >= 0)
            {
                _bValid                         = TRUE;
                pSvcType->svcType               = tcHttpParseSvcTypeUrl;
                pSvcType->nSvcTypeIdx           = _j;
                pSvcType->ePossibleSvcHostIdx   = _i;
                break;
            }
        }
        if(_bValid)
            break;
        for(_j=0;_j<pCntx->tPPSitesTbl[_i].nHttpSessTbl;_j++)
        {
            _nMatchSz =
                    pCntx->tPPSitesTbl[_i].tHttpSessTbl[_j].nMatchSz;
            if(_nMatchSz > nPyldLen)
                _nMatchSz = nPyldLen;
            _pRe =
                    &(pCntx->tPPSitesTbl[_i].tHttpSessTbl[_j].tSig.tReCkey);
            if(NULL == _pRe->code)
                break;
            _rc = tcRegexExec(
                    _pRe,
                    pPyld,_nMatchSz,
                    _subStrVec,TRANSC_PKTPRC_LOAD_REGEXSUBEX_NUM);
            if(_rc >= 0)
            {
                _bValid                         = TRUE;
                pSvcType->svcType               = tcHttpParseSvcTypeSess;
                pSvcType->nSvcTypeIdx           = _j;
                pSvcType->ePossibleSvcHostIdx   = _i;
                break;
            }
        }
        if(_bValid)
            break;
    }

    return _bValid;
}

/***************************************************************************
 * function: tcHttpParseGetRequestType
 *
 * description: Check if HTTP payload contains GET request.
 ***************************************************************************/
CCUR_PRIVATE(tc_pktprc_msgpyldtype_e)
_tcPktPrcGetRequestType(
        tc_pktprc_thread_ctxt_t*    pCntx,
        CHAR*                       pPyld)
{
    tc_pktprc_msgpyldtype_e  _reqType;

    CCURASSERT(pCntx);

    _reqType =
            tcHttpMsgPyldTypeUNKNOWN;
    if(!strncmp(
            pPyld,
            TRANSC_PKTPRC_REQTYPE_GET,
            sizeof(TRANSC_PKTPRC_REQTYPE_GET)-1))
        _reqType    = tcHttpMsgReqPyldTypeGET;

    return (_reqType);
}

/***************************************************************************
 * function: _tcPktProcWriteHealthReport
 *
 * description: Write health report message for health thread.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcPktProcWriteHealthReport(tc_pktprc_thread_ctxt_t* pCntx)
{
    U32 _i;

    if(pCntx->tIntfX.bIntfCfgChanged)
    {
        /* Copy all the values here */
        pCntx->tTmpPPtoHlthMonIntf.nOpen =
                pCntx->tIntfX.nMonIntfOpen;
        pCntx->tTmpPPtoHlthMonIntf.nTotal =
                pCntx->tIntfX.nMonIntfTotal;
        for(_i=0;_i<pCntx->tIntfX.nMonIntfTotal;_i++)
        {
            ccur_strlcpy(pCntx->tTmpPPtoHlthMonIntf.tIntfITbl[_i].strIntfName,
                    pCntx->tIntfX.tMonIntfTbl[_i].tIntf.strIntfName,
                    sizeof(pCntx->tTmpPPtoHlthMonIntf.tIntfITbl[_i].strIntfName));
            ccur_strlcpy(pCntx->tTmpPPtoHlthMonIntf.tIntfITbl[_i].strRedirAddr,
                    pCntx->tIntfX.tMonIntfTbl[_i].strRedirAddr,
                    sizeof(pCntx->tTmpPPtoHlthMonIntf.tIntfITbl[_i].strRedirAddr));
            pCntx->tTmpPPtoHlthMonIntf.tIntfITbl[_i].bIntfRdy =
                    pCntx->tIntfX.tMonIntfTbl[_i].tIntf.bIntfRdy;
        }
        pCntx->bSendHealthReport = TRUE;
    }
}

/**************** PROTECTED Functions **********************/

/***************************************************************************
 * function: _tcPktPrcSiteLogStatsSummary
 *
 * description: dump summary statistics
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
_tcPktPrcSiteLogStatsSummary(
        tc_pktprc_thread_ctxt_t* pCntx)
{
    U32                     _nTotalHttpPkts;
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
    _nTotalHttpPkts = pCntx->nPcapPktTotal;
    if(!_nTotalHttpPkts)
        _nTotalHttpPkts = 1;
    evLogTrace(
            pCntx->pQPktProcToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "****Pkt Proc Summary TID#%x****\n"
            "Uptime:%s\n"
            "Total Running time:%ld mins\n",
            pCntx->tid,
            _pTNowbuf,
            _tDiffTime.nSeconds/60
            );
    return(ESUCCESS);
}

/***************************************************************************
 * function: _tcPktProcUpdateSharedValues
 *
 * description: Update shared values.
 ***************************************************************************/
CCUR_PROTECTED(void)
_tcPktProcUpdateSharedValues(tc_pktprc_thread_ctxt_t* pCntx, BOOL bRead)
{
    tc_shared_pktprcmsg_t*    _pPktPrcToHlthMsg;

    if(!bRead)
    {
        if(pCntx->bSendHealthReport)
        {
            if(pCntx->tIntfX.bIntfCfgChanged)
            {
                pthread_mutex_lock(tcShDGetCompMutex(tcTRCompTypePktPrc));
                _pPktPrcToHlthMsg = tcShDGetPktPrcMsg();
                _pPktPrcToHlthMsg->bMonIntfUpdate = TRUE;
                memcpy(&(_pPktPrcToHlthMsg->tMonIntf),
                       &(pCntx->tTmpPPtoHlthMonIntf),
                       sizeof(_pPktPrcToHlthMsg->tMonIntf));
                pthread_mutex_unlock(tcShDGetCompMutex(tcTRCompTypePktPrc));
                pCntx->tIntfX.bIntfCfgChanged = FALSE;
            }
            pCntx->bSendHealthReport = FALSE;
        }
    }
}

/***************************************************************************
 * function: tcPktPrcFlushMibTable
 *
 * description: Flush MIB table.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktPrcFlushMibTable(tc_pktprc_thread_ctxt_t * pCntx)
{
    tresult_t                               _result;
    tc_qmsgtbl_comptomib_t*                 _pQMsg;

    _result = ESUCCESS;
    if(pCntx->tGenMsg.nHttpMsgInfo)
    {
        _pQMsg = (tc_qmsgtbl_comptomib_t*)
                lkfqMalloc(pCntx->pQPktProcToMib);
        if(_pQMsg)
        {
            memcpy(_pQMsg,&(pCntx->tGenMsg),
                   sizeof(tc_qmsgtbl_comptomib_t));
            /* Write message */
            lkfqWrite(pCntx->pQPktProcToMib,
                    (lkfq_data_p)_pQMsg);
        }
        else
        {
            _result = ENOBUFS;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlWarn,
                    &(pCntx->tLogDescSys),
                    "Pkt Prc Mib traffic bpool out of msg buffer, "
                    "stop queing msg for %d iterations",
                    TRANSC_PKTPRC_RDROVFLW_CNT);
            pCntx->nGenMsgBuffOflw =
                    TRANSC_PKTPRC_RDROVFLW_CNT;
        }
        /* just reset if failure to send, the table entry
         * will be stale anyways.
         */
        pCntx->tGenMsg.nHttpMsgInfo = 0;
    }

    return _result;
}

/***************************************************************************
 * function: tcPktPrcWriteMibQTraffic
 *
 * description: Write stats msg to Queue to be consumed by Mib thread
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktPrcWriteMibQTraffic(
        tc_pktprc_thread_ctxt_t*        pCntx,
        CHAR*                           strDomainName,
        tc_iphdr_ipaddr_t*              pClientIpAddr)
{
    tresult_t                               _result;

    _result = ESUCCESS;
    if(pCntx->nGenMsgBuffOflw <= 0)
    {
        if(pCntx->tGenMsg.nHttpMsgInfo <
           TRANSC_PPTOMIB_TRAFMSG_TABLE_SZ)
        {
            pCntx->tGenMsg.bIsRedir       = FALSE;
            /* Domain name */
            ccur_strlcpy(pCntx->tGenMsg.tHttpMsgInfo[pCntx->tGenMsg.nHttpMsgInfo].strDomain,
                    strDomainName,
                    sizeof(pCntx->tGenMsg.tHttpMsgInfo[pCntx->tGenMsg.nHttpMsgInfo].strDomain));
            /* Client Name */
            memcpy(&(pCntx->tGenMsg.tHttpMsgInfo[pCntx->tGenMsg.nHttpMsgInfo].tIpAaddr),
                   pClientIpAddr,sizeof(tc_iphdr_ipaddr_t));
            /* Service name */
            pCntx->tGenMsg.tHttpMsgInfo[pCntx->tGenMsg.nHttpMsgInfo].strSvcName[0] = '\0';
            pCntx->tGenMsg.nHttpMsgInfo++;
        }
        if(pCntx->tGenMsg.nHttpMsgInfo ==
           TRANSC_PPTOMIB_TRAFMSG_TABLE_SZ)
        {
            tcPktPrcFlushMibTable(pCntx);
        }
    }
    else
        pCntx->nGenMsgBuffOflw--;

    return (_result);
}
/***************************************************************************
 * function: tcHttpParseGetDomainName
 *
 * description: Specific function that parse
 * through HTTP header and picks up the domain name.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktPrcParseGetDomainName(
        tc_pktprc_thread_ctxt_t* pCntx,
        CHAR*                    strDomainName,
        U16                      nStrDomainName,
        const CHAR*              data,
        S32                      len)
{
    const CHAR*       _p;
    U32               _i;
    U32               _len;

    strDomainName[0] = '\0';
    if(len > (S32)sizeof("\r\n"))
    {
        _p  = data;
        for(_i=0; _i < len-sizeof("\r\n"); _i++)
        {
            if (_p[_i] == '\r' && _p[_i+1] == '\n')
            {
                _i+=2;
                if(!strncmp(&(_p[_i]),"Host:",sizeof("Host:")-1))
                {
                    /* Copy the domain name all the way till \r or \n. */
                    _len = 0;
                    _i+=sizeof("Host:");
                   while((_p[_i] != '\r' && _p[_i] != '\n') &&
                         (_len < (U32)nStrDomainName-1))
                   {
                       strDomainName[_len] = _p[_i];
                       _i++;
                       _len++;
                   }
                   strDomainName[_len] = '\0';
                   break;
                }
            }
        }
    }
}

/***************************************************************************
 * function: tcPktPrcLogStats
 *
 * description: dump statistics at current point in time
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktPrcLogStats(tc_pktprc_thread_ctxt_t * pCntx)
{
    tresult_t               _result;
    CHAR                    _strBuff1[256];

    CCURASSERT(pCntx);

    _result = ESUCCESS;
    if (TRANSC_ZLOGCATEGORY_BITMAP_CK(pCntx->tLogDescSys,evLogLvlInfo))
        return _result;
    evLogTrace(
            pCntx->pQPktProcToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "*****Pkt Proc TID#%x Info****\n"
            "    HTTP Pkts Total:%lu\n"
            "       HTTP Pkts Ignored:%lu\n"
            "           HTTP GET Req From Cache Server:%lu\n"
            "           HTTP GET process Err:%lu\n"
            "       Pcap Pkts-parse Error:%lu\n"
            "       Pcap Pkts-parse Ignore:%lu\n"/* 4lvl vlan or non-ipv4/ */
            "       TCP 302 Pop:%lu\n"
            "    Pkt Capture stats:\n"
            "%s\n"
            ,
            pCntx->tid,
            pCntx->nPcapPktTotal,
            pCntx->nPcapPktIgnored,
            pCntx->nPcapIgnGetReqFrmCacheSrvr,
            pCntx->nGetReqErr,
            pCntx->nPcapParseErr,
            pCntx->nPcapParseIgnored,
            pCntx->nGetRedirPop,
            tcPktIORxMonIntfGetStat(pCntx,_strBuff1,sizeof(_strBuff1))
            );

    return(_result);
}

/***************************************************************************
 * function: tcPktPrcIsRequestServiceAvail
 *
 * description: Find if url of a service exists
 * within table of available services
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcPktPrcIsRequestServiceAvail(
        tc_pktprc_thread_ctxt_t*        pCntx,
        tc_g_svcdesc_t*                 pSvcType,
        CHAR*                           pPyld,
        S32                             nPyldLen)
{
    tc_pktprc_msgpyldtype_e         _eReqType;
    BOOL                            _bIsSvc;
    U16                             _nSkipLen;
    CHAR*                           _pPyld;
    S32                             _nPyldLen;

    do
    {
        _bIsSvc = FALSE;
        _eReqType =
                _tcPktPrcGetRequestType(
                        pCntx,pPyld);
        /* only care about GET for now */
        if(tcHttpMsgReqPyldTypeGET != _eReqType)
            break;
        _pPyld      = pPyld+sizeof("GET");
        _nPyldLen   = nPyldLen-sizeof("GET");
        if(pCntx->bProcessProxyReq)
        {
            if(!TCUTIL_IS_URLABSPATH(_pPyld))
            {
                /* Skips http://.../ if exists in case of web proxy */
                if(nPyldLen > (S32)sizeof("http://") &&
                   TCUTIL_IS_URLHTTPSTRING(_pPyld))
                {
                    _nSkipLen = tcUtilSkipGetHttpStringLen(
                                pPyld,nPyldLen);
                    if(_nSkipLen)
                    {
                        _pPyld     = pPyld+_nSkipLen;
                        _nPyldLen  = nPyldLen-_nSkipLen;
                    }
                    else
                        break;
                }
                else
                    break;
            }
        }
        else
        {
            if(!TCUTIL_IS_URLABSPATH(_pPyld))
                break;
        }
        if(!strncmp(_pPyld,
           TRANSC_CCUR_REDIR_PREPEND"/",
           sizeof(TRANSC_CCUR_REDIR_PREPEND"/")-1))
        {
            pCntx->nGetRedirPop++;
            break;
        }
        _bIsSvc = _tcPktPrcIsValidsite(
                    pSvcType,
                    pCntx,
                    _pPyld,
                    _nPyldLen);

    }while(FALSE);

    return _bIsSvc;
}

/***************************************************************************
 * function: tcPktPrcCkIpBlacklist
 *
 * description: Hash the ip addr and Find ip address within ip blacklist table
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktPrcCkIpBlacklist(tc_pktprc_thread_ctxt_t * pCntx,
        tc_pktdesc_t * pPktDesc)
{
    uint32_t ip;
    ip  = pPktDesc->ipHdr.tSrcIP.ip.v4.octet[0] << 24;
    ip |= pPktDesc->ipHdr.tSrcIP.ip.v4.octet[1] << 16;
    ip |= pPktDesc->ipHdr.tSrcIP.ip.v4.octet[2] << 8;
    ip |= pPktDesc->ipHdr.tSrcIP.ip.v4.octet[3];
    if(blacklisted(ip))
    {
        evLogTrace(pCntx->pQPktProcToBkgrnd, evLogLvlDebug, &(pCntx->tLogDescSys),
            "blacklisted ip: %s", str_ip(ip));
        return EIGNORE;
    }
    return ESUCCESS;
}

/***************************************************************************
 * function: tcPktPrcReadCfg
 *
 * description: Check to see if flag is set and reload the config file.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktPrcReadCfg(tc_pktprc_thread_ctxt_t* pCntx, BOOL bForceUpdShVal)
{
    tresult_t                _result;
    tc_ldcfg_conf_t*         _pNewConfigYamlCfg;
    tc_shared_cfgmsg_t* 	 _pConfigYamlCfgDesc;
    BOOL                     _bLoad = FALSE;

    _pConfigYamlCfgDesc = tcShDGetCfgYamlDesc();
    if(tcShProtectedGetSigCfgYamlLoadSts(tcTRCompTypePktPrc))
    {
        pthread_mutex_lock(&(_pConfigYamlCfgDesc->tCfgMutex));
        if(_pConfigYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgConfigYamlPktPrc])
        {
            _bLoad = TRUE;
            _pNewConfigYamlCfg    = &(_pConfigYamlCfgDesc->tNewConfig);
            _result = tcPktProcConfigInitLoadableRes(pCntx,_pNewConfigYamlCfg);
            if(ESUCCESS == _result)
            {
                evLogTrace(
                        pCntx->pQPktProcToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "New config.yaml config loaded successfully by Pkt Prc thread TID#%x",pCntx->tid);
            }
            else
            {
                evLogTrace(pCntx->pQPktProcToBkgrnd,
                        evLogLvlError,
                        &(pCntx->tLogDescSys),
                        "New config.yaml config loading failure by Pkt Prc thread TID#%x",pCntx->tid);
            }
            _pConfigYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgConfigYamlPktPrc] = FALSE;
        }
        pthread_mutex_unlock(&(_pConfigYamlCfgDesc->tCfgMutex));
        tcShProtectedSetSigCfgYamlLoadSts(tcTRCompTypePktPrc,FALSE);
    }
    if(_bLoad || bForceUpdShVal)
    {
        _tcPktProcWriteHealthReport(pCntx);
        _tcPktProcUpdateSharedValues(pCntx,FALSE);
        _tcPktProcLogOutCfgLoadStatus(pCntx);
    }
}

/***************************************************************************
 * function: tcPktPrcToHttpProcQueueMsg
 *
 * description: Send message to Http processing thread through Queue
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktPrcToHttpProcQueueMsg(
        tc_pktprc_thread_ctxt_t*         pCntx,
        tc_monintf_map_t*            pMapIntf,
        tc_g_svcdesc_t*                  pSvcType,
        tc_g_qmsgpptohp_t*               pPktDescQMsg)
{

    tresult_t                        _result = EIGNORE;

    if(pPktDescQMsg->pktDesc.tcpHdr.nPyldLen < (S32)
       sizeof(pPktDescQMsg->pktDesc.tcpHdr.pPyldBuf)-1)
    {
        pPktDescQMsg->nHttpPktTotal =
                pCntx->nPcapPktTotal;
        if(pMapIntf->pOutIntfH)
        {
            ccur_strlcpy(pPktDescQMsg->strOutIntfName,
                    pMapIntf->pOutIntfH->tLinkIntf.strIntfName,
                    sizeof(pPktDescQMsg->strOutIntfName));
        }
        else
            pPktDescQMsg->strOutIntfName[0] = '\0';
        memcpy(&(pPktDescQMsg->svcType),
                pSvcType,
                sizeof(tc_g_svcdesc_t));
        memcpy(pPktDescQMsg->pktDesc.tcpHdr.pPyldBuf,
                pPktDescQMsg->pktDesc.tcpHdr.pPyld,
                pPktDescQMsg->pktDesc.nCaplen);
        pPktDescQMsg->pktDesc.pMsgStrt       = NULL;
        pPktDescQMsg->pktDesc.tcpHdr.pPyld   = pPktDescQMsg->pktDesc.tcpHdr.pPyldBuf;
        /* write and commit to mutex free queue */
        lkfqWrite(pCntx->pQPPktProcToHttpProc,
                (lkfq_data_p)pPktDescQMsg);
        _result = ESUCCESS;
    }

    return _result;
}


/***************************************************************************
 * function: tcPktPrcThreadEntry
 *
 * description: Entry point for pkt processing thread
 ***************************************************************************/
CCUR_PROTECTED(mthread_result_t)
tcPktPrcThreadEntry(void* pthdArg)
{
    U32                                 _retry;
    tresult_t                           _result;
    time_t                              _tnow;
    tc_pktprc_thread_ctxt_t*            _pCntx;
    _pCntx = (tc_pktprc_thread_ctxt_t*)pthdArg;

    CCURASSERT(_pCntx);

    /* TODO: Move Queue init here */

    /****** 1. Thread Common Init ********/
    _tnow = time(0);
    _pCntx->tGMUptime =
            gmtime(&_tnow);
    tUtilUTCTimeGet(&(_pCntx->tUptime));
    _pCntx->tid = (U32)pthread_self();
    /* Sync Queues */
    lkfqSyncQ(&(_pCntx->pQPktProcToBkgrnd->tLkfq));
    lkfqSyncQ(_pCntx->pQPPktProcToHttpProc);
    lkfqSyncQ(_pCntx->pQPktProcToMib);
    /****** 2. Thread Resource Init  ********/
    _pCntx->tIntfCfg.pEvLog =
            _pCntx->pQPktProcToBkgrnd;
    _pCntx->tIntfCfg.pLogDescSysX =
            &(_pCntx->tLogDescSys);
    /****** 3. Thread Synchronization  ********/
    _retry = 0;
    while (!(_pCntx->bExit))
    {
        if(tcShProtectedDMsgIsCompInitRdy(
                tcTRCompTypeHealth))
            break;
        else
        {
            if(_retry >= 5)
                break;
            tcPktPrcReadCfg(_pCntx,TRUE);
            tcShProtectedDMsgSetCompSyncRdy(
                    tcTRCompTypePktPrc,TRUE);
            sleep(1);
            _retry++;
        }
    }
    _tcPktProcUpdateSharedValues(
            _pCntx,TRUE);
    tcPktPrcLogStats(_pCntx);
    _tcPktProcLogOutCfgLoadStatus(_pCntx);
    tcShProtectedDMsgSetCompInitRdy(
            tcTRCompTypePktPrc,TRUE);
    /****** 4. Thread Process ********/
    /* Block here until signaled by main thread,
     * This will allow other thread, Queue
     * to be initialized. */
    while (!(_pCntx->bExit))
    {
        if (ESUCCESS == mSemCondVarSemWaitTimed(
                      &(_pCntx->tCondVarSem),
                      TRANSC_PKTPRC_WAITTIMEOUT_MS
                      ))
            break;
    }
    _result = ESUCCESS;
    if(FALSE == _pCntx->bExit)
    {
        evLogTrace(
                _pCntx->pQPktProcToBkgrnd,
                evLogLvlInfo,
                &(_pCntx->tLogDescSys),
                "Pkt Gen Thd is running with TID#%x",_pCntx->tid);
        _result =
                tcPktIORxMonIntfFromWire(_pCntx);
        tUtilUTCTimeGet(&(_pCntx->tDowntime));
        /* Dump temporary stats */
        tcPktPrcLogStats(_pCntx);
        /* Dump summary stats */
        _tcPktPrcSiteLogStatsSummary(_pCntx);
    }
    tcShProtectedDSetCompSts(tcTRCompTypePktPrc,tcTrStsDown);
    /****** 5. Thread Resource Destroy ********/
    if(_pCntx->tIntfX.bMonIntfOpen)
        tcPktIORxMonIntfClose(_pCntx);
    tcPktPrcInitSiteCleanupRes(_pCntx);
    /****** 6. Exit application ********/
    tcShProtectedDSetAppExitSts(_pCntx->bExit);

    return _result;
}

