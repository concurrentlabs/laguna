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
#include "tcmapintf.h"

//*************** external functions **********************
extern void clear_blacklist(void);
extern void populate_blacklist(const char * s);
extern void print_blacklist(void);

/**************** PRIVATE Functions **********************/

/***************************************************************************
 * function: _tcPktProcBlkListTbl
 *
 * description: loads ip black list table. The first entry is always
 * redirection address field.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcPktProcBlkListTbl(
        tc_pktprc_thread_ctxt_t*      pCntx,
        tc_ldcfg_conf_t*              pLdCfg)
{
    tresult_t           _result;
    U32                 _nHashVal;
    U32                 _i;
    tc_iphdr_ipaddr_t   _tIpAddr;
    CHAR*               _arg;
    CHAR*               _endStr;
    CHAR*               _tmpBuff;

    CCURASSERT(pCntx);
    CCURASSERT(pLdCfg);

    clear_blacklist();

    // load redirect addresses first.
    for(_i=0;_i<pCntx->tIntfX.nIntfMapTblTotal;_i++)
        populate_blacklist(pCntx->tIntfX.tIntfMapTbl[_i].pMonIntf->strRedirAddr);

    // load remaining blacklist ips.
    populate_blacklist(pLdCfg->strCmdArgIpBlackList);
    return ESUCCESS;

    //*******************************************************************
    // The code below is obsolete due to the new blacklist class - LCS.
    //*******************************************************************
    do
    {
        _result = ESUCCESS;
        ccur_memclear(&(pCntx->pHashIpAddrBlkListTbl[0]),
                sizeof(pCntx->pHashIpAddrBlkListTbl));
        pCntx->nHashIpAddrBlkListTbl = 0;
        for(_i=0;_i<pCntx->tIntfX.nIntfMapTblTotal;_i++)
        {
            if(pCntx->tIntfX.tIntfMapTbl[_i].pMonIntf)
            {
                /* load redirect ip address as 1st entry. */
                _result = tcUtilAsciitoIPAddr(
                        &_tIpAddr,
                        pCntx->tIntfX.tIntfMapTbl[_i].pMonIntf->strRedirAddr);
                if(ESUCCESS != _result)
                {
                    evLogTrace(
                            pCntx->pQPktProcToBkgrnd,
                           evLogLvlFatal,
                           &(pCntx->tLogDescSys),
                           "tcUtilAsciitoIPAddr() "
                           "ip conversion failed, redirection address for "
                           "monitoring interface: %s must be specified.",
                           pCntx->tIntfX.tIntfMapTbl[_i].pMonIntf->tIntf.strIntfName);
                    _result = EFAILURE;
                    break;
                }
                _result = tcUtilASymHashGet(
                            &_nHashVal,
                            &_tIpAddr);
                if(ESUCCESS != _result)
                {
                    evLogTrace(
                             pCntx->pQPktProcToBkgrnd,
                            evLogLvlFatal,
                            &(pCntx->tLogDescSys),
                            "ip conversion failed, redirection address for "
                            "monitoring interface: %s must be specified.",
                            pCntx->tIntfX.tIntfMapTbl[_i].pMonIntf->tIntf.strIntfName);
                     _result = EFAILURE;
                     break;
                 }
                 evLogTrace(
                          pCntx->pQPktProcToBkgrnd,
                         evLogLvlInfo,
                         &(pCntx->tLogDescSys),
                         "ip blacklist %lu:%s/%lu",
                         _i,pCntx->tIntfX.tIntfMapTbl[_i].pMonIntf->strRedirAddr,_nHashVal);
                 pCntx->pHashIpAddrBlkListTbl[_i] = _nHashVal;
                 pCntx->nHashIpAddrBlkListTbl++;
            }
        }
         if( '\0' == pLdCfg->strCmdArgIpBlackList[0])
             break;
         /* load the rest of ip black list table. */
         /* strdup() allocates memory, its okay to use it for
          * loading config but not for pkt processing or
          * other heavy duty processing. */
         _tmpBuff = strdup(pLdCfg->strCmdArgIpBlackList);
         if(_tmpBuff)
         {
             _arg = strtok_r(
                     _tmpBuff,",",&_endStr);
             _result = EINVAL;
             while(_arg)
             {
                 if(_i >= TRANSC_LDCFG_BLKLISTTBL_SZ)
                 {
                     _result = ENOBUFS;
                     break;
                 }
                 else
                     _result = ESUCCESS;
                 _result = tcUtilAsciitoIPAddr(
                         &_tIpAddr,_arg);
                 if(ESUCCESS != _result)
                 {
                     evLogTrace(
                             pCntx->pQPktProcToBkgrnd,
                            evLogLvlFatal,
                            &(pCntx->tLogDescSys),
                            "tcUtilAsciitoIPAddr() ip "
                            "conversion failed: %s",_arg);
                     _result = EFAILURE;
                     break;
                 }
                 _result = tcUtilASymHashGet(
                             &_nHashVal,
                             &_tIpAddr);
                 if(ESUCCESS != _result)
                 {
                     evLogTrace(
                             pCntx->pQPktProcToBkgrnd,
                            evLogLvlFatal,
                            &(pCntx->tLogDescSys),
                            "tcUtilASymHashGet() ip "
                            "conversion failed: %s",
                            _arg);
                     _result = EFAILURE;
                     break;
                 }
                 evLogTrace(
                          pCntx->pQPktProcToBkgrnd,
                         evLogLvlInfo,
                         &(pCntx->tLogDescSys),
                         "ip blacklist %lu:%s/%lu",
                         _i,_arg,_nHashVal);
                 pCntx->pHashIpAddrBlkListTbl[_i] = _nHashVal;
                 pCntx->nHashIpAddrBlkListTbl++;
                 _arg = strtok_r( NULL, "," ,&_endStr);
                 _i++;
             }
         }
         else
             _result = ENOMEM;
         if(_tmpBuff)
             free(_tmpBuff);
     }while(FALSE);
     evLogTrace(
             pCntx->pQPktProcToBkgrnd,
             evLogLvlDebug,
             &(pCntx->tLogDescSys),
             "blacklist table entries: %d",
             pCntx->nHashIpAddrBlkListTbl);

     return _result;
 }

