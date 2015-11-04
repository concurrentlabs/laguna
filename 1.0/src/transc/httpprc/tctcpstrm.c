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
 * function: _tcTCPStreamSessEntryHtableFind
 *
 * description: Hash entry search function
 ***************************************************************************/
CCUR_PRIVATE(cdll_node_t*)
_tcTCPStreamSessEntryHtableFind(
        cdll_node_t*       pStrmSessList,
        U32                pktHash
        )
{
    cdll_node_t*                        _pNode;
    cdll_node_t*                        _pNextNode;
    tc_httpparse_sess_t*                _pStrmSess;

    _pNode = pStrmSessList;
    while(_pNode)
    {
        _pNextNode = _pNode->dllNext;
        _pStrmSess = (tc_httpparse_sess_t*)_pNode;
        if(pktHash == _pStrmSess->tSessData.PktHash)
            break;
        if(_pNextNode == pStrmSessList )
            _pNode = NULL;
        else
            _pNode = _pNextNode;
    }
    return _pNode;
}

/***************************************************************************
 * function: _tcTCPStreamSessEntryDestroy
 *
 * description: Destroy session control block entry and return it back to
 * memory pool.
 ***************************************************************************/
CCUR_PROTECTED(void)
_tcTCPStreamSessEntryDestroy(
        tc_httpprc_thread_ctxt_t*             pCntx,
        tc_httpparse_sess_t*                 pStrmSess)
{
    CCURASSERT(pCntx);
    CCURASSERT(pStrmSess);

    pCntx->shTcpStrmSessSitesTbl[pStrmSess->tSessData.eSiteHost][pStrmSess->tSessData.nBin].nCntr--;
    pCntx->nTcpStrmSessActive--;
    CcurCdllRemoveFromList(
            &(pCntx->shTcpStrmSessSitesTbl[pStrmSess->tSessData.eSiteHost][pStrmSess->tSessData.nBin].phdNode),
            &(pStrmSess->tStrmNode));
    cp_mempool_free(pCntx->pTcpStrmSessMpool,(void*)pStrmSess);
}

/***************************************************************************
 * function: _tcTCPStreamSessHtableCkTimeout
 *
 * description: Helper function to check timeout and destroy the entry.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcTCPStreamSessHtableCkTimeout(
        tc_httpprc_thread_ctxt_t*            pCntx,
        cdll_node_t*                        pNode,
        tc_gd_time_t*                       pNow)
{
    tc_httpparse_sess_t*    _pStrmSess;
    tc_gd_time_t            _tUpdDiff;

    _pStrmSess = (tc_httpparse_sess_t*)pNode;
    if(_pStrmSess)
    {
        if( pNow->nSeconds <= _pStrmSess->tLastUpdateTime.nSeconds)
        {
            _tUpdDiff.nSeconds = 1;
        }
        else
        {
            tUtilUTCTimeDiff(
                    &_tUpdDiff,
                    pNow,
                    &(_pStrmSess->tLastUpdateTime)
                    );
        }
        if( _tUpdDiff.nSeconds >= _pStrmSess->tSessData.nSessTimoutVal )
        {
            _tcTCPStreamSessEntryDestroy(pCntx,_pStrmSess);
        }
    }
}

/***************************************************************************
 * function: _tcTCPStreamSessHtableFindTimeout
 *
 * description: Search for timeout session control block, nth entries at
 * a time.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcTCPStreamSessHtableFindTimeout(
        tc_httpprc_thread_ctxt_t*            pCntx,
        hlpr_httpsite_hndl                  eSiteHost,
        tc_gd_time_t*                       pNow,
        U32                                 nNumEntriesCkd
        )
{
    tresult_t             _result = ESUCCESS;
    cdll_node_t*          _pNode;
    cdll_node_t*          _pHeadNode;
    cdll_node_t*          _pPrevNode;
    U32                   _nBinStart;
    U32                   _i;
    U32                   _NodeSrcCntr = 0;

    CCURASSERT(pCntx);
    CCURASSERT(pNow);

    if( (0 == pCntx->nTcpStrmSessHtblBin) ||
        (pCntx->nResTcpStrmSessBinMax <=
         pCntx->nTcpStrmSessHtblBin))
    {
        _nBinStart = 0;
    }
    else
    {
        _nBinStart = pCntx->nTcpStrmSessHtblBin;
    }
    for( _i = _nBinStart; _i < pCntx->nResTcpStrmSessBinMax; ++_i )
    {
        if(nNumEntriesCkd < _NodeSrcCntr )
            break;
        _pHeadNode =
                pCntx->shTcpStrmSessSitesTbl[eSiteHost][_i].phdNode;
        if(_pHeadNode)
        {
            _pNode = _pHeadNode->dllPrev;
            if(_pNode == _pHeadNode)
            {
                _tcTCPStreamSessHtableCkTimeout(
                        pCntx,_pNode,pNow);
                _NodeSrcCntr++;
            }
            else
            {
                while(_pNode)
                {
                    _pPrevNode = _pNode->dllPrev;
                    if(_pPrevNode == _pHeadNode )
                    {
                        _tcTCPStreamSessHtableCkTimeout(
                                pCntx,_pNode,pNow);
                        _tcTCPStreamSessHtableCkTimeout(
                                pCntx,_pPrevNode,pNow);
                        break;
                    }
                    else
                    {
                        _tcTCPStreamSessHtableCkTimeout(
                                pCntx,_pNode,pNow);
                    }
                    _NodeSrcCntr++;
                    _pNode = _pPrevNode;
                }
            }
        }
    }
    pCntx->nTcpStrmSessHtblBin = _i;

    return _result;
}

/**************** PROTECTED Functions **********************/

