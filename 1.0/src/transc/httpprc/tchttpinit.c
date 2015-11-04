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
/**************************PRIVATE******************************************/
/***************************************************************************
 * function: _tcPktProcHttpHdrMatchSplit
 *
 * description: split http header string into multiple array entries delimited by ;
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcPktProcHttpHdrMatchSplit(
        CHAR*                       strTmp,
        tc_httpprc_httphdmatch_t*    pPktPrcArgHttpHdrMatch,
        U16*                        pnHttpHdrMatch)
{

    tresult_t   _result;
    U32         _i;
    CHAR*       _ptr;
    CHAR*       _arg;
    CHAR*       _endStr;
    CHAR*       _tmpBuff;
    U16         _nFldLen;

    _result = ESUCCESS;
    _tmpBuff = strdup(strTmp);
    if(_tmpBuff)
    {
        _arg = strtok_r(_tmpBuff,";\"",&_endStr);
        _i   = 0;
        while(_arg)
        {
            if(_i < TRANSC_HTTPPRC_SITE_MAXHTTPHDRMATCH_LST)
            {
                _ptr = strchr(_arg,':');
                _nFldLen = (_ptr-_arg);
                if(_ptr && _nFldLen &&
                   _nFldLen < sizeof(pPktPrcArgHttpHdrMatch[_i].strFieldName)-1)
                {
                    strncpy(pPktPrcArgHttpHdrMatch[_i].strFieldName,
                            _arg,_nFldLen);
                    pPktPrcArgHttpHdrMatch[_i].strFieldName[_nFldLen] = '\0';
                    if('\0' != *(_ptr++))
                    {
                        pPktPrcArgHttpHdrMatch[_i].tFieldValue.strKey[0] = '\0';
                        ccur_strlcpy(pPktPrcArgHttpHdrMatch[_i].tFieldValue.strKey,_ptr,
                                sizeof(pPktPrcArgHttpHdrMatch[_i].tFieldValue.strKey));
                        (*pnHttpHdrMatch)++;
                    }
                }
                _arg = strtok_r( NULL, ";\"" ,&_endStr);
            }
            else
            {
                _result = ENOMEM;
                break;
            }
            _i++;
        }
    }
    else
        _result = ENOMEM;

    if(_tmpBuff)
        free(_tmpBuff);

    return _result;
}

/***************************************************************************
 * function: _tcHttpInitHostSplit
 *
 * description: split host string into multiple array entries delimited by ;
 * TODO: redundant, need to remove this function
 * and use _tcHttpInitValueSplit() instead
 ***************************************************************************/

CCUR_PRIVATE(tresult_t)
_tcHttpInitHostSplit(
        CHAR*                  strTmp,
        tc_httpprc_httpsites_t* pHosts)
{

    tresult_t   _result;
    U32         _i;
    CHAR*       _arg;
    CHAR*       _endStr;
    CHAR*       _tmpBuff;

    _result = ESUCCESS;
    _tmpBuff = strdup(strTmp);
    if(_tmpBuff)
    {
        _arg = strtok_r(_tmpBuff,";\"",&_endStr);
        _i   = 0;
        while(_arg)
        {
            if(_i < TRANSC_HTTPPRC_SITE_MAXHOSTYPE_LST)
            {
                ccur_strlcpy(pHosts->tHost[_i].strHostSig,
                        _arg,sizeof(pHosts->tHost[_i].strHostSig));
                pHosts->nHost++;
                _arg = strtok_r( NULL, ";\"" ,&_endStr);
            }
            else
            {
                _result = ENOMEM;
                break;
            }
            _i++;
        }
    }
    else
        _result = ENOMEM;
    if(_tmpBuff)
        free(_tmpBuff);

    return _result;
}

/***************************************************************************
 * function: _tcHttpInitCkeyFixString
 *
 * description: cleanup and copy string
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpInitCkeyFixString(
        CHAR*      strKeyOut,
        U32        nStrKeyOut,
        CHAR*      strKey,
        U32        nStrKey)
{
    tresult_t _result;
#if 0
    CHAR*     _pstrS;
    CHAR*     _pstrE;
    CHAR*     _psc;

    do
    {
        _result = EFAILURE;
        if(nStrKeyOut < nStrKey)
            break;
        /* Find first " */
        _pstrS = strchr(strKey,"\"");
        if(_pstrS)
        {
            _pstrE = strchr(_pstrS,"\"");

            /* Find another " or EOF */
            while(_pstrS)
            {

            }
            _result = ESUCCESS;
        }
        else
        {
            ccur_strlcpy(strKeyOut,strKey,nStrKeyOut);
            _result = ESUCCESS;
        }
    }while(FALSE);
#else
    ccur_strlcpy(strKeyOut,strKey,nStrKeyOut);
    _result = ESUCCESS;
#endif

    return (_result);
}