/***************************************************************************
 * function: _tcPktPrcInitUriRegexTbl
 *
 * description: Loads and initialize all regex expressions for uri
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcPktPrcInitUriRegexTbl(
        tc_pktprc_thread_ctxt_t*     pCntx)
{
    tresult_t               _result;
    S32                     _iSites;
    U32                     _iUrl;
    tc_pktprc_httpuri_t*    _pUri;
    tc_regex_compile_t      _rCompile;

    CCURASSERT(pCntx);

    _rCompile.compileOptions  = 0;
    _rCompile.stdyOptions  = 0;
    _rCompile.pTableptr  = NULL;
    _result = ESUCCESS;

    for(_iSites=1;
            _iSites<pCntx->nPPSitesTbl;_iSites++)
    {
        if(_iSites >= TRANSC_LDCFG_SITE_MAXSITES_LST)
        {
            _result = EFAILURE;
            break;
        }
        for(_iUrl=0;
                _iUrl<pCntx->tPPSitesTbl[_iSites].nHttpURITbl;_iUrl++)
        {
            _result = EFAILURE;
            if(_iUrl >= TRANSC_LDCFG_SITE_MAXURITYPE_LST)
                break;
            /* Reload URL Signature */
            _pUri = &(pCntx->tPPSitesTbl[_iSites].tHttpURITbl[_iUrl]);
            if(_pUri->tSig.tReCkey.code)
            {
                pcre_free(_pUri->tSig.tReCkey.code);
                _pUri->tSig.tReCkey.code = NULL;
            }
            if(_pUri->tSig.tReCkey.extra)
            {
                pcre_free(_pUri->tSig.tReCkey.extra);
                _pUri->tSig.tReCkey.extra = NULL;
            }
            _rCompile.strPattern    = _pUri->tSig.strKey;
            _rCompile.pRegex        = &(_pUri->tSig.tReCkey);
            _result = tcRegexCompile(&_rCompile);
            /* Reload cachekeyrange Signature */
            if(ESUCCESS != _result)
                break;
            _result = ESUCCESS;
        }
        if(ESUCCESS != _result)
            break;
        _result = ESUCCESS;
    }

    return _result;
}