/***************************************************************************
 * function: tcTCPStreamSessEntryMpoolCreate
 *
 * description: create new memory pool
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcTCPStreamSessEntryMpoolCreate(
        tc_httpprc_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);

    pCntx->pTcpStrmSessMpool =
        cp_mempool_create_by_option(COLLECTION_MODE_NOSYNC,
            sizeof(tc_httpparse_sess_t), 10);
    return ESUCCESS;
}

/***************************************************************************
 * function: tcTCPStreamSessEntryMpoolDestroy
 *
 * description: Destroy entire memory pool
 ***************************************************************************/
CCUR_PROTECTED(void)
tcTCPStreamSessEntryMpoolDestroy(
        tc_httpprc_thread_ctxt_t*        pCntx)
{
    if(pCntx->pTcpStrmSessMpool)
	cp_mempool_destroy(pCntx->pTcpStrmSessMpool);
}

/***************************************************************************
 * function: tcTCPStreamSessEntryMpoolGetStats
 *
 * description: Get memory pool statistics.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcTCPStreamSessEntryMpoolGetStats(
        tc_httpprc_thread_ctxt_t*        pCntx)
{
    CCURASSERT(pCntx);

#ifdef ZZZ
    if(pCntx->pTcpStrmSessMpool)
        tcUtilMemPoolGetStats(pCntx->pTcpStrmSessMpool,
                &(pCntx->tTcpStrmSessMPoolStats));
#endif
}

/***************************************************************************
 * function: tcTCPStreamSessHtableTimeoutCheck
 *
 * description: Search for timed out session GET requests within
 * session hash table.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcTCPStreamSessHtableTimeoutCheck(
        tc_httpprc_thread_ctxt_t*   pCntx,
        tc_gd_time_t*               pUpdDiff)
{
    /* TODO: Handle time rollback */
    hlpr_httpsite_hndl  _eSiteHost;
    tc_gd_time_t        _tTimeNow;

    CCURASSERT(pCntx);

    pCntx->tTcpStrmHashTblCkTime.nSeconds += pUpdDiff->nSeconds;
    if(pCntx->tTcpStrmHashTblCkTime.nSeconds >=
            TRANSC_HTTPPRC_TCPSTRM_TTLTIME_SEC)
    {
        pCntx->tTcpStrmHashTblCkTime.nSeconds = 0;
        tUtilUTCTimeGet(&_tTimeNow);
        for(_eSiteHost=1;
                _eSiteHost<pCntx->nPPSitesTbl;_eSiteHost++)
        {
            if(pCntx->tPPSitesTbl[_eSiteHost].nHttpSess &&
               pCntx->tPPSitesTbl[_eSiteHost].nHttpURI)
            {
                _tcTCPStreamSessHtableFindTimeout(
                        pCntx,
                        _eSiteHost,
                        &_tTimeNow,
                        TRANSC_HTTPPRC_TCPSTRM_CTLSRCENTRIES_NUM);
            }
        }
    }
}