/***************************************************************************
 * function: _tcHttpInitValueSplit
 *
 * description: split string into multiple array entries delimited by ;
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpInitValueSplit(
        CHAR*                  strTmp,
        tc_httpprc_ckeyq_t*     pValId,
        U32                    nValIdMaxLst,
        U16*                   pnValId)
{
    tresult_t   _result;
    U32         _i;
    CHAR*       _pStrKey;
    U16         _nStrKey;
    CHAR*       _arg;
    CHAR*       _endStr;
    CHAR*       _tmpBuff;

    _i = 0;
    _tmpBuff = strdup(strTmp);
    if(_tmpBuff)
    {
        _arg = strtok_r(
                _tmpBuff,";\"",&_endStr);
        _result = EINVAL;
        while(_arg)
        {
            if(_i >= nValIdMaxLst)
            {
                _result = ENOBUFS;
                break;
            }
            else
                _result = ESUCCESS;
            _pStrKey = pValId[_i].strKey;
            _nStrKey = sizeof(pValId[_i].strKey);
            ccur_memclear(_pStrKey,
                          _nStrKey);
            _result = _tcHttpInitCkeyFixString(
                    _pStrKey,_nStrKey,_arg,strlen(_arg));
            if(ESUCCESS != _result)
                break;
            (*pnValId)++;
            _arg = strtok_r( NULL, ";\"" ,&_endStr);
            _i++;
        }
    }
    else
        _result = ENOMEM;
    if(_tmpBuff)
        free(_tmpBuff);

    return _result;
}

/***************************************************************************
 * function: _tcHttpInitSessRegexTbl
 *
 * description: Loads and initialize all regex expressions for session
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpInitSessRegexTbl(
        tc_httpprc_thread_ctxt_t*     pCntx)
{
    tresult_t               _result;
    S32                     _iSites;
    U32                     _iSessId;
    U32                     _nSessId;
    U32                     _ickey;
    tc_httpprc_httpsess_t*   _pSess;
    tc_regex_compile_t      _rCompile;

    CCURASSERT(pCntx);

    _rCompile.compileOptions  = 0;
    _rCompile.stdyOptions  = 0;
    _rCompile.pTableptr  = NULL;
    /* Count number of session id. If 0, then
     * success, that means session id is not specified. */
    _nSessId = 0;
    _result = ESUCCESS;
    for(_iSites=1;
            _iSites<pCntx->nPPSitesTbl;_iSites++)
    {
        if(_iSites >= TRANSC_LDCFG_SITE_MAXSITES_LST)
        {
            _result = EFAILURE;
            break;
        }
        for(_iSessId=0;
                _iSessId<pCntx->tPPSitesTbl[_iSites].nHttpSess;_iSessId++)
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
    if(ESUCCESS == _result && _nSessId > 0)
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
                    _iSessId<pCntx->tPPSitesTbl[_iSites].nHttpSess;_iSessId++)
            {
                _result = EFAILURE;
                if(_iSessId >= TRANSC_LDCFG_SITE_MAXSESSTYPE_LST)
                    break;
                /* Reload URL Signature */
                _pSess = &(pCntx->tPPSitesTbl[_iSites].tHttpSess[_iSessId]);
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
                /* Reload cachekeyrange Signature */
                if(_pSess->tCKeyRange.tReCkey.code)
                {
                    pcre_free(_pSess->tCKeyRange.tReCkey.code);
                    _pSess->tCKeyRange.tReCkey.code = NULL;
                }
                if(_pSess->tCKeyRange.tReCkey.extra)
                {
                    pcre_free(_pSess->tCKeyRange.tReCkey.extra);
                    _pSess->tCKeyRange.tReCkey.extra = NULL;
                }
                _rCompile.strPattern    = _pSess->tCKeyRange.strKey;
                _rCompile.pRegex        = &(_pSess->tCKeyRange.tReCkey);
                _result = tcRegexCompile(&_rCompile);
                if(ESUCCESS != _result)
                    break;
                /* Reload Cache Key Id Signature */
                for(_ickey=0;
                        _ickey<_pSess->nCKeyId;_ickey++)
                {
                    _result     = EFAILURE;
                    if(_ickey >= TRANSC_HTTPPRC_URL_CKEYARGS_LST)
                        break;
                    if(_pSess->tCKeyId[_ickey].tReCkey.code)
                    {
                        pcre_free(_pSess->tCKeyId[_ickey].tReCkey.code);
                        _pSess->tCKeyId[_ickey].tReCkey.code = NULL;
                    }
                    if(_pSess->tCKeyId[_ickey].tReCkey.extra)
                    {
                        pcre_free(_pSess->tCKeyId[_ickey].tReCkey.extra);
                        _pSess->tCKeyId[_ickey].tReCkey.extra = NULL;
                    }
                    _rCompile.strPattern =
                            _pSess->tCKeyId[_ickey].strKey;
                    _rCompile.pRegex     =
                            &(_pSess->tCKeyId[_ickey].tReCkey);
                    _result = tcRegexCompile(&_rCompile);
                    if(ESUCCESS != _result)
                        break;
                }
                if(ESUCCESS != _result)
                    break;
                /* Reload Cache Key Id Signature */
                for(_ickey=0;
                        _ickey<_pSess->nCtxId;_ickey++)
                {
                    _result     = EFAILURE;
                    if(_ickey >= TRANSC_HTTPPRC_URL_CKEYARGS_LST)
                        break;
                    if(_pSess->tCtxId[_ickey].tReCkey.code)
                    {
                        pcre_free(_pSess->tCtxId[_ickey].tReCkey.code);
                        _pSess->tCtxId[_ickey].tReCkey.code = NULL;
                    }
                    if(_pSess->tCtxId[_ickey].tReCkey.extra)
                    {
                        pcre_free(_pSess->tCtxId[_ickey].tReCkey.extra);
                        _pSess->tCtxId[_ickey].tReCkey.extra = NULL;
                    }
                    _rCompile.strPattern =
                            _pSess->tCtxId[_ickey].strKey;
                    _rCompile.pRegex     =
                            &(_pSess->tCtxId[_ickey].tReCkey);
                    _result = tcRegexCompile(&_rCompile);
                    if(ESUCCESS != _result)
                        break;
                }
                if(ESUCCESS != _result)
                    break;
                /* Reload Cache Key Misc Signature */
                for(_ickey=0;
                        _ickey<_pSess->nCKeyMisc;_ickey++)
                {
                    _result     = EFAILURE;
                    if(_ickey >= TRANSC_HTTPPRC_URL_CKEYARGS_LST)
                        break;
                    if(_pSess->tCkeyMisc[_ickey].tReCkey.code)
                    {
                        pcre_free(_pSess->tCkeyMisc[_ickey].tReCkey.code);
                        _pSess->tCkeyMisc[_ickey].tReCkey.code = NULL;
                    }
                    if(_pSess->tCkeyMisc[_ickey].tReCkey.extra)
                    {
                        pcre_free(_pSess->tCkeyMisc[_ickey].tReCkey.extra);
                        _pSess->tCkeyMisc[_ickey].tReCkey.extra = NULL;
                    }
                    _rCompile.strPattern =
                            _pSess->tCkeyMisc[_ickey].strKey;
                    _rCompile.pRegex     =
                            &(_pSess->tCkeyMisc[_ickey].tReCkey);
                    _result = tcRegexCompile(&_rCompile);
                    if(ESUCCESS != _result)
                        break;
                }
                if(ESUCCESS != _result)
                    break;
                _result = ESUCCESS;
            }
            if(ESUCCESS != _result)
                break;
            _result = ESUCCESS;
        }
    }

    return _result;
}