/***************************************************************************
 * function: _tcPktPrcInitSessRegexTbl
 *
 * description: Loads and initialize all regex expressions for session
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcPktPrcInitSessRegexTbl(
        tc_pktprc_thread_ctxt_t*     pCntx)
{
    tresult_t               _result;
    S32                     _iSites;
    U32                     _iSessId;
    U32                     _nSessId;
    tc_pktprc_httpsess_t*   _pSess;
    tc_regex_compile_t      _rCompile;

    CCURASSERT(pCntx);

    _rCompile.compileOptions  = 0;
    _rCompile.stdyOptions  = 0;
    _rCompile.pTableptr  = NULL;
    /* Count number of session id. If 0, then
     * success, that means session id is not specified. */
    _nSessId = 0;
    _result  = ESUCCESS;

    for(_iSites=1;
            _iSites<pCntx->nPPSitesTbl;_iSites++)
    {
        if(_iSites >= TRANSC_LDCFG_SITE_MAXSITES_LST)
        {
            _result = EFAILURE;
            break;
        }
        for(_iSessId=0;
                _iSessId<pCntx->tPPSitesTbl[_iSites].nHttpSessTbl;_iSessId++)
        {
            _nSessId++;
            if(_iSessId >= TRANSC_LDCFG_SITE_MAXSESSTYPE_LST)
            {
                _result = EFAILURE;
                break;
            }
        }
        if(ESUCCESS != _result)
            break;
    }
    if(_nSessId > 0)
    {
        /* populate and check */
        for(_iSites=1;
                _iSites<pCntx->nPPSitesTbl;_iSites++)
        {
            if(_iSites >= TRANSC_LDCFG_SITE_MAXSITES_LST)
            {
                _result = EFAILURE;
                break;
            }
            /* Reload Sess Signature */
            for(_iSessId=0;
                    _iSessId<pCntx->tPPSitesTbl[_iSites].nHttpSessTbl;_iSessId++)
            {
                _result = EFAILURE;
                if(_iSessId >= TRANSC_LDCFG_SITE_MAXSESSTYPE_LST)
                    break;
                /* Reload URL Signature */
                _pSess = &(pCntx->tPPSitesTbl[_iSites].tHttpSessTbl[_iSessId]);
                if(_pSess->tSig.tReCkey.code)
                {
                    pcre_free(_pSess->tSig.tReCkey.code);
                    _pSess->tSig.tReCkey.code = NULL;
                }
                if(_pSess->tSig.tReCkey.extra)
                {
                    pcre_free(_pSess->tSig.tReCkey.extra);
                    _pSess->tSig.tReCkey.extra = NULL;
                }
                _rCompile.strPattern    = _pSess->tSig.strKey;
                _rCompile.pRegex        = &(_pSess->tSig.tReCkey);
                _result = tcRegexCompile(&_rCompile);
                if(ESUCCESS != _result)
                    break;
                _result = ESUCCESS;
            }
            if(ESUCCESS != _result)
                break;
            _result = ESUCCESS;
        }
    }
    else
        _result = ESUCCESS;

    return _result;
}

