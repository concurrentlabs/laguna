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

/**************** PRIVATE Functions **********************/

/***************************************************************************
 * function: _tcHttpParseHttpHdrCk
 *
 * description: Search for matching header value based on specified field
 ***************************************************************************/
CCUR_PRIVATE(tc_httpparse_hdrmatch_e)
_tcHttpParseHttpHdrCk(
        tc_httpprc_thread_ctxt_t*   pCntx,
        hlpr_httpsite_hndl          eSiteHost,
        U16                         idx,
        const CHAR*                 buf,
        size_t                      len)
{


    tc_regex_t*             _pRe;
    I32                     _rc;
    tc_httpparse_hdrmatch_e _eHttpHdrMatchType;
    I32                     _subStrVec[TRANSC_HTTPPRC_REGEXSUBEX_NUM];

    CCURASSERT(pCntx);
    CCURASSERT(buf);

    _eHttpHdrMatchType = tcHttpParseHdrMatchIgnore;
    /* Check valid http header match, exit if match is found. */
    _pRe =
        &(pCntx->tPPSitesTbl[eSiteHost].
                tHttpHdrMatch[idx].tFieldValue.tReCkey);
    if(_pRe->code)
    {
        _rc = tcRegexExec(
                _pRe,
                buf,len,
                _subStrVec,TRANSC_HTTPPRC_REGEXSUBEX_NUM);
        if(_rc >= 0)
            _eHttpHdrMatchType = tcHttpParseHdrMatchCheck;
    }

    return _eHttpHdrMatchType;
}

/***************************************************************************
 * function: _tcHttpParseGetHostType
 *
 * description: Search for matching hostname based on value specified within
 * config file.
 ***************************************************************************/
CCUR_PRIVATE(hlpr_httpsite_hndl)
_tcHttpParseGetHostType(
        tc_httpprc_thread_ctxt_t*    pCntx,
        hlpr_httpsite_hndl          ePossibleSiteHost,
        const CHAR*                 buf,
        size_t                      len)
{

    U32                     _iHost;
    tc_regex_t*             _pRe;
    I32                     _rc;
    hlpr_httpsite_hndl      _eSiteHost;
    I32                     _subStrVec[TRANSC_HTTPPRC_REGEXSUBEX_NUM];

    CCURASSERT(pCntx);
    CCURASSERT(buf);

    _eSiteHost  = tcHttpParseSiteTypeUnknown;
    for(_iHost=0;
            _iHost<pCntx->tPPSitesTbl[ePossibleSiteHost].nHost;_iHost++)
    {
        /* Check valid Host,
         * OR operation, exit if match is found. */
        _pRe =
            &(pCntx->tPPSitesTbl[ePossibleSiteHost].tHost[_iHost].tReHost);
        if(_pRe->code)
        {
            _rc = tcRegexExec(
                    _pRe,
                    buf,len,
                    _subStrVec,TRANSC_HTTPPRC_REGEXSUBEX_NUM);
            if(_rc >= 0)
            {
                _eSiteHost = ePossibleSiteHost;
                break;
            }
        }
    }
    return _eSiteHost;
}

/***************************************************************************
 * function: _tcHttpParseGetHttpReferer
 *
 * description: Search for matching referer based on value specified within
 * config file.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHttpParseGetHttpReferer(
        tc_httpparse_calldef_t*     pHttpMsg,
        const CHAR*                 buf,
        size_t                      len)
{
    tc_httpprc_thread_ctxt_t*    _pCntx;
    U32                         _iRef;
    tc_regex_t*                 _pRe;
    I32                         _rc;
    U16                         _iVec;
    I32                         _ExecRet;
    I32                         _subStrVec[TRANSC_HTTPPRC_REGEXSUBEX_NUM];
    CHAR                        _subStrMatchStr[TRANSC_HTTPPRC_REGEXSUBEX_BUFFSZ];

    CCURASSERT(pHttpMsg);
    CCURASSERT(buf);

    _pCntx      = pHttpMsg->pCntx;
    for(_iRef=0;
            _iRef<_pCntx->tPPSitesTbl[pHttpMsg->ePossibleSiteHost].nReferrer;_iRef++)
    {
        /* Check valid referer,
         * OR operation, exit if match is found. */
        _pRe =
            &(_pCntx->tPPSitesTbl[pHttpMsg->ePossibleSiteHost].tReferrer[_iRef].tReCkey);
        if(_pRe->code)
        {
            _rc = tcRegexExec(
                    _pRe,
                    buf,len,
                    _subStrVec,TRANSC_HTTPPRC_REGEXSUBEX_NUM);
            if(_rc >= 0)
            {
                _subStrMatchStr[0] = '\0';
                for(_iVec=1; _iVec<_rc; _iVec++)
                {
                    _ExecRet = tcRegexCopySubstring(buf,
                            _subStrVec, _rc, _iVec,
                            _subStrMatchStr,
                            sizeof(_subStrMatchStr));
                    if(_ExecRet <= 0)
                        break;
                    ccur_strlcpy(pHttpMsg->tInjMsg.strCId,
                            _subStrMatchStr,
                            sizeof(pHttpMsg->tInjMsg.strCId));
                }
                break;
            }
        }
    }
}