/***************************************************************************
 * function: _tcHttpInitSessUriRelantionships
 *
 * description: build relationships between session with uri value.
 * if the value is the same, then there is a link between the two.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHttpInitSessUriRelantionships(
        tc_httpprc_thread_ctxt_t*     pCntx)
{
    S32                     _iSites;
    U32                     _iUrl;
    U32                     _ickey;
    tc_httpprc_httpuri_t*    _pUri;
    tc_httpprc_httpsess_t*   _pSess;
    U32                     _iSessId;
    BOOL                    _bFnd;

    /* Check if there is a relationships
     * between ckey cntx and ckey id for
     * each url */
    for(_iSites=1;
            _iSites<pCntx->nPPSitesTbl;_iSites++)
    {
        for(_iUrl=0;
                _iUrl<pCntx->tPPSitesTbl[_iSites].nHttpURI;_iUrl++)
        {
            _pUri = &(pCntx->tPPSitesTbl[_iSites].tHttpURI[_iUrl]);
            _bFnd = FALSE;
            for(_iSessId=0;
                    _iSessId<pCntx->tPPSitesTbl[_iSites].nHttpSess;_iSessId++)
            {
                _pSess = &(pCntx->tPPSitesTbl[_iSites].tHttpSess[_iSessId]);
                for(_ickey=0;
                        _ickey<_pSess->nCtxId;_ickey++)
                {
                    if('\0' != _pUri->tCtxId[_ickey].strKey[0])
                    {
                        if(!strcmp(_pUri->tCtxId[_ickey].strKey,
                                _pSess->tCtxId[_ickey].strKey))
                        {
                            _bFnd = TRUE;
                            break;
                        }
                    }
                }
                if(_bFnd)
                    break;
            }
            if(_bFnd)
                pCntx->tPPSitesTbl[_iSites].tHttpURI[_iUrl].bSessRel = TRUE;
        }
    }
}