/***************************************************************************
 * function: _tcPktPrcInitPopulateSites
 *
 * description: Initialize pkt processing control block based on information
 * from configuration file.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcPktPrcInitPopulateSites(
        tc_ldcfg_conf_t*            pLdCfg,
        tc_pktprc_thread_ctxt_t*    pCntx)
{
    tresult_t               _result;
    S32                     _iSites;
    U32                     _idx;
    tc_pktprc_httpuri_t*    _pUri;
    tc_ldcfg_httpuri_t*     _pLdCfgUri;
    tc_pktprc_httpsess_t*   _pSess;
    tc_ldcfg_httpsess_t*    _pLdCfgSess;

    CCURASSERT(pLdCfg);
    CCURASSERT(pCntx);

    do
    {
        _result = ESUCCESS;
        tcPktPrcInitSiteCleanupRes(pCntx);
        ccur_memclear(pCntx->tPPSitesTbl,
                sizeof(pCntx->tPPSitesTbl));
        /* Expand  loadable config and copy all the info */
        /* Starts from 1,
         * 0 suppose to be unknown sites resources */
        /* Services */
        pCntx->nPPSitesTbl = pLdCfg->nSites;
        for(_iSites=1;_iSites<pCntx->nPPSitesTbl;_iSites++)
        {
            pCntx->tPPSitesTbl[_iSites].nHttpURITbl =
                    pLdCfg->tSites[_iSites].nCfgArgURI;
            pCntx->tPPSitesTbl[_iSites].nHttpSessTbl =
                    pLdCfg->tSites[_iSites].nCfgArgSess;
        }
        for(_iSites=1;
                _iSites<pCntx->nPPSitesTbl;_iSites++)
        {
            /* ---> Breakdown sess string into list of values */
            for(_idx=0;
                    _idx<pCntx->tPPSitesTbl[_iSites].nHttpSessTbl;_idx++)
            {
                _pSess = &(pCntx->tPPSitesTbl[_iSites].tHttpSessTbl[_idx]);
                _pLdCfgSess = &(pLdCfg->tSites[_iSites].tCfgArgSess[_idx]);
                if('\0' != _pLdCfgSess->strCfgArgSessSig[0])
                {
                    ccur_strlcpy(_pSess->tSig.strKey,
                            _pLdCfgSess->strCfgArgSessSig,
                            sizeof(_pSess->tSig.strKey));
                }
                if('\0' != _pLdCfgSess->strCfgArgMatchSz[0])
                {
                    _pSess->nMatchSz =
                            strtol(_pLdCfgSess->strCfgArgMatchSz,
                                 (char **)NULL, 10);
                }
            }
            if(ESUCCESS != _result)
                break;
            /* ---> Breakdown url strings into list of values */
            for(_idx=0;
                    _idx<pCntx->tPPSitesTbl[_iSites].nHttpURITbl;_idx++)
            {
                _pUri       = &(pCntx->tPPSitesTbl[_iSites].tHttpURITbl[_idx]);
                _pLdCfgUri  = &(pLdCfg->tSites[_iSites].tCfgArgURI[_idx]);
                if('\0' != _pLdCfgUri->strCfgArgUriSig[0])
                {
                    ccur_strlcpy(_pUri->tSig.strKey,
                            _pLdCfgUri->strCfgArgUriSig,
                            sizeof(_pUri->tSig.strKey));
                }
                if('\0' != _pLdCfgUri->strCfgArgMatchSz[0])
                {
                    _pUri->nMatchSz =
                            strtol(_pLdCfgUri->strCfgArgMatchSz,
                                 (char **)NULL, 10);
                }
            }
            if(ESUCCESS != _result)
                break;
        }
        if(ESUCCESS != _result)
            break;
        _result = _tcPktPrcInitSessRegexTbl(pCntx);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                   evLogLvlFatal,
                   &(pCntx->tLogDescSys),
                   "unable to load content session");
            break;
        }
        _result = _tcPktPrcInitUriRegexTbl(pCntx);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                   evLogLvlFatal,
                   &(pCntx->tLogDescSys),
                   "unable to load content URI list");
            break;
        }
    }while(FALSE);

    return _result;
}