/***************************************************************************
 * function: _tcHttpParseGetHttpOrigin
 *
 * description: Pull origin information from HTTP header.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHttpParseGetHttpOrigin(
        tc_httpparse_calldef_t*     pHttpMsg,
        const CHAR*                 buf,
        size_t                      len)
{
    if(sizeof(pHttpMsg->tInjMsg.strCOrigin)-1 > len)
    {
        strncpy(pHttpMsg->tInjMsg.strCOrigin,buf,len);
        pHttpMsg->tInjMsg.strCOrigin[len] = '\0';
    }
    else
        pHttpMsg->bParseErr = TRUE;
}

#if TRANSC_TCSIM
CCUR_PRIVATE(void)
_tcHttpParseGetHttpHdrSimUAgent(
        tc_httpparse_calldef_t*     pHttpMsg,
        const CHAR*                 buf,
        size_t                      len)
{
    if(sizeof(pHttpMsg->tInjMsg.strUAgent)-1 > len)
    {
        strncpy(pHttpMsg->tInjMsg.strUAgent,buf,len);
        pHttpMsg->tInjMsg.strUAgent[len] = '\0';
    }
    else
        pHttpMsg->bParseErr = TRUE;
}

CCUR_PRIVATE(void)
_tcHttpParseGetHttpHdrSimRange(
        tc_httpparse_calldef_t*     pHttpMsg,
        const CHAR*                 buf,
        size_t                      len)
{
    if(sizeof(pHttpMsg->tInjMsg.strCSimRange)-1 > len)
    {
        strncpy(pHttpMsg->tInjMsg.strCSimRange,buf,len);
        pHttpMsg->tInjMsg.strCSimRange[len] = '\0';
    }
    else
        pHttpMsg->bParseErr = TRUE;
}

#endif /* TRANSC_TCSIM */

CCUR_PRIVATE(void)
_tcHttpParseGetHttpHdrRange(
        tc_httpparse_calldef_t*     pHttpMsg,
        const CHAR*                 buf,
        size_t                      len)
{
    if(sizeof(pHttpMsg->tInjMsg.strCRange)-1 > len)
    {
        strncpy(pHttpMsg->tInjMsg.strCRange,buf,len);
        pHttpMsg->tInjMsg.strCRange[len] = '\0';
    }
    else
        pHttpMsg->bParseErr = TRUE;
}