/***************************************************************************
 * function: _tcHttpInitUriRegexTbl
 *
 * description: Loads and initialize all regex expressions for uri
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpInitUriRegexTbl(
        tc_httpprc_thread_ctxt_t*     pCntx)
{
    tresult_t               _result;
    S32                     _iSites;
    U32                     _iUrl;
    U32                     _ickey;
    tc_httpprc_httpuri_t*    _pUri;
    tc_regex_compile_t      _rCompile;

    CCURASSERT(pCntx);

    _result = ESUCCESS;
    _rCompile.compileOptions  = 0;
    _rCompile.stdyOptions  = 0;
    _rCompile.pTableptr  = NULL;

    for(_iSites=1;
            _iSites<pCntx->nPPSitesTbl;_iSites++)
    {
        if(_iSites >= TRANSC_LDCFG_SITE_MAXSITES_LST)
        {
            _result = EFAILURE;
            break;
        }
        for(_iUrl=0;
                _iUrl<pCntx->tPPSitesTbl[_iSites].nHttpURI;_iUrl++)
        {
            _result = EFAILURE;
            if(_iUrl >= TRANSC_LDCFG_SITE_MAXURITYPE_LST)
                break;
            /* Reload URL Signature */
            _pUri = &(pCntx->tPPSitesTbl[_iSites].tHttpURI[_iUrl]);
            _pUri->bSessRel = FALSE;
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
            if(_pUri->tCKeyRange.tReCkey.code)
            {
                pcre_free(_pUri->tCKeyRange.tReCkey.code);
                _pUri->tCKeyRange.tReCkey.code = NULL;
            }
            if(_pUri->tCKeyRange.tReCkey.extra)
            {
                pcre_free(_pUri->tCKeyRange.tReCkey.extra);
                _pUri->tCKeyRange.tReCkey.extra = NULL;
            }
            _rCompile.strPattern    = _pUri->tCKeyRange.strKey;
            _rCompile.pRegex        = &(_pUri->tCKeyRange.tReCkey);
            _result = tcRegexCompile(&_rCompile);
            if(ESUCCESS != _result)
                break;
            /* Reload Cache key cntx Id Signature */
            for(_ickey=0;
                    _ickey<_pUri->nCtxId;_ickey++)
            {
                _result     = EFAILURE;
                if(_ickey >= TRANSC_HTTPPRC_URL_CKEYARGS_LST)
                    break;
                /* Reload Cache Key Id Signature */
                if(_pUri->tCtxId[_ickey].tReCkey.code)
                {
                    pcre_free(_pUri->tCtxId[_ickey].tReCkey.code);
                    _pUri->tCtxId[_ickey].tReCkey.code = NULL;
                }
                if(_pUri->tCtxId[_ickey].tReCkey.extra)
                {
                    pcre_free(_pUri->tCtxId[_ickey].tReCkey.extra);
                    _pUri->tCtxId[_ickey].tReCkey.extra = NULL;
                }
                _rCompile.strPattern =
                        _pUri->tCtxId[_ickey].strKey;
                _rCompile.pRegex     =
                        &(_pUri->tCtxId[_ickey].tReCkey);
                _result = tcRegexCompile(&_rCompile);
                if(ESUCCESS != _result)
                    break;
            }
            if(ESUCCESS != _result)
                break;
            /* Reload Cache key Id Signature */
            for(_ickey=0;
                    _ickey<_pUri->nCKeyId;_ickey++)
            {
                _result     = EFAILURE;
                if(_ickey >= TRANSC_HTTPPRC_URL_CKEYARGS_LST)
                    break;
                /* Reload Cache Key Id Signature */
                if(_pUri->tCKeyId[_ickey].tReCkey.code)
                {
                    pcre_free(_pUri->tCKeyId[_ickey].tReCkey.code);
                    _pUri->tCKeyId[_ickey].tReCkey.code = NULL;
                }
                if(_pUri->tCKeyId[_ickey].tReCkey.extra)
                {
                    pcre_free(_pUri->tCKeyId[_ickey].tReCkey.extra);
                    _pUri->tCKeyId[_ickey].tReCkey.extra = NULL;
                }
                _rCompile.strPattern =
                        _pUri->tCKeyId[_ickey].strKey;
                _rCompile.pRegex     =
                        &(_pUri->tCKeyId[_ickey].tReCkey);
                _result = tcRegexCompile(&_rCompile);
                if(ESUCCESS != _result)
                    break;
            }
            if(ESUCCESS != _result)
                break;
            /* Reload Cache Key Misc Signature */
            for(_ickey=0;
                    _ickey<_pUri->nCKeyMisc;_ickey++)
            {
                _result     = EFAILURE;
                if(_ickey >= TRANSC_HTTPPRC_URL_CKEYARGS_LST)
                    break;
                if(_pUri->tCkeyMisc[_ickey].tReCkey.code)
                {
                    pcre_free(_pUri->tCkeyMisc[_ickey].tReCkey.code);
                    _pUri->tCkeyMisc[_ickey].tReCkey.code = NULL;
                }
                if(_pUri->tCkeyMisc[_ickey].tReCkey.extra)
                {
                    pcre_free(_pUri->tCkeyMisc[_ickey].tReCkey.extra);
                    _pUri->tCkeyMisc[_ickey].tReCkey.extra = NULL;
                }
                _rCompile.strPattern =
                        _pUri->tCkeyMisc[_ickey].strKey;
                _rCompile.pRegex     =
                        &(_pUri->tCkeyMisc[_ickey].tReCkey);
                _result = tcRegexCompile(&_rCompile);
                if(ESUCCESS != _result)
                    break;
            }
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
 * function: _tcHttpInitUriRegexTbl
 *
 * description: Init all URI sites
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpInitUriSitesTbl(
        tc_httpprc_thread_ctxt_t*     pCntx)
{
    tresult_t               _result;
    S32                     _iSites;
    U32                     _idx;
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
        _result = ESUCCESS;
        for(_idx=0;
                _idx<pCntx->tPPSitesTbl[_iSites].nHost;_idx++)
        {
            _result = EFAILURE;
            if(_idx >= TRANSC_HTTPPRC_SITE_MAXHOSTYPE_LST)
                break;
            /* Reload Hostname Signature */
            if(pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.code)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.code);
                pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.code = NULL;
            }
            if(pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.extra)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.extra);
                pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.extra = NULL;
            }
            _rCompile.strPattern =
                    pCntx->tPPSitesTbl[_iSites].tHost[_idx].strHostSig;
            _rCompile.pRegex     =
                    &(pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost);
            _result = tcRegexCompile(&_rCompile);
            if(ESUCCESS != _result)
                break;
        }
        if(ESUCCESS != _result)
            break;
        for(_idx=0;
                _idx<pCntx->tPPSitesTbl[_iSites].nReferrer;_idx++)
        {
            _result = EFAILURE;
            if(_idx >= TRANSC_HTTPPRC_SITE_MAXREFTYPE_LST)
                break;
            /* Reload Hostname Signature */
            if(pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.code)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.code);
                pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.code = NULL;
            }
            if(pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.extra)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.extra);
                pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.extra = NULL;
            }
            _rCompile.strPattern =
                    pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].strKey;
            _rCompile.pRegex     =
                    &(pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey);
            _result = tcRegexCompile(&_rCompile);
            if(ESUCCESS != _result)
                break;
        }
        if(ESUCCESS != _result)
            break;
        for(_idx=0;
                _idx<pCntx->tPPSitesTbl[_iSites].nHttpHdrMatch;_idx++)
        {
            _result = EFAILURE;
            if(_idx >= TRANSC_HTTPPRC_SITE_MAXHTTPHDRMATCH_LST)
                break;
            /* Reload http hdr match Signature */
            if(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.code)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.code);
                pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.code = NULL;
            }
            if(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.extra)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.extra);
                pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.extra = NULL;
            }
            _rCompile.strPattern =
                    pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.strKey;
            _rCompile.pRegex     =
                    &(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey);
            _result = tcRegexCompile(&_rCompile);
            if(ESUCCESS != _result)
                break;
        }
        if(ESUCCESS != _result)
            break;
        /* Reload httprangefield  */
        if(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.code)
        {
            pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.code);
            pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.code = NULL;
        }
        if(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.extra)
        {
            pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.extra);
            pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.extra = NULL;
        }
        _rCompile.strPattern =
                pCntx->tPPSitesTbl[_iSites].tHttpRgFld.strKey;
        _rCompile.pRegex     =
                &(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey);
        _result = tcRegexCompile(&_rCompile);
        if(ESUCCESS != _result)
            break;
        /* Reload httpsessionfield  */
        if(pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.code)
        {
            pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.code);
            pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.code = NULL;
        }
        if(pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.extra)
        {
            pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.extra);
            pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.extra = NULL;
        }
        _rCompile.strPattern =
                pCntx->tPPSitesTbl[_iSites].tHttpSessFld.strKey;
        _rCompile.pRegex     =
                &(pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey);
        _result = tcRegexCompile(&_rCompile);
        if(ESUCCESS != _result)
            break;
        _result = ESUCCESS;
    }

    return _result;
}