/**************** PROTECTED Functions **********************/

/***************************************************************************
 * function: _tcPktPrcInitSiteCleanupRes
 *
 * description: Cleans up the service resources.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktPrcInitSiteCleanupRes(
        tc_pktprc_thread_ctxt_t *     pCntx)
{
    S32                     _iSites;
    U32                     _idxa;
    tc_pktprc_httpuri_t*    _pUri;
    tc_pktprc_httpsess_t*   _pSess;

    CCURASSERT(pCntx);

    /* Cleanup all sites regex */
    for(_iSites=1;
            _iSites<pCntx->nPPSitesTbl;_iSites++)
    {
        /* Cleanup Sessions */
        for(_idxa=0;
                _idxa<pCntx->tPPSitesTbl[_iSites].nHttpSessTbl;_idxa++)
        {
            /* Free URL Signature */
            _pSess = &(pCntx->tPPSitesTbl[_iSites].tHttpSessTbl[_idxa]);
            if(_pSess->tSig.tReCkey.code)
            {
                pcre_free(_pSess->tSig.tReCkey.code);
                _pSess->tSig.tReCkey.code = NULL;
            }
            if(_pSess->tSig.tReCkey.extra)
            {
                pcre_free(_pSess->tSig.tReCkey.extra);
                _pSess->tSig.tReCkey.extra = NULL;
            }
        }
        /* Clean up URL */
        for(_idxa=0;
                _idxa<pCntx->tPPSitesTbl[_iSites].nHttpURITbl;_idxa++)
        {
            /* Free URL Signature */
            _pUri = &(pCntx->tPPSitesTbl[_iSites].tHttpURITbl[_idxa]);
            if(_pUri->tSig.tReCkey.code)
            {
                pcre_free(_pUri->tSig.tReCkey.code);
                _pUri->tSig.tReCkey.code = NULL;
            }
            if(_pUri->tSig.tReCkey.extra)
            {
                pcre_free(_pUri->tSig.tReCkey.extra);
                _pUri->tSig.tReCkey.extra = NULL;
            }
        }
    }
}

/***************************************************************************
 * function: tcPktProcInitIntf
 *
 * description: Open outgoing or monitoring interface if not being set,
 * if already opened then closing and reopening the interface.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktPrcInitIntf(
        tc_pktprc_thread_ctxt_t*     pCntx,
        BOOL                         bRxLoad)
{
    tresult_t   _result;

    CCURASSERT(pCntx);

    _result = ESUCCESS;

    if(bRxLoad)
    {
        if(pCntx->tIntfX.bMonIntfOpen)
            tcPktIORxMonIntfClose(pCntx);
        if(FALSE == pCntx->tIntfX.bMonIntfOpen)
            _result = tcPktIORxMonIntfOpen(pCntx);
    }

    return _result;
}

/***************************************************************************
 * function: tcPktProcLogDownStatusAndRetry
 *
 * description: Dumps the reason why TCS is down and retry in the case
 * network interface is not up yet.
 * TCS can be down due to network interface is not up yet or
 * configuration failure.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktProcInitLogDownStatusAndRetry(
        tc_pktprc_thread_ctxt_t*     pCntx)
{
    BOOL _bWait = TRUE;
    /* Wait until both monitoring and
     * output interfaces are up and running
     * then continue.
     */
    if(FALSE == pCntx->bNonIntfCfgLoadSuccess)
    {
        evLogTrace(
              pCntx->pQPktProcToBkgrnd,
              evLogLvlError,
              &(pCntx->tLogDescSys),
              "TCS is down, signature service "
              "configuration loading failure, "
              "please check config file.");
    }
    else if(FALSE == pCntx->bIntfCfgLoadSuccess)
    {
        evLogTrace(
              pCntx->pQPktProcToBkgrnd,
              evLogLvlError,
              &(pCntx->tLogDescSys),
              "TCS is down, interface "
              "configuration loading failure, "
              "please check config file.");
    }
    else if(FALSE == pCntx->tIntfX.bMonIntfUp)
    {
        /* if Rings are not initialized then try to
         * initialize it, this case happens when
         * pfring trying to open interface not
         * up yet in case of fresh bootup with
         * multiple interfaces. */
        if(FALSE == pCntx->tIntfX.bMonIntfOpen)
            tcPktPrcInitIntf(pCntx,TRUE);
        /* Check to make sure both links are
         * up and running */
        if(pCntx->tIntfX.bMonIntfOpen &&
           tcPktIORxMonIntfIsLinkUp(pCntx))
            pCntx->tIntfX.bMonIntfUp = TRUE;
        else
            pCntx->tIntfX.bMonIntfUp = FALSE;
        /* Definition of transc is up :
           transc all system is ready to process.
           - monitoring up,
           - injection up,
           - all threads up,
           - configuration successfully loaded. */
        /* TODO: Add all threads up check */
        if(pCntx->bIntfCfgLoadSuccess &&
           pCntx->bNonIntfCfgLoadSuccess &&
           pCntx->tIntfX.bMonIntfUp)
        {
            pCntx->bTrSts = tcTrStsUp;
            _bWait = FALSE;
        }
    }
    else
    {
        evLogTrace(
              pCntx->pQPktProcToBkgrnd,
              evLogLvlError,
              &(pCntx->tLogDescSys),
              "TCS is down");
    }
    if(_bWait)
        sleep(5);
}