/**************** PROTECTED Functions **********************/
/***************************************************************************
 * function: tcHttpParserGetCacheKey
 *
 * description: Generic get cache key function that implements regex
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcHttpParserGetCacheKey(
        tc_httpprc_thread_ctxt_t*    pCntx,
        CHAR*                       pStrCid,
        U32                         nStrCidSz,
        tc_httpprc_ckeyq_t*          pKeyId,
        U16                         nKeyId,
        tc_httpparse_calldef_t*     pHttpMsg,
        CHAR*                       delim)
{
    I32                     _rc;
    I32                     _ExecRet;
    U32                     _ickey;
    BOOL                    _bIsIdSet;
    tc_regex_t*             _pRe;
    U16                     _iVec;
    CHAR*                   _pPUrl;
    S32                     _nPUrlLen;
    U16                     _nSkipLen;
    I32                     _subStrVec[TRANSC_HTTPPRC_REGEXSUBEX_NUM];
    CHAR                    _subStrMatchStr[TRANSC_HTTPPRC_REGEXSUBEX_BUFFSZ];

    CCURASSERT(pCntx);
    CCURASSERT(pHttpMsg);

    _bIsIdSet   = FALSE;
    _pPUrl      = NULL;
    _nPUrlLen   = 0;
    if(TCUTIL_IS_URLABSPATH(pHttpMsg->tInjMsg.pUrl))
    {
        _pPUrl      = pHttpMsg->tInjMsg.pUrl;
        _nPUrlLen   = pHttpMsg->tInjMsg.nUrlLen;
    }
    else
    {
        /* Skips http://.../ if exists in case of web proxy */
        if(TCUTIL_IS_URLHTTPSTRING(pHttpMsg->tInjMsg.pUrl))
        {
            _nSkipLen = tcUtilSkipGetHttpStringLen(
                        pHttpMsg->tInjMsg.pUrl,pHttpMsg->tInjMsg.nUrlLen);
            if(_nSkipLen)
            {
                _pPUrl     = pHttpMsg->tInjMsg.pUrl+_nSkipLen;
                _nPUrlLen  = pHttpMsg->tInjMsg.nUrlLen-_nSkipLen;

            }
        }
    }
    if(_pPUrl)
    {
        pStrCid[0] = '\0';
        for(_ickey=0;
                _ickey<nKeyId;_ickey++)
        {
            _pRe =
                    &(pKeyId[_ickey].tReCkey);
            if(NULL == _pRe->code)
                break;
            _rc = tcRegexExec(
                    _pRe,_pPUrl,_nPUrlLen,
                    _subStrVec,TRANSC_HTTPPRC_REGEXSUBEX_NUM);
            if(_rc <= 0)
            {
                switch(_rc)
                {
                    case PCRE_ERROR_NOMATCH      :
                        evLogTrace(pCntx->pQHttpProcToBkgrnd,
                                evLogLvlDebug,&(pCntx->tLogDescSys),
                                "Regex String did not match the pattern %s",
                                pKeyId[_ickey].strKey);
                        break;
                    case PCRE_ERROR_NULL         :
                        evLogTrace(pCntx->pQHttpProcToBkgrnd,
                                evLogLvlDebug,&(pCntx->tLogDescSys),
                                "Regex Something was null %s",
                                pKeyId[_ickey].strKey);
                        break;
                    case PCRE_ERROR_BADOPTION    :
                        evLogTrace(pCntx->pQHttpProcToBkgrnd,
                                evLogLvlDebug,&(pCntx->tLogDescSys),
                                "Regex A bad option was passed %s",
                                pKeyId[_ickey].strKey);
                        break;
                    case PCRE_ERROR_BADMAGIC     :
                        evLogTrace(pCntx->pQHttpProcToBkgrnd,
                                evLogLvlDebug,&(pCntx->tLogDescSys),
                                "Regex Magic number bad (compiled re corrupt?) %s",
                                pKeyId[_ickey].strKey);
                        break;
                    case PCRE_ERROR_UNKNOWN_NODE :
                        evLogTrace(pCntx->pQHttpProcToBkgrnd,
                                evLogLvlDebug,&(pCntx->tLogDescSys),
                                "Regex Something wrong in the compiled re %s",
                                pKeyId[_ickey].strKey);
                        break;
                    case PCRE_ERROR_NOMEMORY     :
                        evLogTrace(pCntx->pQHttpProcToBkgrnd,
                                evLogLvlDebug,&(pCntx->tLogDescSys),
                                "Regex Ran out of memory %s",
                                pKeyId[_ickey].strKey);
                        break;
                    default                      :
                        evLogTrace(pCntx->pQHttpProcToBkgrnd,
                                evLogLvlDebug,&(pCntx->tLogDescSys),
                                "Unknown error %s",
                                pKeyId[_ickey].strKey);
                        break;
                } /* end switch */
            }
            else
            {
                _subStrMatchStr[0] = '\0';
                for(_iVec=1; _iVec<_rc; _iVec++)
                {
                    _ExecRet = tcRegexCopySubstring(
                            _pPUrl,
                            _subStrVec, _rc, _iVec,
                            _subStrMatchStr,
                            sizeof(_subStrMatchStr));
                    if(_ExecRet <= 0)
                        break;
                    if('\0' != pStrCid[0])
                    {
                        ccur_strlcat(pStrCid,
                                delim,
                                nStrCidSz);
                        ccur_strlcat(pStrCid,
                                _subStrMatchStr,
                                nStrCidSz);
                        _bIsIdSet = TRUE;
                    }
                    else
                    {
                        ccur_strlcpy(pStrCid,
                            _subStrMatchStr,
                            nStrCidSz);
                        _bIsIdSet = TRUE;
                    }
                }
                if(_rc <= 0)
                {
                    break;
                }
            }
        }
    }

    return (_bIsIdSet);
}