/***************************************************************************
 * function: tcTCPStreamSessGetSessCacheKeyRel
 *
 * description: Associate TCP stream session with Video stream when cache
 * key id or misc information can not be found within video stream GET request.
 * video stream GET request will be ignored if it arrives before the GET
 * request session.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcTCPStreamSessGetSessCacheKeyRel(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_httpparse_calldef_t*      pHttpMsg,
        tc_g_svcdesc_t*              pSvcType
        )
{
    tresult_t                           _result;
    U32                                 _nBin;
    cdll_node_t*                        _pNode;
    cdll_node_t*                        _pStrmSessList;
    U32                                 _pHash;
    tc_httpparse_sess_t*                _pStrmSess;

    CCURASSERT(pCntx);
    CCURASSERT(pHttpMsg);

    do
    {
        _pStrmSess = NULL;
        /* TODO: Split into separate functions */
        if(tcHttpParseSvcTypeSess ==
                pSvcType->svcType)
        {
            _result =
                    tcHttpParserGetSessHashCntxId(
                            pCntx,
                            &_pHash,
                            pSvcType,
                            pHttpMsg);
            if(ESUCCESS != _result)
                break;
            _nBin = _pHash %
                    pCntx->nResTcpStrmSessBinMax;
            if(_nBin >= pCntx->nResTcpStrmSessBinMax)
                break;
            _pStrmSessList = pCntx->shTcpStrmSessSitesTbl[pHttpMsg->eSiteHost][_nBin].phdNode;
            _pNode = _tcTCPStreamSessEntryHtableFind(
                    _pStrmSessList,_pHash);
            if(_pNode)
            {
                _pStrmSess = (tc_httpparse_sess_t*)_pNode;
                _pStrmSess->tSessData.nSessTimoutVal    =
                        TRANSC_HTTPPRC_TCPSTRM_UNCORRINACTIVETIME_SEC;
                tUtilUTCTimeGet(
                        &(_pStrmSess->tLastUpdateTime) );
                _result = EIGNORE;
            }
            else
            {
                /* Create new stream */
                _pStrmSess = (tc_httpparse_sess_t*)
			cp_mempool_alloc(pCntx->pTcpStrmSessMpool);
                if(NULL == _pStrmSess)
                    break;
                /*********************************/
                /* Init Stream Session Tracking  */
                /*********************************/
                _pStrmSess->nReq                        = 0;
                _pStrmSess->tSessData.nBin              = _nBin;
                _pStrmSess->tSessData.PktHash           = _pHash;
                _pStrmSess->tSessData.eSiteHost         = pHttpMsg->eSiteHost;
                _pStrmSess->tSessData.strCId[0]         = '\0';
                _pStrmSess->tSessData.strCRange[0]      = '\0';
                _pStrmSess->tSessData.strCMisc[0]       = '\0';
                _pStrmSess->tSessData.nSessTimoutVal    = TRANSC_HTTPPRC_TCPSTRM_UNCORRINACTIVETIME_SEC;
                /* Store cachkeyid, range and cachekey misc into streams,
                 * careful _pStrmSess values will be concatenated */
                _result = tcHttpParserGetAllSessCacheKeyId(
                        pCntx,pSvcType,pHttpMsg,_pStrmSess);
                if(ESUCCESS != _result)
                {
                    /* Release if sess doesn't even have video cachekey*/
                    cp_mempool_free(pCntx->pTcpStrmSessMpool, (void*)_pStrmSess);
                    _pStrmSess = NULL;
                    break;
                }
                tUtilUTCTimeGet(
                        &(_pStrmSess->tLastUpdateTime) );
                /******************************/
                /* Log Stream Statistics      */
                /******************************/
                pCntx->shTcpStrmSessSitesTbl[pHttpMsg->eSiteHost][_nBin].nCntr++;
                CcurCdllInsertToList(
                        &(pCntx->shTcpStrmSessSitesTbl[pHttpMsg->eSiteHost][_nBin].phdNode),
                        &(_pStrmSess->tStrmNode));
                pCntx->nTcpStrmSessActive++;
            }
            if(NULL == _pStrmSess)
            {
                if(ESUCCESS == _result)
                    _result = EIGNORE;
                else
                    _result = EFAILURE;
            }
        }
        else
        {
            /* if video id is provided within http header then
             * use it
             */
            if('\0' != pHttpMsg->tInjMsg.strCId[0])
            {
                /* if range is provided within http header then
                 * use it
                 */
                if('\0' == pHttpMsg->tInjMsg.strCRange[0])
                {
                    /* If range is provided within this video msg then
                     * use it */
                    tcHttpParserGetUrlCacheKeyRange(
                            pCntx,
                            pSvcType,
                            pHttpMsg);
                    /* No Range means, the range will be
                       set to "0-" */
                }
                /* ---> (Optional) Miscelaneous cache key is OR-ed value,
                 * just append any needed info
                 */
                /*pHttpMsg->tInjMsg.strCMisc[0] = '\0';*/
                tcHttpParserGetUrlCacheKeyMisc(
                        pCntx,
                        pSvcType,
                        pHttpMsg);
                if('\0' == pHttpMsg->tInjMsg.strCMisc[0])
                {
                    ccur_strlcat(pHttpMsg->tInjMsg.strCMisc,
                            "-sessid-0",
                            sizeof(pHttpMsg->tInjMsg.strCMisc));
                }
                _result = ESUCCESS;
            }
            /* Otherwise we need to grab it from other message
             * within this session.
             */
            else
            {
                _result =
                        tcHttpParserGetUrlHashCntxId(
                                pCntx,
                                &_pHash,
                                pSvcType,
                                pHttpMsg);
                if(ESUCCESS != _result)
                    break;
                _nBin = _pHash %
                        pCntx->nResTcpStrmSessBinMax;
                if(_nBin >= pCntx->nResTcpStrmSessBinMax)
                    break;
                _pStrmSessList = pCntx->shTcpStrmSessSitesTbl[pHttpMsg->eSiteHost][_nBin].phdNode;
                _pNode =
                        _tcTCPStreamSessEntryHtableFind(
                                _pStrmSessList,_pHash);
                if(_pNode)
                {
                    _pStrmSess = (tc_httpparse_sess_t*)_pNode;
                    _pStrmSess->nReq++;
                    _pStrmSess->tSessData.nSessTimoutVal    =
                            TRANSC_HTTPPRC_TCPSTRM_CORRINACTIVETIME_SEC;
                    /* video and session are correlated, need to
                     * extend the time to cleanup the video message*/
                    tUtilUTCTimeGet(
                            &(_pStrmSess->tLastUpdateTime) );

                    /******************************/
                    /* Construct send message     */
                    /******************************/

                    do
                    {
                        /* Construct send message */
                        /* Need to construct cache key, that consists of atleast
                         * video id + range + misc (optional)
                         */
                        /* ---> (Required) Cache key video id priority order of existence:
                         * 1. use from http stream if exists
                         * 2. error out if none exists
                         */
                        if('\0' != _pStrmSess->tSessData.strCId[0])
                        {
                            ccur_strlcpy(pHttpMsg->tInjMsg.strCId,
                                    _pStrmSess->tSessData.strCId,
                                    sizeof(pHttpMsg->tInjMsg.strCId));
                        }
                        /* Cachekey video id must exists */
                        /* No, video id is not acceptable, error out. */
                        else
                        {
                            _result = EFAILURE;
                            _pStrmSess = NULL;
                            break;
                        }
                        /* ---> (Required) Cache key Range priority order of existence:
                         * 1. use from http header if exists
                         * 2. use from http stream if exists
                         * 3. use from http sess   if exists
                         * 4. use random number if none exists
                         */
                        /* if range is provided within http header then
                         * use it
                         */
                        if('\0' == pHttpMsg->tInjMsg.strCRange[0])
                        {
                            /* If range is provided within this video msg then
                             * use it */
                            tcHttpParserGetUrlCacheKeyRange(
                                    pCntx,
                                    pSvcType,
                                    pHttpMsg);
                            if('\0' == pHttpMsg->tInjMsg.strCRange[0])
                            {
                                /* if range is provided within session msg
                                 * then use it
                                 */
                                if('\0' != _pStrmSess->tSessData.strCRange[0])
                                {
                                    ccur_strlcpy(
                                            pHttpMsg->tInjMsg.strCRange,
                                            _pStrmSess->tSessData.strCRange,
                                            sizeof(pHttpMsg->tInjMsg.strCRange));
                                }
                            }
                            /* Cachekey id and some kind of range must exists */
                            /* No, Range, odd. Then use counters to make
                             * distinction */
                            if('\0' == pHttpMsg->tInjMsg.strCRange[0])
                            {
                                snprintf(pHttpMsg->tInjMsg.strCMisc,
                                        sizeof(pHttpMsg->tInjMsg.strCMisc),
                                        "range-%lu",_pStrmSess->nReq);
                                pHttpMsg->tInjMsg.strCMisc[sizeof(pHttpMsg->tInjMsg.strCMisc)-1] = '\0';
                            }
                        }
                        /* ---> (Optional) Miscelaneous cache key is OR-ed value,
                         * just append any needed info
                         */
                        /*pHttpMsg->tInjMsg.strCMisc[0] = '\0';*/
                        tcHttpParserGetUrlCacheKeyMisc(
                                pCntx,
                                pSvcType,
                                pHttpMsg);
                        if('\0' != _pStrmSess->tSessData.strCMisc[0])
                        {
                            ccur_strlcat(
                                    pHttpMsg->tInjMsg.strCMisc,
                                    _pStrmSess->tSessData.strCMisc,
                                    sizeof(pHttpMsg->tInjMsg.strCMisc));
                        }
                        else
                        {
                            ccur_strlcat(pHttpMsg->tInjMsg.strCMisc,
                                    "-sessid-0",
                                    sizeof(pHttpMsg->tInjMsg.strCMisc));
                        }
                    }while(FALSE);
                }
                if(NULL == _pStrmSess)
                {
                    if(ESUCCESS == _result)
                        _result = EIGNORE;
                    else
                        _result = EFAILURE;
                }
            }
        }
    } while (FALSE);

    return _result;
}