CCUR_PROTECTED(tresult_t)
tcPktProcConfigInitLoadableRes(
        tc_pktprc_thread_ctxt_t*     pCntx,
        tc_ldcfg_conf_t*             pLdCfg)
{
    tresult_t                   _sts;
    BOOL                        _bRxLoad;
    BOOL                        _bTxLoad;
    BOOL                        _bMapLoad;
    BOOL                        _bRedirLoad;
    U32                         _i;

    CCURASSERT(pCntx);
    CCURASSERT(pLdCfg);

    evLogTrace(
          pCntx->pQPktProcToBkgrnd,
          evLogLvlWarn,
          &(pCntx->tLogDescSys),
          "Warning, pktproc is down, reloading config!");

    do
    {
        pCntx->bTrSts = tcTrStsDown;
        pCntx->bNonIntfCfgLoadSuccess = FALSE;
        pCntx->bIntfCfgLoadSuccess = FALSE;
        if(!strcasecmp("true",
                pLdCfg->strCmdArgProcessProxyReq))
            pCntx->bProcessProxyReq = TRUE;
        else
            pCntx->bProcessProxyReq = FALSE;
        /* unpack loadable config info and
         * copy only necessary data to pkt processing
         * thread. */
        _sts = _tcPktPrcInitPopulateSites(
                                        pLdCfg,pCntx);
        if(ESUCCESS != _sts)
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                   evLogLvlFatal,
                   &(pCntx->tLogDescSys),
                   "failure to load data to pkt proc data structure\n");
            break;
        }
        /* At this point all non-interface has been successfully loaded */
        pCntx->bNonIntfCfgLoadSuccess = TRUE;
        /* initialize all interfaces */
        /* First Check to see if we need to reload RX or TX before
         * rewritting the context data structure. */
        tcIntfCkIntf(&(pCntx->tIntfCfg),
                pLdCfg,&_bRxLoad,&_bTxLoad,&_bMapLoad,&_bRedirLoad);
        if(_bRxLoad)
        {
            /* Reloading, set the RX interface to down */
            pCntx->tIntfX.bMonIntfUp = FALSE;
            if(pCntx->tIntfX.bMonIntfOpen)
                tcPktIORxMonIntfClose(pCntx);
        }
        _sts =
                tcMonIntfConfigYamlInitAllInterfaceTbls(
                        &(pCntx->tIntfX),&(pCntx->tIntfCfg),pLdCfg,
                            _bRxLoad,_bTxLoad,_bMapLoad,_bRedirLoad);
        if(ESUCCESS != _sts)
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                  evLogLvlError,
                  &(pCntx->tLogDescSys),
                  "failure, config.yaml loading, intf init");
            break;
        }
        if(_bRxLoad)
        {
             if(FALSE == pCntx->tIntfX.bMonIntfOpen)
             {
                 _sts = tcPktIORxMonIntfOpen(pCntx);
                 if(ESUCCESS != _sts)
                 {
                     evLogTrace(
                           pCntx->pQPktProcToBkgrnd,
                           evLogLvlError,
                           &(pCntx->tLogDescSys),
                           "failure to open RX /Interface");
                     break;
                 }
             }
         }
         /* Init all loadable tables to be passed to thread */
         _sts = _tcPktProcBlkListTbl(pCntx,pLdCfg);
         if( ESUCCESS != _sts)
         {
printf("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n");
             evLogTrace(
                   pCntx->pQPktProcToBkgrnd,
                   evLogLvlError,
                   &(pCntx->tLogDescSys),
                   "unable to init black list table");
             break;
         }
         pCntx->tIntfX.nTotalMonMappedRings = 0;
         /* Find Total monitored interface mapped rings */
         for(_i=0;_i<pCntx->tIntfX.nIntfMapTblTotal;_i++)
         {
             if(pCntx->tIntfX.tIntfMapTbl[_i].pMonIntf)
                 pCntx->tIntfX.nTotalMonMappedRings +=
                         pCntx->tIntfX.tIntfMapTbl[_i].pMonIntf->nFilterActv;
         }
         evLogTrace(
               pCntx->pQPktProcToBkgrnd,
               evLogLvlInfo,
               &(pCntx->tLogDescSys),
               "Total Active Rings:%lu",
               pCntx->tIntfX.nTotalMonMappedRings);
         pCntx->bIntfCfgLoadSuccess = TRUE;
        /* Definition of transc is up :
           transc all system is ready to process.
           - monitoring up,
           - injection up,
           - all threads up,
           - configuration successfully loaded. */
        /* TODO: Add all threads up check */
        if(pCntx->bIntfCfgLoadSuccess &&
           pCntx->bNonIntfCfgLoadSuccess &&
           pCntx->tIntfX.bMonIntfUp)
        {
            pCntx->bTrSts = tcTrStsUp;
        }
    }while(FALSE);

    /* Cleanup operations */
    if(tcTrStsUp != pCntx->bTrSts)
    {
        if(FALSE == pCntx->bNonIntfCfgLoadSuccess)
            tcPktPrcInitSiteCleanupRes(pCntx);
        /* Other cleanup here if necessary */
    }
    else
    {
        _sts = tcMonIntfInitMapCheck(
                &(pCntx->tIntfX),&(pCntx->tIntfCfg));
        if(ESUCCESS != _sts)
        {
            pCntx->bTrSts = tcTrStsDown;
            pCntx->bIntfCfgLoadSuccess = FALSE;
            evLogTrace(
                  pCntx->pQPktProcToBkgrnd,
                  evLogLvlError,
                  &(pCntx->tLogDescSys),
                  "failure, map interface link config, please "
                  "check configuration.");
        }
        else
        {
            evLogTrace(
                  pCntx->pQPktProcToBkgrnd,
                  evLogLvlWarn,
                  &(pCntx->tLogDescSys),
                  "Warning, pktproc is UP, Finished reloading config!");
        }
    }
    /* Set Component status */
    tcShProtectedDSetCompSts(tcTRCompTypePktPrc,pCntx->bTrSts);

    return (pCntx->bTrSts ? ESUCCESS : EFAILURE);
}