/***************************************************************************
 * function: _tcHttpInitPopulateSites
 *
 * description: Initialize control block based on information
 * from configuration file.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcHttpInitPopulateSites(
        tc_ldcfg_conf_t*            pLdCfg,
        tc_httpprc_thread_ctxt_t*    pCntx)
{
    tresult_t               _result;
    S32                     _iSites;
    U32                     _idx;
    tc_httpprc_httpsites_t*  _pSite;
    tc_ldcfg_httpsites_t*   _pLdCfgSite;
    tc_httpprc_httpuri_t*    _pUri;
    tc_ldcfg_httpuri_t*     _pLdCfgUri;
    tc_httpprc_httpsess_t*   _pSess;
    tc_ldcfg_httpsess_t*    _pLdCfgSess;

    CCURASSERT(pLdCfg);
    CCURASSERT(pCntx);

    do
    {
        _result = ESUCCESS;
        tcHttpInitSiteCleanupRes(pCntx);
        ccur_memclear(pCntx->tPPSitesTbl,
                sizeof(pCntx->tPPSitesTbl));
        /* Expand  loadable config and copy all the info */
        /* Starts from 1,
         * 0 suppose to be unknown sites resources */
        /* Services */
        pCntx->nPPSitesTbl = pLdCfg->nSites;
        for(_iSites=1;_iSites<pCntx->nPPSitesTbl;_iSites++)
        {
            pCntx->tPPSitesTbl[_iSites].nHttpURI =
                    pLdCfg->tSites[_iSites].nCfgArgURI;
            pCntx->tPPSitesTbl[_iSites].nHttpSess =
                    pLdCfg->tSites[_iSites].nCfgArgSess;
            ccur_strlcpy(
                    pCntx->tPPSitesTbl[_iSites].strPktPrcArgType,
                    pLdCfg->tSites[_iSites].strCfgArgType,
                    sizeof(pCntx->tPPSitesTbl[_iSites].strPktPrcArgType));
            ccur_strlcpy(
                    pCntx->tPPSitesTbl[_iSites].strPktPrcArgSites,
                    pLdCfg->tSites[_iSites].strCfgArgSites,
                    sizeof(pCntx->tPPSitesTbl[_iSites].strPktPrcArgSites));
            ccur_strlcpy(
                    pCntx->tPPSitesTbl[_iSites].strPktPrcArgOpt,
                    pLdCfg->tSites[_iSites].strCfgArgOpt,
                    sizeof(pCntx->tPPSitesTbl[_iSites].strPktPrcArgOpt));
            ccur_strlcpy(
                    pCntx->tPPSitesTbl[_iSites].tHttpRgFld.strKey,
                    pLdCfg->tSites[_iSites].strCfgArgHttpRangeField,
                    sizeof(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.strKey));
        }
        for(_iSites=1;
                _iSites<pCntx->nPPSitesTbl;_iSites++)
        {
            /* ---> Breakdown and populate http header range list */
            if('\0' != pLdCfg->tSites[_iSites].strCfgArgHttpHdrMatch[0])
            {
                _result = _tcPktProcHttpHdrMatchSplit(
                        pLdCfg->tSites[_iSites].strCfgArgHttpHdrMatch,
                        &(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[0]),
                        &(pCntx->tPPSitesTbl[_iSites].nHttpHdrMatch));
                if(ESUCCESS != _result)
                {
                    _result = EFAILURE;
                    break;
                }
            }
            /* ---> Breakdown and populate Host list */
            if('\0' != pLdCfg->tSites[_iSites].strCfgArgHost[0])
            {
                /* TODO: Change it to use _tcHttpInitValueSplit() */
                _result = _tcHttpInitHostSplit(
                        pLdCfg->tSites[_iSites].strCfgArgHost,
                        &(pCntx->tPPSitesTbl[_iSites]));
                if(ESUCCESS != _result)
                {
                    _result = EFAILURE;
                    break;
                }
            }
            /* ---> Breakdown and populate referrer list */
            if('\0' != pLdCfg->tSites[_iSites].strCfgArgReferrer[0])
            {
                _pSite = &(pCntx->tPPSitesTbl[_iSites]);
                _pLdCfgSite = &(pLdCfg->tSites[_iSites]);
                _result = _tcHttpInitValueSplit(
                        _pLdCfgSite->strCfgArgReferrer,
                        _pSite->tReferrer,
                        TRANSC_HTTPPRC_SITE_MAXREFTYPE_LST,
                        &(_pSite->nReferrer));
                if(ESUCCESS != _result)
                {
                    _result = EFAILURE;
                    break;
                }
            }
            /* ---> Breakdown sess string into list of values */
            for(_idx=0;
                    _idx<pCntx->tPPSitesTbl[_iSites].nHttpSess;_idx++)
            {
                _pSess = &(pCntx->tPPSitesTbl[_iSites].tHttpSess[_idx]);
                _pLdCfgSess = &(pLdCfg->tSites[_iSites].tCfgArgSess[_idx]);
                if('\0' != _pLdCfgSess->strCfgArgSessSig[0])
                {
                    _result = _tcHttpInitCkeyFixString(
                            _pSess->tSig.strKey,
                            sizeof(_pSess->tSig.strKey),
                            _pLdCfgSess->strCfgArgSessSig,
                            sizeof(_pLdCfgSess->strCfgArgSessSig));
                    if(ESUCCESS != _result)
                        break;
                }
                if('\0' != _pLdCfgSess->strCfgArgMatchSz[0])
                {
                    _pSess->nMatchSz =
                            strtol(_pLdCfgSess->strCfgArgMatchSz,
                                 (char **)NULL, 10);
                }
                if('\0' != _pLdCfgSess->strCfgArgCKeyId[0])
                {
                    _result = _tcHttpInitValueSplit(
                            _pLdCfgSess->strCfgArgCKeyId,
                            _pSess->tCKeyId,
                            TRANSC_HTTPPRC_URL_CKEYARGS_LST,
                            &(_pSess->nCKeyId));
                    if(ESUCCESS != _result)
                    {
                        _result = EFAILURE;
                        break;
                    }
                }
                if('\0' != _pLdCfgSess->strCfgArgCtxId[0])
                {
                    _result = _tcHttpInitValueSplit(
                            _pLdCfgSess->strCfgArgCtxId,
                            _pSess->tCtxId,
                            TRANSC_HTTPPRC_URL_CKEYARGS_LST,
                            &(_pSess->nCtxId));
                    if(ESUCCESS != _result)
                    {
                        _result = EFAILURE;
                        break;
                    }
                }
                if('\0' != _pLdCfgSess->strCfgArgCKeyRange[0])
                {
                    ccur_memclear(_pSess->tCKeyRange.strKey,
                                  sizeof(_pSess->tCKeyRange.strKey));
                    ccur_strlcpy(_pSess->tCKeyRange.strKey,
                            _pLdCfgSess->strCfgArgCKeyRange,
                            sizeof(_pSess->tCKeyRange.strKey));
                }
                if('\0' != _pLdCfgSess->strCfgArgCKeyMisc[0])
                {
                    _result = _tcHttpInitValueSplit(
                            _pLdCfgSess->strCfgArgCKeyMisc,
                            _pSess->tCkeyMisc,
                            TRANSC_HTTPPRC_URL_CKEYARGS_LST,
                            &(_pSess->nCKeyMisc));
                    if(ESUCCESS != _result)
                    {
                        _result = EFAILURE;
                        break;
                    }
                }
            }
            if(ESUCCESS != _result)
                break;
            /* ---> Breakdown url strings into list of values */
            for(_idx=0;
                    _idx<pCntx->tPPSitesTbl[_iSites].nHttpURI;_idx++)
            {
                _pUri       = &(pCntx->tPPSitesTbl[_iSites].tHttpURI[_idx]);
                _pLdCfgUri  = &(pLdCfg->tSites[_iSites].tCfgArgURI[_idx]);
                if('\0' != _pLdCfgUri->strCfgArgUriSig[0])
                {
                    _result = _tcHttpInitCkeyFixString(
                            _pUri->tSig.strKey,
                            sizeof(_pUri->tSig.strKey),
                            _pLdCfgUri->strCfgArgUriSig,
                            sizeof(_pLdCfgUri->strCfgArgUriSig));
                    if(ESUCCESS != _result)
                        break;
                }
                if('\0' != _pLdCfgUri->strCfgArgMatchSz[0])
                {
                    _pUri->nMatchSz =
                            strtol(_pLdCfgUri->strCfgArgMatchSz,
                                 (char **)NULL, 10);
                }
                if('\0' != _pLdCfgUri->strCfgArgCKeyId[0])
                {
                    _result = _tcHttpInitValueSplit(
                            _pLdCfgUri->strCfgArgCKeyId,
                            _pUri->tCKeyId,
                            TRANSC_HTTPPRC_URL_CKEYARGS_LST,
                            &(_pUri->nCKeyId));
                    if(ESUCCESS != _result)
                    {
                        _result = EFAILURE;
                        break;
                    }
                }
                if('\0' != _pLdCfgUri->strCfgArgCtxId[0])
                {
                    _result = _tcHttpInitValueSplit(
                            _pLdCfgUri->strCfgArgCtxId,
                            _pUri->tCtxId,
                            TRANSC_HTTPPRC_URL_CKEYARGS_LST,
                            &(_pUri->nCtxId));
                    if(ESUCCESS != _result)
                    {
                        _result = EFAILURE;
                        break;
                    }
                }
                if('\0' != _pLdCfgUri->strCfgArgCKeyRange[0])
                {
                    ccur_memclear(_pUri->tCKeyRange.strKey,
                                  sizeof(_pUri->tCKeyRange.strKey));
                    ccur_strlcpy(_pUri->tCKeyRange.strKey,
                            _pLdCfgUri->strCfgArgCKeyRange,
                            sizeof(_pUri->tCKeyRange.strKey));
                }
                if('\0' != _pLdCfgUri->strCfgArgCKeyMisc[0])
                {
                    _result = _tcHttpInitValueSplit(
                            _pLdCfgUri->strCfgArgCKeyMisc,
                            _pUri->tCkeyMisc,
                            TRANSC_HTTPPRC_URL_CKEYARGS_LST,
                            &(_pUri->nCKeyMisc));
                    if(ESUCCESS != _result)
                    {
                        _result = EFAILURE;
                        break;
                    }
                }
            }
            if(ESUCCESS != _result)
                break;
        }
        if(ESUCCESS != _result)
            break;
        _result = _tcHttpInitUriSitesTbl(pCntx);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                   pCntx->pQHttpProcToBkgrnd,
                   evLogLvlFatal,
                   &(pCntx->tLogDescSys),
                   "unable to load sites/service");
            break;
        }
        _result = _tcHttpInitSessRegexTbl(pCntx);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                   pCntx->pQHttpProcToBkgrnd,
                   evLogLvlFatal,
                   &(pCntx->tLogDescSys),
                   "unable to load content session");
            break;
        }
        _result = _tcHttpInitUriRegexTbl(pCntx);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                   pCntx->pQHttpProcToBkgrnd,
                   evLogLvlFatal,
                   &(pCntx->tLogDescSys),
                   "unable to load content URI list");
            break;
        }
        _tcHttpInitSessUriRelantionships(pCntx);
    }while(FALSE);

    return _result;
}