/**************************************************************************
 *
 * URL keys
 *
 **************************************************************************/

/***************************************************************************
 * function: tcHttpParserGetUrlCacheKey
 *
 * description: Get URL cache key id and cache key misc value.
 * Need to construct cache key, that consists of atleast
 * video id + range + misc (optional). No Video id or range or both is not
 * acceptable.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcHttpParserGetAllUrlCacheKeyId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*             pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg)
{
    tresult_t               _result;
    tc_httpprc_ckeyq_t*      _pKeyId;
    U16                     _nKeyId;

    CCURASSERT(pCntx);
    CCURASSERT(pHttpMsg);

    do
    {
        _result     = EFAILURE;
        if(tcHttpParseSvcTypeSess == pSvcType->svcType )
            break;
        if('\0' == pHttpMsg->tInjMsg.strCId[0])
        {
            _pKeyId =
                    &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].tCKeyId[0]);
            _nKeyId = pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].nCKeyId;
            tcHttpParserGetCacheKey(
                    pCntx,
                    pHttpMsg->tInjMsg.strCId,
                    sizeof(pHttpMsg->tInjMsg.strCId),
                    _pKeyId,
                    _nKeyId,
                    pHttpMsg,
                    "-"
                    );
        }
        if('\0' == pHttpMsg->tInjMsg.strCId[0])
            break;
        if('\0' == pHttpMsg->tInjMsg.strCRange[0])
        {
            _pKeyId =
                    &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].tCKeyRange);
            if(_pKeyId->tReCkey.code)
                _nKeyId = 1;
            else
                _nKeyId = 0;
            tcHttpParserGetCacheKey(
                    pCntx,
                    pHttpMsg->tInjMsg.strCRange,
                    sizeof(pHttpMsg->tInjMsg.strCRange),
                    _pKeyId,
                    _nKeyId,
                    pHttpMsg,
                    ","
                    );
        }
        _pKeyId =
                &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].tCkeyMisc[0]);
        _nKeyId = pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].nCKeyMisc;
        tcHttpParserGetCacheKey(
                pCntx,
               pHttpMsg->tInjMsg.strCMisc,
               sizeof(pHttpMsg->tInjMsg.strCMisc),
               _pKeyId,
               _nKeyId,
               pHttpMsg,
               "-"
               );
        _result     = ESUCCESS;
    }while(FALSE);

    return (_result);
}

/***************************************************************************
 * function: tcHttpParserGetUrlCacheKeyId
 *
 * description: Get URL cache key id value
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcHttpParserGetUrlCacheKeyId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg)
{
    BOOL                    _bIsIdSet;
    tc_httpprc_ckeyq_t*      _pKeyId;
    U16                     _nKeyId;

    CCURASSERT(pCntx);
    CCURASSERT(pHttpMsg);

    _bIsIdSet = FALSE;
    if(tcHttpParseSvcTypeUrl == pSvcType->svcType )
    {
		_pKeyId =
		        &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].tCKeyId[0]);
		_nKeyId = pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].nCKeyId;
		_bIsIdSet = tcHttpParserGetCacheKey(
		        pCntx,
		        pHttpMsg->tInjMsg.strCId,
		        sizeof(pHttpMsg->tInjMsg.strCId),
		        _pKeyId,
		        _nKeyId,
		        pHttpMsg,
		        "-"
		        );
    }
    return (_bIsIdSet);
}

/***************************************************************************
 * function: tcHttpParserGetUrlCacheKeyRange
 *
 * description: Get URL cache key Range value
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcHttpParserGetUrlCacheKeyRange(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*             pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg)
{
    BOOL                    _bIsIdSet;
    tc_httpprc_ckeyq_t*      _pKeyId;
    U16                     _nKeyId;

    CCURASSERT(pCntx);
    CCURASSERT(pHttpMsg);

    _bIsIdSet = FALSE;
    if(tcHttpParseSvcTypeUrl == pSvcType->svcType )
    {
		_pKeyId =
		        &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].tCKeyRange);
		if(_pKeyId->tReCkey.code)
		    _nKeyId = 1;
		else
		    _nKeyId = 0;
		_bIsIdSet = tcHttpParserGetCacheKey(
		        pCntx,
		        pHttpMsg->tInjMsg.strCRange,
		        sizeof(pHttpMsg->tInjMsg.strCRange),
		        _pKeyId,
		        _nKeyId,
		        pHttpMsg,
		        ","
		        );
    }
    return (_bIsIdSet);
}

/***************************************************************************
 * function: tcHttpParserGetUrlCacheKeyMisc
 *
 * description: Get URL cache key misc value
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcHttpParserGetUrlCacheKeyMisc(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*             pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg)
{
    BOOL                    _bIsIdSet;
    tc_httpprc_ckeyq_t*      _pKeyId;
    U16                     _nKeyId;

    CCURASSERT(pCntx);
    CCURASSERT(pHttpMsg);

    _bIsIdSet = FALSE;
    if(tcHttpParseSvcTypeUrl == pSvcType->svcType )
    {
		_pKeyId =
		        &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].tCkeyMisc[0]);
		_nKeyId = pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].nCKeyMisc;
		_bIsIdSet = tcHttpParserGetCacheKey(
		        pCntx,
		        pHttpMsg->tInjMsg.strCMisc,
		        sizeof(pHttpMsg->tInjMsg.strCMisc),
		        _pKeyId,
		        _nKeyId,
		        pHttpMsg,
		        "-"
		        );
    }
    return (_bIsIdSet);
}

/***************************************************************************
 * function: tcHttpParserGetUrlHashKey
 *
 * description: Get URL hash key id value by hashing session cache key id.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcHttpParserGetUrlHashKeyId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        U32*                        pHash,
        tc_g_svcdesc_t*             pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg)
{
    tresult_t           _result;
    BOOL                _bIsIdSet;
    tc_httpprc_ckeyq_t*  _pKeyId;
    U16                 _nKeyId;
    CHAR                _strCKey[512];

    _result = EFAILURE;
    if(tcHttpParseSvcTypeUrl == pSvcType->svcType )
    {
		_pKeyId =
		        &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].tCKeyId[0]);
		_nKeyId = pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].nCKeyId;
		_bIsIdSet = tcHttpParserGetCacheKey(
		        pCntx,
		        _strCKey,
		        sizeof(_strCKey),
		        _pKeyId,
		        _nKeyId,
		        pHttpMsg,
		        "-");
		if(_bIsIdSet)
		{
		    *pHash =
		            tcUtilHashBytes(
		                    (U8*)_strCKey, strlen(_strCKey));
		    _result = ESUCCESS;
		}
	}

    return _result;
}


/***************************************************************************
 * function: tcHttpParserGetUrlHashCntxId
 *
 * description: Get URL hash key id value by hashing session context id.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcHttpParserGetUrlHashCntxId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        U32*                        pHash,
        tc_g_svcdesc_t*             pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg)
{
    tresult_t           _result;
    BOOL                _bIsIdSet;
    tc_httpprc_ckeyq_t*  _pKeyId;
    U16                 _nKeyId;
    CHAR                _strCKey[512];

    _result = EFAILURE;
    if(tcHttpParseSvcTypeUrl == pSvcType->svcType )
    {
		_pKeyId =
		        &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].tCtxId[0]);
		_nKeyId = pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpURI[pSvcType->nSvcTypeIdx].nCtxId;
		_bIsIdSet = tcHttpParserGetCacheKey(
		        pCntx,
		        _strCKey,
		        sizeof(_strCKey),
		        _pKeyId,
		        _nKeyId,
		        pHttpMsg,
		        "-");
		if(_bIsIdSet)
		{
		    *pHash =
		            tcUtilHashBytes(
		                    (U8*)_strCKey, strlen(_strCKey));
		    _result = ESUCCESS;
		}
	}

    return _result;
}


/**************************************************************************
 *
 * Sess keys
 *
 **************************************************************************/

/***************************************************************************
 * function: tcHttpParserGetSessCacheKey
 *
 * description: Get session cache key id and cache key misc value
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcHttpParserGetAllSessCacheKeyId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*             pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg,
        tc_httpparse_sess_t*        pSymStrm)
{
    tresult_t               _result;
    tc_httpprc_ckeyq_t*      _pKeyId;
    U16                     _nKeyId;

    CCURASSERT(pCntx);
    CCURASSERT(pHttpMsg);

    do
    {
        if(tcHttpParseSvcTypeUrl == pSvcType->svcType )
        {
            _result = EFAILURE;
            break;
        }
        _result     = ESUCCESS;
        if('\0' == pHttpMsg->tInjMsg.strCId[0])
        {
            _pKeyId =
                    &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpSess[pSvcType->nSvcTypeIdx].tCKeyId[0]);
            _nKeyId = pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpSess[pSvcType->nSvcTypeIdx].nCKeyId;
            tcHttpParserGetCacheKey(
                    pCntx,
                    pSymStrm->tSessData.strCId,
                    sizeof(pSymStrm->tSessData.strCId),
                    _pKeyId,
                    _nKeyId,
                    pHttpMsg,
                    "-"
                    );
        }
        else
        {
            ccur_strlcpy(pSymStrm->tSessData.strCId,
                    pHttpMsg->tInjMsg.strCId,
                    sizeof(pSymStrm->tSessData.strCId));
        }
        if('\0' == pSymStrm->tSessData.strCId[0])
        {
            _result     = EIGNORE;
            break;
        }
        if('\0' == pHttpMsg->tInjMsg.strCRange[0])
        {
            _pKeyId =
                    &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpSess[pSvcType->nSvcTypeIdx].tCKeyRange);
            if(_pKeyId->tReCkey.code)
                _nKeyId = 1;
            else
                _nKeyId = 0;
            tcHttpParserGetCacheKey(
                    pCntx,
                    pSymStrm->tSessData.strCRange,
                    sizeof(pSymStrm->tSessData.strCRange),
                    _pKeyId,
                    _nKeyId,
                    pHttpMsg,
                    ","
                    );
        }
        else
        {
            ccur_strlcpy(pSymStrm->tSessData.strCRange,
                    pHttpMsg->tInjMsg.strCRange,
                    sizeof(pSymStrm->tSessData.strCRange));
        }
        _pKeyId =
                &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpSess[pSvcType->nSvcTypeIdx].tCkeyMisc[0]);
        _nKeyId = pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpSess[pSvcType->nSvcTypeIdx].nCKeyMisc;
        tcHttpParserGetCacheKey(
                pCntx,
                pSymStrm->tSessData.strCMisc,
               sizeof(pSymStrm->tSessData.strCMisc),
               _pKeyId,
               _nKeyId,
               pHttpMsg,
               "-"
               );
        _result     = ESUCCESS;
    }while(FALSE);

    return (_result);
}

/***************************************************************************
 * function: tcHttpParserGetSessHashKey
 *
 * description: Get session hash key id value by hashing session context id.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcHttpParserGetSessHashCntxId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        U32*                        pHash,
        tc_g_svcdesc_t*             pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg)
{
    tresult_t           _result;
    BOOL                _bIsIdSet;
    tc_httpprc_ckeyq_t*  _pKeyId;
    U16                 _nKeyId;
    CHAR                _strCKey[512];

    _result = EFAILURE;
    if(tcHttpParseSvcTypeSess == pSvcType->svcType )
    {
		_pKeyId =
		        &(pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpSess[pSvcType->nSvcTypeIdx].tCtxId[0]);
		_nKeyId = pCntx->tPPSitesTbl[pHttpMsg->eSiteHost].tHttpSess[pSvcType->nSvcTypeIdx].nCtxId;
		_bIsIdSet = tcHttpParserGetCacheKey(
		        pCntx,
		        _strCKey,
		        sizeof(_strCKey),
		        _pKeyId,
		        _nKeyId,
		        pHttpMsg,
		        "-");
		if(_bIsIdSet)
		{
		    *pHash =
		            tcUtilHashBytes(
		                    (U8*)_strCKey, strlen(_strCKey));
		    _result = ESUCCESS;
		}
	}
    return _result;
}

/***************************************************************************
 * function: tcHttpParseReqUrlCb
 *
 * description: nginx HTTP parser Url callback
 ***************************************************************************/