/**************************PROTECTED******************************************/
/***************************************************************************
 * function: _tcHttpInitSiteCleanupRes
 *
 * description: Cleans up the service resources.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcHttpInitSiteCleanupRes(
        tc_httpprc_thread_ctxt_t *     pCntx)
{
    S32                     _iSites;
    U32                     _idxa;
    U32                     _idx;
    tc_httpprc_httpuri_t*    _pUri;
    tc_httpprc_httpsess_t*   _pSess;

    CCURASSERT(pCntx);

    /* Cleanup all sites regex */
    for(_iSites=1;
            _iSites<pCntx->nPPSitesTbl;_iSites++)
    {
        /* Free Host Signature */
        for(_idx=0;
                _idx<pCntx->tPPSitesTbl[_iSites].nHost;_idx++)
        {
            if(_idx >= TRANSC_HTTPPRC_SITE_MAXHOSTYPE_LST)
                break;
            if(pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.code)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.code);
                pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.code = NULL;
            }
            if(pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.extra)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.extra);
                pCntx->tPPSitesTbl[_iSites].tHost[_idx].tReHost.extra = NULL;
            }
        }
        /* Free referral Signature */
        for(_idx=0;
                _idx<pCntx->tPPSitesTbl[_iSites].nReferrer;_idx++)
        {
            if(_idx >= TRANSC_HTTPPRC_SITE_MAXREFTYPE_LST)
                break;
            if(pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.code)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.code);
                pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.code = NULL;
            }
            if(pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.extra)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.extra);
                pCntx->tPPSitesTbl[_iSites].tReferrer[_idx].tReCkey.extra = NULL;
            }
        }
        /* Free http header Signature */
        for(_idx=0;
                _idx<pCntx->tPPSitesTbl[_iSites].nHttpHdrMatch;_idx++)
        {
            if(_idx >= TRANSC_HTTPPRC_SITE_MAXHTTPHDRMATCH_LST)
                break;
            if(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.code)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.code);
                pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.code = NULL;
            }
            if(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.extra)
            {
                pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.extra);
                pCntx->tPPSitesTbl[_iSites].tHttpHdrMatch[_idx].tFieldValue.tReCkey.extra = NULL;
            }
        }
        /* Free httprangefield */
        if(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.code)
        {
           pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.code);
           pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.code = NULL;
        }
        if(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.extra)
        {
           pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.extra);
           pCntx->tPPSitesTbl[_iSites].tHttpRgFld.tReCkey.extra = NULL;
        }
        /* Free httpsessionfield */
        if(pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.code)
        {
           pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.code);
           pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.code = NULL;
        }
        if(pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.extra)
        {
           pcre_free(pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.extra);
           pCntx->tPPSitesTbl[_iSites].tHttpSessFld.tReCkey.extra = NULL;
        }
        /* Cleanup Sessions */
        for(_idxa=0;
                _idxa<pCntx->tPPSitesTbl[_iSites].nHttpSess;_idxa++)
        {
            /* Free URL Signature */
            _pSess = &(pCntx->tPPSitesTbl[_iSites].tHttpSess[_idxa]);
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
            /* Free cache key Range Signature */
            if(_pSess->tCKeyRange.tReCkey.code)
            {
                pcre_free(_pSess->tCKeyRange.tReCkey.code);
                _pSess->tCKeyRange.tReCkey.code = NULL;
            }
            if(_pSess->tCKeyRange.tReCkey.extra)
            {
                pcre_free(_pSess->tCKeyRange.tReCkey.extra);
                _pSess->tCKeyRange.tReCkey.extra = NULL;
            }
            /* Free Cache Key Context Signature */
            for(_idx=0;
                    _idx<_pSess->nCtxId;_idx++)
            {
                if(_pSess->tCtxId[_idx].tReCkey.code)
                {
                    pcre_free(_pSess->tCtxId[_idx].tReCkey.code);
                    _pSess->tCtxId[_idx].tReCkey.code = NULL;
                }
                if(_pSess->tCtxId[_idx].tReCkey.extra)
                {
                    pcre_free(_pSess->tCtxId[_idx].tReCkey.extra);
                    _pSess->tCtxId[_idx].tReCkey.extra = NULL;
                }
            }
            /* Free Cache Key Id Signature */
            for(_idx=0;
                    _idx<_pSess->nCKeyId;_idx++)
            {
                if(_pSess->tCKeyId[_idx].tReCkey.code)
                {
                    pcre_free(_pSess->tCKeyId[_idx].tReCkey.code);
                    _pSess->tCKeyId[_idx].tReCkey.code = NULL;
                }
                if(_pSess->tCKeyId[_idx].tReCkey.extra)
                {
                    pcre_free(_pSess->tCKeyId[_idx].tReCkey.extra);
                    _pSess->tCKeyId[_idx].tReCkey.extra = NULL;
                }
            }
            /* Free Cache Key Misc Signature */
            for(_idx=0;
                    _idx<_pSess->nCKeyMisc;_idx++)
            {
                if(_pSess->tCkeyMisc[_idx].tReCkey.code)
                {
                    pcre_free(_pSess->tCkeyMisc[_idx].tReCkey.code);
                    _pSess->tCkeyMisc[_idx].tReCkey.code = NULL;
                }
                if(_pSess->tCkeyMisc[_idx].tReCkey.extra)
                {
                    pcre_free(_pSess->tCkeyMisc[_idx].tReCkey.extra);
                    _pSess->tCkeyMisc[_idx].tReCkey.extra = NULL;
                }
            }
        }
        /* Clean up URL */
        for(_idxa=0;
                _idxa<pCntx->tPPSitesTbl[_iSites].nHttpURI;_idxa++)
        {
            /* Free URL Signature */
            _pUri = &(pCntx->tPPSitesTbl[_iSites].tHttpURI[_idxa]);
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
            /* Free cache key Range Signature */
            if(_pUri->tCKeyRange.tReCkey.code)
            {
                pcre_free(_pUri->tCKeyRange.tReCkey.code);
                _pUri->tCKeyRange.tReCkey.code = NULL;
            }
            if(_pUri->tCKeyRange.tReCkey.extra)
            {
                pcre_free(_pUri->tCKeyRange.tReCkey.extra);
                _pUri->tCKeyRange.tReCkey.extra = NULL;
            }
            /* Free Cache Key Context Signature */
            for(_idx=0;
                    _idx<_pUri->nCtxId;_idx++)
            {
                if(_pUri->tCtxId[_idx].tReCkey.code)
                {
                    pcre_free(_pUri->tCtxId[_idx].tReCkey.code);
                    _pUri->tCtxId[_idx].tReCkey.code = NULL;
                }
                if(_pUri->tCtxId[_idx].tReCkey.extra)
                {
                    pcre_free(_pUri->tCtxId[_idx].tReCkey.extra);
                    _pUri->tCtxId[_idx].tReCkey.extra = NULL;
                }
            }
            /* Free Cache Key Id Signature */
            for(_idx=0;
                    _idx<_pUri->nCKeyId;_idx++)
            {
                if(_pUri->tCKeyId[_idx].tReCkey.code)
                {
                    pcre_free(_pUri->tCKeyId[_idx].tReCkey.code);
                    _pUri->tCKeyId[_idx].tReCkey.code = NULL;
                }
                if(_pUri->tCKeyId[_idx].tReCkey.extra)
                {
                    pcre_free(_pUri->tCKeyId[_idx].tReCkey.extra);
                    _pUri->tCKeyId[_idx].tReCkey.extra = NULL;
                }
            }
            /* Free Cache Key Misc Signature */
            for(_idx=0;
                    _idx<_pUri->nCKeyMisc;_idx++)
            {
                if(_pUri->tCkeyMisc[_idx].tReCkey.code)
                {
                    pcre_free(_pUri->tCkeyMisc[_idx].tReCkey.code);
                    _pUri->tCkeyMisc[_idx].tReCkey.code = NULL;
                }
                if(_pUri->tCkeyMisc[_idx].tReCkey.extra)
                {
                    pcre_free(_pUri->tCkeyMisc[_idx].tReCkey.extra);
                    _pUri->tCkeyMisc[_idx].tReCkey.extra = NULL;
                }
            }
        }
    }
}