CCUR_PROTECTED(int)
tcHttpParseReqUrlCb (http_parser *pParse, const CHAR *buf, size_t len)
{
    tc_httpparse_calldef_t*      _pHttpMsg;

    CCURASSERT(pParse);
    CCURASSERT(buf);

    _pHttpMsg = (tc_httpparse_calldef_t*)pParse->data;
    _pHttpMsg->tInjMsg.pUrl       = (CHAR*)buf;
    _pHttpMsg->tInjMsg.nUrlLen    = len;
    /*tcDebugPrintBuf(NULL,buf,len);*/

    return ESUCCESS;
}

/***************************************************************************
 * function: tcHttpParseReqHdrFieldCb
 *
 * description: nginx HTTP parser header field callback.
 ***************************************************************************/
CCUR_PROTECTED(int)
tcHttpParseReqHdrFieldCb (http_parser *pParse, const CHAR *buf, size_t len)
{
    tc_httpparse_calldef_t*  _pHttpMsg;
    tresult_t               _result;
    U16                     _i;
    I32                     _subStrVec[TRANSC_HTTPPRC_REGEXSUBEX_NUM];

    CCURASSERT(pParse);
    CCURASSERT(buf);

    _result = ESUCCESS;

    _pHttpMsg =
            (tc_httpparse_calldef_t*)pParse->data;
    if(tcHttpParseHdrMatchCheck ==
                    _pHttpMsg->eHttpHdrMatchType)
    {
        if(_pHttpMsg->pCntx->
                tPPSitesTbl[_pHttpMsg->ePossibleSiteHost].
                nHttpHdrMatch)
        {
            for(_i=0;_i<_pHttpMsg->pCntx->
                tPPSitesTbl[_pHttpMsg->ePossibleSiteHost].nHttpHdrMatch;_i++)
            {
                if(!strncmp(buf,_pHttpMsg->pCntx->
                        tPPSitesTbl[_pHttpMsg->ePossibleSiteHost].
                        tHttpHdrMatch[_i].strFieldName,len))
                {
                    _pHttpMsg->eHttpHdrMatchType = tcHttpParseHdrMatchProcess;
                    _pHttpMsg->nHttpHdrMatchIdx  = _i;
                    break;
                }
            }
        }
    }
    if(!strncmp(buf,"Host",len))
    {
        _pHttpMsg->eFieldType =
                tcHttpReqFieldypeHost;
    }
    else if(!strncmp(buf,"Referer",len))
    {
        _pHttpMsg->eFieldType =
                tcHttpReqFieldypeReferer;
    }
    else if(!strncmp(buf,"Origin",len))
    {
        _pHttpMsg->eFieldType =
                tcHttpReqFieldypeOrigin;
    }
    else
    {
#if TRANSC_TCSIM
        if(_pHttpMsg->pCntx->bBwSimMode)
        {
            if(!strncasecmp(buf,"Range",len))
            {
                _pHttpMsg->eFieldType =
                        tcHttpReqFieldypeSimRange;
            }
        }
#endif /* TRANSC_TCSIM */
        if(!strncasecmp(buf,"User-Agent",len))
        {
            _pHttpMsg->eFieldType =
                    tcHttpReqFieldypeUAgent;
        }
        if(_pHttpMsg->ePossibleSiteHost > 0 &&
           _pHttpMsg->ePossibleSiteHost < TRANSC_LDCFG_SITE_MAXSITES_LST)
        {
            tc_regex_t*                 _pRe;
            U32                         _nMatchSz;
            I32                         _rc;

            do
            {
                if('\0' != _pHttpMsg->pCntx->tPPSitesTbl[_pHttpMsg->ePossibleSiteHost].tHttpRgFld.strKey[0])
                {
                    _nMatchSz =
                            strlen(_pHttpMsg->pCntx->tPPSitesTbl[_pHttpMsg->ePossibleSiteHost].tHttpRgFld.strKey);
                    _pRe =
                            &(_pHttpMsg->pCntx->tPPSitesTbl[_pHttpMsg->ePossibleSiteHost].tHttpRgFld.tReCkey);
                    if(NULL == _pRe->code)
                        break;
                    _rc = tcRegexExec(
                            _pRe,
                            buf,_nMatchSz,
                            _subStrVec,TRANSC_HTTPPRC_REGEXSUBEX_NUM);
                    if(_rc >= 0)
                    {
                        _pHttpMsg->eFieldType =
                                tcHttpReqFieldypeRange;
                        break;
                    }
                }
            }while(FALSE);
        }
    }


    return _result;
}

/***************************************************************************
 * function: tcHttpParseReqHdrValueCb
 *
 * description: nginx HTTP parser header value callback.
 * match hostname specified within config file with hostname specified within
 * the url string.
 ***************************************************************************/
CCUR_PROTECTED(int)
tcHttpParseReqHdrValueCb (
        http_parser* pParse,
        const CHAR*  buf,
        size_t       len)
{
    tc_httpparse_calldef_t*  _pHttpMsg;
    hlpr_httpsite_hndl       _eSiteHost;

    CCURASSERT(pParse);
    CCURASSERT(buf);

    do
    {
        _pHttpMsg =
                (tc_httpparse_calldef_t*)pParse->data;
        if((_pHttpMsg->ePossibleSiteHost <= 0 &&
            _pHttpMsg->ePossibleSiteHost >= TRANSC_LDCFG_SITE_MAXSITES_LST))
        {
            _pHttpMsg->eSiteHost =
                    tcHttpParseSiteTypeUnknown;
            break;
        }
        switch(_pHttpMsg->eFieldType)
        {
            case tcHttpReqFieldypeHost:
                /* Process Hostname */
                if(len <
                          sizeof(_pHttpMsg->tInjMsg.strHostName)-1)
                {
                    strncpy(_pHttpMsg->tInjMsg.strHostName,buf,len);
                    _pHttpMsg->tInjMsg.strHostName[len] = '\0';
                    _pHttpMsg->eSiteHost =
                          _tcHttpParseGetHostType(
                                  _pHttpMsg->pCntx,
                                  _pHttpMsg->ePossibleSiteHost,
                                  buf,len);
                    if(tcHttpParseSiteTypeUnknown ==
                            _pHttpMsg->eSiteHost)
                    {
                        /* No config setup
                         * on the site host so use the possible host name */
                        if(0 ==
                                _pHttpMsg->pCntx->tPPSitesTbl[_pHttpMsg->ePossibleSiteHost].nHost)
                            _pHttpMsg->eSiteHost =
                                    _pHttpMsg->ePossibleSiteHost;
                    }
                }
                break;
            case tcHttpReqFieldypeOrigin:
                /* Process origin field */
                _tcHttpParseGetHttpOrigin(_pHttpMsg, buf,len);
                break;
            case tcHttpReqFieldypeReferer:
                /* Process httpreferersignature */
                _tcHttpParseGetHttpReferer(_pHttpMsg, buf,len);
                break;
            case tcHttpReqFieldypeRange:
                /* Process httprangefield */
                _tcHttpParseGetHttpHdrRange(_pHttpMsg, buf,len);
                break;
            case tcHttpReqFieldypeUAgent:
                /* Process Sim User Agent */
                _tcHttpParseGetHttpHdrSimUAgent(_pHttpMsg, buf,len);
                break;
#if TRANSC_TCSIM
            case tcHttpReqFieldypeSimRange:
                if(_pHttpMsg->pCntx->bBwSimMode)
                {
                    /* Process Sim Range */
                    _tcHttpParseGetHttpHdrSimRange(_pHttpMsg, buf,len);
                }
                break;
#endif /* TRANSC_TCSIM */
            default:
                break;
        }
        if(tcHttpParseHdrMatchProcess ==
                _pHttpMsg->eHttpHdrMatchType)
        {
            if(tcHttpParseSiteTypeUnknown == _pHttpMsg->eSiteHost)
                _eSiteHost = _pHttpMsg->ePossibleSiteHost;
            else
                _eSiteHost = _pHttpMsg->eSiteHost;
            /* Overloading hostsignature: checking */
            _pHttpMsg->eHttpHdrMatchType =
                  _tcHttpParseHttpHdrCk(
                          _pHttpMsg->pCntx,
                          _eSiteHost,
                          _pHttpMsg->nHttpHdrMatchIdx,
                          buf,len);
        }
        _pHttpMsg->eFieldType =
                tcHttpReqFieldypeNone;
    }while(FALSE);

    return ESUCCESS;
}