/***************************************************************************
 * function: tcHttpInitSiteGetRes
 *
 * description: get resource handle for particular service or site.
 ***************************************************************************/
CCUR_PROTECTED(tc_httpprc_stats_t*)
tcHttpInitSiteGetRes(
        tc_httpprc_thread_ctxt_t *       pCntx,
        hlpr_httpsite_hndl               eSiteHost)
{
    tc_httpprc_stats_t* _pHttpSiteRes = NULL;
    if(eSiteHost > 0 &&
       eSiteHost < TRANSC_LDCFG_SITE_MAXSITES_LST)
        _pHttpSiteRes = &(pCntx->tGenSiteTbl[eSiteHost]);
    return(_pHttpSiteRes);
}

/***************************************************************************
 * function: tcHttpInitLoadableRes
 *
 * description: Init loadable resources.
 * This function will be used to load config values at runtime.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcHttpInitLoadableRes(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_ldcfg_conf_t*             pLdCfg)
{
    tresult_t                   _sts;
    tresult_t                   _result;
    CHAR*                       _pStr;
    U32                         _nBufLen;
    CHAR                        _strBuff[512];

    CCURASSERT(pCntx);
    CCURASSERT(pLdCfg);

    do
    {
        _result = EFAILURE;
        pCntx->bTrSts = tcTrStsDown;
        evLogTrace(
              pCntx->pQHttpProcToBkgrnd,
              evLogLvlWarn,
              &(pCntx->tLogDescSys),
              "Warning, http proc is down, reloading config!");
        ccur_strlcpy(
                pCntx->strHttpPrcRedirTarget,
                pLdCfg->strCmdArgRedirTarget,
                sizeof(pCntx->strHttpPrcRedirTarget));
        _sts = _tcHttpInitPopulateSites(
                                        pLdCfg,pCntx);
        if(ESUCCESS != _sts)
        {
            evLogTrace(
                   pCntx->pQHttpProcToBkgrnd,
                   evLogLvlFatal,
                   &(pCntx->tLogDescSys),
                   "failure to load data to pkt proc data structure\n");
            break;
        }
        if(!ccur_strcasecmp(pLdCfg->strCmdArgSimBwSimMode,"true"))
            pCntx->bBwSimMode = TRUE;
        else
            pCntx->bBwSimMode = FALSE;
        /* Adds "http:// if not specified */
        if(strncmp(pCntx->strHttpPrcRedirTarget,"http://",sizeof("http://")-1))
        {
            _pStr = pCntx->strHttpPrcRedirTarget;
            ccur_strlcpy(_strBuff,pCntx->strHttpPrcRedirTarget,sizeof(_strBuff));
            strncpy(pCntx->strHttpPrcRedirTarget,"http://",sizeof("http://"));
            _pStr += sizeof("http://")-1;
            _nBufLen = sizeof(pCntx->strHttpPrcRedirTarget)-sizeof("http://");
            ccur_strlcpy(_pStr,_strBuff,_nBufLen);
            evLogTrace(
                    pCntx->pQHttpProcToBkgrnd,
                    evLogLvlDebug,
                    &(pCntx->tLogDescSys),
                    "Redirect Address:%s",
                    pCntx->strHttpPrcRedirTarget);
        }
        evLogTrace(
              pCntx->pQHttpProcToBkgrnd,
              evLogLvlWarn,
              &(pCntx->tLogDescSys),
              "Warning, http proc is UP, Finished reloading config!");
        pCntx->bTrSts = tcTrStsUp;
        _result = ESUCCESS;
    }while(FALSE);

    if(ESUCCESS != _result)
        tcHttpInitSiteCleanupRes(pCntx);

    /* Set Component status */
    tcShProtectedDSetCompSts(tcTRCompTypeHttpPrc,pCntx->bTrSts);

    return (pCntx->bTrSts ? ESUCCESS : EFAILURE);
}
