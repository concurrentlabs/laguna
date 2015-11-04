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

#include <arpa/inet.h>
#include <curl/curl.h>
#include <fcntl.h>
#include "simprc.h"

/***************************************************************************
 * function: _tcSimLogStatsSummary
 *
 * description: thread summary such as thread up time and
 * message processed/sec.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcSimLogStatsSummary(
        tc_sim_thread_ctxt_t* pCntx)
{
    U32                     _nRunTimeSecs;
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
    evLogTrace(
            pCntx->pQSimToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "****Sim Proc TID#%x Summary****\n"
            "Uptime:%s\n"
            "Total Running time:%ld mins\n"
            "Total HTTP processed/sec:%ld\n",
            pCntx->tid,
            _pTNowbuf,
            _tDiffTime.nSeconds/60,
            (pCntx->nHttpHEADReq+pCntx->nHttpGETReq)/_nRunTimeSecs
            );
}

/***************************************************************************
 * function: _tcSimLogStats
 *
 * description: thread detailed stats dump.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcSimLogStats(tc_sim_thread_ctxt_t * pCntx)
{
    CCURASSERT(pCntx);

    if (TRANSC_ZLOGCATEGORY_BITMAP_CK(pCntx->tLogDescSys,evLogLvlInfo))
        return;

    evLogTrace(
            pCntx->pQSimToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "****Sim Proc TID#%x Info****\n"
            "    HTTP Pkts Total:%lu\n"
            "       HTTP Pkts GET:%lu\n"
            "       HTTP Pkts HEAD:%lu\n"
            "       HTTP Pkts Error:%lu\n"
            "           No Q Memory sim->SimSnd Error:%lu\n"
            "       HTTP Pkts Ignored:%lu\n"
            "    CkeyMap Hash Table info:\n"
            "       Active Video Streams:%lu\n"
            ,
            pCntx->tid,
            pCntx->nHttpReq,
            pCntx->nHttpGETReq,
            pCntx->nHttpHEADReq,
            pCntx->nErrCount,
                pCntx->nErrNoMemSimToSimSnd,
            pCntx->nIgnCount,
            pCntx->nCkeyActive
            );
}

/***************************************************************************
 * function: _tcSimW3cLog
 *
 * description: W3c log formatting and writing to background logging thread.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimW3cLog(
        tc_sim_thread_ctxt_t*        pCntx,
        CHAR*                        pStrStaticId,
        CHAR*                        pStrContentLen,
        tc_sim_msgpyldtype_e         eMsgPyldType,
        tc_qmsgtbl_pptosim_t*        pQMsg)
{
    tresult_t   _result;
    CHAR        _strDstddr[64];
    CHAR        _strSrcddr[64];
    CHAR        _strtmbuf[64];
    CHAR        _strRespType[8];
    CHAR        _strUrlBuf[TRANSC_SIM_URLBUFF];
    U16         _nStrUrl;
    CHAR*       _pHostName;
    time_t      _tNowTime;
    struct tm*  _pNowTm;

    do
    {
        _result = EFAILURE;
        if('\0' == pStrContentLen[0] ||
           '\0' == pStrStaticId[0])
            break;

        if('\0' != pQMsg->strHostName[0])
            _pHostName = pQMsg->strHostName;
        else
        {
            tcUtilIPAddrtoAscii(
                &(pQMsg->tDstIP),
                _strDstddr,
                sizeof(_strDstddr));
            _pHostName = _strDstddr;
        }
        tcUtilIPAddrtoAscii(
            &(pQMsg->tSrcIP),
            _strSrcddr,
            sizeof(_strSrcddr));
        /*The common logfile format is as follows:
        remotehost rfc931 authuser [date] "request" status bytes */
        time( &_tNowTime );
        _pNowTm = localtime(&(_tNowTime));
        strftime(_strtmbuf, sizeof(_strtmbuf), "%d/%b/%Y:%T %z",_pNowTm);
        _strUrlBuf[0] = '\0';
        _nStrUrl = strlen(pQMsg->strUrl);
        if(_nStrUrl < sizeof(_strUrlBuf)-1)
        {
            strncpy(_strUrlBuf,pQMsg->strUrl,_nStrUrl);
            _strUrlBuf[_nStrUrl] = '\0';
        }
        else
            ccur_strlcpy(_strUrlBuf,pQMsg->strUrl,sizeof(_strUrlBuf)-1);
        if(tcSimHttpMsgRespPyldType206 == eMsgPyldType)
            strcpy(_strRespType,"206");
        else
            strcpy(_strRespType,"200");
        if('\0' == pQMsg->strUAgent[0])
        {
            evLogTrace(
                    pCntx->pQSimToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSvc),
                    "TID:%x %s %s - [%s] \"GET %s/ccur/%s/%s/tcshost/%s/tcskey/%s/%s_%s/tcsopt/%s/tcsosig%s HTTP/1.1\" %s %s \"-\" \"-\"\n",
                    pCntx->tid,
                    _strSrcddr,
                    _pHostName,
                    _strtmbuf,
                    pQMsg->strRedirTarget,
                    pQMsg->strSvcType,
                    pQMsg->strSvcName,
                    pQMsg->strHostName,
                    pStrStaticId,
                    pQMsg->strCRange,
                    pQMsg->strCMisc,
                    pQMsg->strOptions,
                    _strUrlBuf,
                    _strRespType,
                    pStrContentLen
                    );
        }
        else
        {
            evLogTrace(
                    pCntx->pQSimToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSvc),
                    "TID:%x %s %s - [%s] \"GET %s/ccur/%s/%s/tcshost/%s/tcskey/%s/%s_%s/tcsopt/%s/tcsosig%s HTTP/1.1\" %s %s \"-\" \"%s\"\n",
                    pCntx->tid,
                    _strSrcddr,
                    _pHostName,
                    _strtmbuf,
                    pQMsg->strRedirTarget,
                    pQMsg->strSvcType,
                    pQMsg->strSvcName,
                    pQMsg->strHostName,
                    pStrStaticId,
                    pQMsg->strCRange,
                    pQMsg->strCMisc,
                    pQMsg->strOptions,
                    _strUrlBuf,
                    _strRespType,
                    pStrContentLen,
                    pQMsg->strUAgent
                    );
        }
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcSimCkeyEntryDestroy
 *
 * description: Destroy session control block entry and return it back to
 * memory pool.
 ***************************************************************************/
CCUR_PROTECTED(void)
_tcSimCkeyEntryDestroy(tc_sim_thread_ctxt_t * pCntx, tc_sim_ckeymap_t * pEntry)
{
    CCURASSERT(pCntx);
    CCURASSERT(pEntry);

    pCntx->shCkeyMapHashTable[pEntry->nBin].nCntr--;
    pCntx->nCkeyActive--;
    CcurCdllRemoveFromList(
            &(pCntx->shCkeyMapHashTable[pEntry->nBin].phdNode),
            &(pEntry->tNode));
    cp_mempool_free(pCntx->pCkeyMapMpool,(void*)pEntry);
}

/***************************************************************************
 * function: _tcSimHtableCkTimeout
 *
 * description: Helper function to check timeout and destroy the entry.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcSimHtableCkTimeout(tc_sim_thread_ctxt_t * pCntx, cdll_node_t * pNode,
        tc_gd_time_t * pNow)
{
    tc_sim_ckeymap_t*    _pCkeyMap;
    tc_gd_time_t         _tUpdDiff;

    _pCkeyMap = (tc_sim_ckeymap_t*)pNode;
    if(_pCkeyMap)
    {
        if( pNow->nSeconds <= _pCkeyMap->tLastUpdateTime.nSeconds)
        {
            _tUpdDiff.nSeconds = 1;
        }
        else
        {
            tUtilUTCTimeDiff(
                    &_tUpdDiff,
                    pNow,
                    &(_pCkeyMap->tLastUpdateTime)
                    );
        }
        if( _tUpdDiff.nSeconds >= _pCkeyMap->nTimoutSec )
        {
            _tcSimCkeyEntryDestroy(pCntx,_pCkeyMap);
        }
    }
}

/***************************************************************************
 * function: _tcSimHtableFindTimeout
 *
 * description: Search for timeout session control block, nth entries at
 * a time.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimHtableFindTimeout(tc_sim_thread_ctxt_t * pCntx, tc_gd_time_t * pNow,
        U32 nNumEntriesCkd)
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

    if( (0 == pCntx->nCkeyMapHtblBin) ||
        (pCntx->nResCkeyMapBinMax <=
         pCntx->nCkeyMapHtblBin))
    {
        _nBinStart = 0;
    }
    else
    {
        _nBinStart = pCntx->nCkeyMapHtblBin;
    }
    for( _i = _nBinStart; _i < pCntx->nResCkeyMapBinMax; ++_i )
    {
        if(nNumEntriesCkd < _NodeSrcCntr )
            break;
        _pHeadNode =
                pCntx->shCkeyMapHashTable[_i].phdNode;
        if(_pHeadNode)
        {
            _pNode = _pHeadNode->dllPrev;
            if(_pNode == _pHeadNode)
            {
                _tcSimHtableCkTimeout(
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
                        _tcSimHtableCkTimeout(
                                pCntx,_pNode,pNow);
                        _tcSimHtableCkTimeout(
                                pCntx,_pPrevNode,pNow);
                        break;
                    }
                    else
                    {
                        _tcSimHtableCkTimeout(
                                pCntx,_pNode,pNow);
                    }
                    _NodeSrcCntr++;
                    _pNode = _pPrevNode;
                }
            }
        }
    }
    pCntx->nCkeyMapHtblBin = _i;

    return _result;
}

/***************************************************************************
 * function: _tcSimIsCkeyMap
 *
 * description: check to see if cache key map "ckeymap" option is
 * specified as cache option value or not.
 ***************************************************************************/
CCUR_PRIVATE(BOOL) _tcSimIsCkeyMap(CHAR* strOptions)
{
    BOOL        _bIsCkeyMap;
    CHAR        _tmpBuff[256];
    CHAR*       _arg;
    CHAR*       _endStr;
    U16         _i;

    CCURASSERT(strOptions);

    _bIsCkeyMap = FALSE;
    ccur_strlcpy(_tmpBuff,strOptions,sizeof(_tmpBuff));
    _arg = strtok_r(
         _tmpBuff,"-",&_endStr);
    _i=0;
    while(_arg && _i < 2)
    {
        if(1 == _i)
        {
            if(!ccur_strcasecmp("ckeymap",_arg))
                _bIsCkeyMap = TRUE;
        }
        _arg = strtok_r( NULL, "-" ,&_endStr);
        _i++;
    }

    return _bIsCkeyMap;
}

/***************************************************************************
 * function: _tcSimGetPktInfoFromCkeyMapTable
 *
 * description: Get packet info entry from ckey map hash table.
 * Packet info is request message coming from pp to sim thread.
 ***************************************************************************/
CCUR_PRIVATE(tc_sim_ckeymap_t*)
_tcSimGetPktInfoFromCkeyMapTable(tc_sim_thread_ctxt_t * pCntx, CHAR * strDynId)
{
    cdll_node_t*        _pNode;
    cdll_node_t*        _pCkeyMapList;
    cdll_node_t*        _pNextNode;
    tc_sim_ckeymap_t*   _pCkeyMap;
    U32                 _nHash;
    U32                 _nBin;

    do
    {
        _pCkeyMap = NULL;
        _nHash =
                tcUtilHashBytes(
                        (U8*)strDynId,
                        strlen(strDynId));
        _nBin = _nHash % pCntx->nResCkeyMapBinMax;
        if(_nBin >= pCntx->nResCkeyMapBinMax)
            break;
        _pCkeyMapList = _pNode =
                pCntx->shCkeyMapHashTable[_nBin].phdNode;
        while(_pNode)
        {
            _pNextNode = _pNode->dllNext;
            _pCkeyMap = (tc_sim_ckeymap_t*)_pNode;
            if(_nHash == _pCkeyMap->nDynHashId)
                break;
            if(_pNextNode == _pCkeyMapList)
                _pNode = NULL;
            else
                _pNode = _pNextNode;
        }
        /* Found! */
        if(_pNode)
        {
            _pCkeyMap = (tc_sim_ckeymap_t*)_pNode;
            tUtilUTCTimeGet(
                    &(_pCkeyMap->tLastUpdateTime) );
        }
    }while(FALSE);

    return _pCkeyMap;
}

/***************************************************************************
 * function: _tcSimCreateCkeyMapEntry
 *
 * description: Create packet info and add it to ckey map hash table.
 * Packet info is request message coming from pp to sim thread.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimCreateCkeyMapEntry(tc_sim_thread_ctxt_t * pCntx,
	tc_qmsgtbl_pptosim_t * pMsg)
{
    tresult_t           _result;
    U32                 _nHash;
    U32                 _nBin;
    tc_sim_ckeymap_t*   _pCkeyMap;

    do
    {
        _result   = EFAILURE;
        _nHash =
                tcUtilHashBytes(
                        (U8*)pMsg->strCId,
                        strlen(pMsg->strCId));
        _nBin = _nHash % pCntx->nResCkeyMapBinMax;
        if(_nBin >= pCntx->nResCkeyMapBinMax)
            break;
        _pCkeyMap =  (tc_sim_ckeymap_t*)
                cp_mempool_alloc(pCntx->pCkeyMapMpool);
        if(NULL == _pCkeyMap)
        {
            _result = ENOMEM;
            break;
        }
        _pCkeyMap->nBin       = _nBin;
        _pCkeyMap->nDynHashId = _nHash;
        _pCkeyMap->nTimoutSec =
                TRANSC_SIMCKEY_UNCORRINACTIVETIME_SEC;
        memcpy(&(_pCkeyMap->tPktInfo),
                pMsg,sizeof(tc_qmsgtbl_pptosim_t));
        _pCkeyMap->tPktInfo.strCId[0] = '\0';
        /* Time Stamp it */
        tUtilUTCTimeGet(
                &(_pCkeyMap->tLastUpdateTime) );
        /* Add it and count it */
        CcurCdllInsertToList(
                &(pCntx->shCkeyMapHashTable[_nBin].phdNode),
                &(_pCkeyMap->tNode));
        pCntx->shCkeyMapHashTable[_nBin].nCntr++;
        pCntx->nCkeyActive++;
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcSimSendReqToSimsndQ
 *
 * description: Send message request from sim thread to sim/snd worker thread
 * and distribute the requests to multiple sim/snd worker threads, RR style.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimSendReqToSimsndQ(
        tc_sim_thread_ctxt_t*        pCntx,
        CHAR*                        pStrSId,
        CHAR*                        pStrDynId,
        tc_sim_reqtype_e             eReqType,
        tc_sim_rangetype_e           eRgType,
        tc_sim_msgpyldtype_e         ePyldType,
        tc_qmsgtbl_pptosim_t*        pQMsg)
{
    tresult_t                               _result;
    tc_qmsgtbl_simtosimsnd_t*               _pQMsg;

    _result = ESUCCESS;
    if(pCntx->nSimMsgOflw <= 0)
    {
        if(pCntx->nRRQSimToSimSnd == pCntx->nSimSendWThreadsNum)
            pCntx->nRRQSimToSimSnd = 0;
        _pQMsg = (tc_qmsgtbl_simtosimsnd_t*)
                lkfqMalloc(pCntx->pQSimToSimSendWTbl[pCntx->nRRQSimToSimSnd]);
        if(_pQMsg)
        {
            /* Perform other copy operations here ... */
            /* Write message */
            /* Whole lot of copying...*/
            /*memcpy(&(_pQMsg->tMsg),pQMsg,sizeof(tc_qmsgtbl_pptosim_t));*/
            ccur_strlcpy(_pQMsg->strDynId,pStrDynId,sizeof(_pQMsg->strDynId));
            _pQMsg->eReqType    = eReqType;
            _pQMsg->eRgType     = eRgType;
            _pQMsg->ePyldType   = ePyldType;
            ccur_strlcpy(_pQMsg->tMsg.strCId,pStrSId,sizeof(_pQMsg->tMsg.strCId));
            ccur_strlcpy(_pQMsg->tMsg.strUrl,
                    pQMsg->strUrl,
                    sizeof(_pQMsg->tMsg.strUrl));
            ccur_strlcpy(_pQMsg->tMsg.strCRange,
                    pQMsg->strCRange,
                    sizeof(_pQMsg->tMsg.strCRange));
            _pQMsg->tMsg.bIsHttpRgReq = pQMsg->bIsHttpRgReq;
            ccur_strlcpy(_pQMsg->tMsg.strCMisc,
                    pQMsg->strCMisc,
                    sizeof(_pQMsg->tMsg.strCMisc));
            _pQMsg->tMsg.tSrcIP = pQMsg->tSrcIP;
            _pQMsg->tMsg.tDstIP = pQMsg->tDstIP;
            _pQMsg->tMsg.nSrcPort = pQMsg->nSrcPort;
            _pQMsg->tMsg.nDstPort = pQMsg->nDstPort;
            ccur_strlcpy(_pQMsg->tMsg.strUAgent,
                    pQMsg->strUAgent,
                    sizeof(_pQMsg->tMsg.strUAgent));
            ccur_strlcpy(_pQMsg->tMsg.strHostName,
                    pQMsg->strHostName,
                    sizeof(_pQMsg->tMsg.strHostName));
            ccur_strlcpy(_pQMsg->tMsg.strRedirTarget,
                    pQMsg->strRedirTarget,
                    sizeof(_pQMsg->tMsg.strRedirTarget));
            /* Service type */
            ccur_strlcpy(_pQMsg->tMsg.strSvcType,
                    pQMsg->strSvcType,
                    sizeof(_pQMsg->tMsg.strSvcType));
            /* Service name */
            ccur_strlcpy(_pQMsg->tMsg.strSvcName,
                    pQMsg->strSvcName,
                    sizeof(_pQMsg->tMsg.strSvcName));
            ccur_strlcpy(_pQMsg->tMsg.strOptions,
                    pQMsg->strOptions,
                    sizeof(_pQMsg->tMsg.strOptions));
            /* Quick and dirty solution of passing the reload
             * config flag. This can be an issue if chlrns Qs
             * are all full.
             */
            if(pCntx->bLoadSimSndCfgTbl[pCntx->nRRQSimToSimSnd])
            {
                _pQMsg->bLdCfgYaml = pCntx->bLoadSimSndCfgTbl[pCntx->nRRQSimToSimSnd];
                pCntx->bLoadSimSndCfgTbl[pCntx->nRRQSimToSimSnd] = FALSE;
            }
            lkfqWrite(pCntx->pQSimToSimSendWTbl[pCntx->nRRQSimToSimSnd],
                    (lkfq_data_p)_pQMsg);
        }
        else
        {
            _result = ENOBUFS;
            evLogTrace(
                    pCntx->pQSimToBkgrnd,
                    evLogLvlWarn,
                    &(pCntx->tLogDescSys),
                    "Sim thread bpool out of msg buffer, "
                    "stop queing msg for %d iterations",
                    TRANSC_SIMPRC_RDROVFLW_CNT);
            pCntx->nSimMsgOflw =
                    TRANSC_SIMPRC_RDROVFLW_CNT;
        }
        pCntx->nRRQSimToSimSnd++;
    }
    else
        pCntx->nSimMsgOflw--;

    return _result;
}

/***************************************************************************
 * function: _tcSimGetRangeValType
 *
 * description: Get Range type, there can be only few options:
 * There are 4 types of range types:
 * 1. (digits)-
 * 2. -(digits)
 * 3. (digits)-(digits)
 * 4. (digits)-(digits),(digits)-(digits),..
 ***************************************************************************/
CCUR_PRIVATE(tc_sim_rangetype_e)
_tcSimGetRangeValType(CHAR * strCRange, BOOL bIsHttpRgReq)
{
    BOOL                _bIsFirstValid;
    BOOL                _bIsSecondValid;
    BOOL                _bMultiDigitsValid;
    CHAR                _strCRange[TRANSC_SIM_RGBUFF];
    CHAR*               _StrS;
    CHAR*               _StrE;
    tc_sim_rangetype_e  _eRgType;

    do
    {
        _eRgType = tcRangeTypeUnknown;
        /* quick check */
        if('-' != strCRange[0] &&
           !isdigit(strCRange[0]))
            break;
        else if(strchr(strCRange,','))
        {
            /* ',' and and http header, it is definitely
             * http range request. */
            if(bIsHttpRgReq)
                _eRgType = tcRangeTypeMultiPartRanges;
            else
            {
                /* URL, this could be multi part byte range or
                 * incorrect input. Need to validate.
                 * Check only the first segment of multi part byte
                 * ranges, later on we will do full check.
                 */
                ccur_strlcpy(_strCRange,strCRange,sizeof(_strCRange));
                _StrE = strchr(_strCRange,',');
                if(_StrE)
                {
                    *_StrE = '\0';
                    _bMultiDigitsValid =
                            tcSimUtilIsMultiDigitsRange(_strCRange);
                    if(_bMultiDigitsValid)
                        _eRgType = tcRangeTypeMultiPartRanges;
                    else
                        _eRgType = tcRangeTypeUnknown;
                }
                else
                    _eRgType = tcRangeTypeUnknown;
            }
        }
        else if(TRANSC_SIM_ISNO_RANGE_CK(strCRange))
            _eRgType = tcRangeTypeDigitsAndDash;
        else
        {
            /* There are only four range possibilities:
             * 1. (digits)-
             * 2. -(digits)
             * 3. (digits)-(digits)
             * 4. (digits)-(digits),(digits)-(digits),...
             */
            _StrS = strCRange;
            _StrE = strchr(strCRange,'-');
            if(NULL == _StrE)
                break;
            /* -(digits) case */
            if(_StrE == _StrS)
            {
                _StrS = ++_StrE;
                if('\0' == *_StrS )
                {
                    _eRgType = tcRangeTypeDigitsAndDash;
                    break;
                }
                _bIsSecondValid = TRUE;
                while('\0' != *_StrS)
                {
                    if(!isdigit(*_StrS))
                    {
                        _bIsSecondValid = FALSE;
                        break;
                    }
                    _StrS++;
                }
                if(FALSE == _bIsSecondValid)
                    break;
                _eRgType = tcRangeTypeDigitsAndDash;
            }
            /* (digits)- and (digits)-(digits) case */
            else
            {
                _bIsFirstValid = FALSE;
                _bIsSecondValid = FALSE;
                if(!isdigit(_StrS[0]))
                    break;
                /* Check the first portion */
                _bIsFirstValid = TRUE;
                while('-' != *_StrS)
                {
                    if(!isdigit(*_StrS))
                    {
                        _bIsFirstValid = FALSE;
                        break;
                    }
                    _StrS++;
                }
                if(FALSE == _bIsFirstValid)
                    break;
                _StrS = ++_StrE;
                if('\0' == *_StrS )
                {
                    _eRgType = tcRangeTypeDigitsAndDash;
                    break;
                }
                /* Check the second portion */
                _bIsSecondValid = TRUE;
                while('\0' != *_StrS &&
                      ('/' != *_StrS))
                {
                    if(!isdigit(*_StrS))
                    {
                        _bIsSecondValid = FALSE;
                        break;
                    }
                    _StrS++;
                }
                if(FALSE == _bIsSecondValid)
                    break;
                _eRgType = tcRangeTypeDigitsToDigits;
            }
        }
    }while(FALSE);

    return _eRgType;
}

/***************************************************************************
 * function: _tcSimGetRangeType
 *
 * description: Process, cleanup cache key range and detect range value type.
 * There are 4 types of range types:
 * 1. (digits)-
 * 2. -(digits)
 * 3. (digits)-(digits)
 * 4. (digits)-(digits),(digits)-(digits),..
 * This function also Removes any starting "bytes=" string and change the
 * request into http range format. Specify "0-" for unknown range format.
 ***************************************************************************/
CCUR_PRIVATE(tc_sim_rangetype_e)
_tcSimGetRangeType(
        CHAR*   strCRange,
        U32     nStrCRange,
        CHAR*   strCMisc,
        U32     nStrCMisc,
        BOOL    bIsHttpRgReq)
{
    tc_sim_rangetype_e  _eRgValType;
    _eRgValType = _tcSimGetRangeValType(strCRange,bIsHttpRgReq);
    /* unknown could be the range value is file segments
     * in the case of mpeg2 transport.
     */
    if(tcRangeTypeUnknown == _eRgValType)
    {
        ccur_strlcat(strCMisc,"-",nStrCMisc);
        ccur_strlcat(strCMisc,strCRange,nStrCMisc);
        strCRange[0] = '0';
        strCRange[1] = '-';
        strCRange[2] = '\0';
        _eRgValType = tcRangeTypeDigitsAndDash;
    }

    return _eRgValType;
}

/***************************************************************************
 * function: _tcSimCkValidRgReq
 *
 * description: Check http range and make sure the range value is valid
 * http range for cache key map operation. The range value must at least starts
 * with "(digits)-" and can come from http header or URL.
 ***************************************************************************/
CCUR_PRIVATE(BOOL)
_tcSimCkValidRgReq(
        CHAR*               strCRange,
        U32                 nstrCRange,
        BOOL                bIsHttpRgReq,
        tc_sim_rangetype_e  eRgType)
{
    BOOL                _bIsValid;

    _bIsValid = FALSE;
    if(bIsHttpRgReq)
    {
        if('0' == strCRange[0] &&
           '-' == strCRange[1])
            _bIsValid = TRUE;
    }
    else
    {
        if(tcRangeTypeDigitsToDigits == eRgType ||
           tcRangeTypeDigitsAndDash == eRgType)
            _bIsValid = TRUE;
    }

    return _bIsValid;
}

/***************************************************************************
 * function: _tcSimProcessPktPrcMsg
 *
 * description: The idea is to find unique static id key of video stream.
 * The cache key id is retrieved two ways:
 * 1. cache key map way - cache key is generated through calculating
 * n-bytes of response received for "0-" request for URL range request
 * and "0-*" for http range request. The n-bytes value is specified within
 * edge option variable. For example: options: 'ytb-ckeymap-mp2ts-0_256'
 * ytb     = youtube
 * ckeymap = cache key mapping of dynamic id to static id edge option
 * mp2ts   = mp2 transport stream
 * 0_256   = calculate checksum from 0 to 256 as cache key static id.
 * In order to calculate checksum, we need to issue GET request of
 * 0-256 bytes but not every service allow us to pull only 256 bytes. If valid,
 * Multiple dynamic IDs to 1 static id records will be stored within LRU hash table
 * for period of 60 seconds. If not being being updated within 60 seconds, then
 * the dynamic id will be evicted from hash table. Pulling of HTTP GET will
 * be pushed to sim/snd worker thread, which will use Curlib to pull necessary range.
 * once retrieved, the dynamic id is send back to sim thread and mapping of
 * dynamic id to static id per video stream is done. The next subsequent http request
 * will have dynamic id matching to unique static id.
 * 2. generic cache way - unique cache key of a video is given in URL string.
 * No extra operation needed to get unique static cache key id needed.
 *
 *The requests types are the following:
 *  tcSimReqTypeError       - unknown, error.
 *  tcSimReqTypeCkeyMap,        - GET request to sim/send thread
 *  tcSimReqTypeGetContentLen,       - HEAD request to sim/send thread
 *  tcSimReqTypeLog,        - Log request to logging background thread
 *  tcSimReqTypeMiss,       - Miss request
 *  tcSimReqTypePending     - Pending to see if range value is specified within
 *                            http URL or header.
 *
 *  tcSimReqTypePending means we need to check if we need to do a HEAD request
 *  or we can calculate the request from http URL or header given.
 *
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimProcessPktPrcMsg(tc_sim_thread_ctxt_t * pCntx,
        tc_qmsgtbl_pptosim_t * pQMsg)
{
    tresult_t               _result;
    BOOL                    _bIsCkeyMap;
    CHAR*                   _pStrSId;
    CHAR*                   _pStrDynId;
    tc_sim_ckeymap_t*       _pCkeyMap;
    tc_sim_rangetype_e      _eRgType;
    tc_sim_reqtype_e        _eReqType;
    tc_sim_msgpyldtype_e    _eMsgPyldType;
    CHAR                    _strTmpBuff[TRANSC_SIM_RGBUFF];
    CHAR                    _strHttpContentLen[TRANSC_SIM_CONTLENBUFF];

    CCURASSERT(pCntx);
    CCURASSERT(pQMsg);

    do
    {
        _pCkeyMap               = NULL;
        _pStrSId                = NULL;
        _pStrDynId              = NULL;
        _strHttpContentLen[0]   = '\0';
        _eRgType                = tcRangeTypeUnknown;
        _eReqType               = tcSimReqTypeError;
        pCntx->nHttpReq++;
        /* Fixup "bytes=" string from range, This should only affect
         * http header case but in case bytes is included in URL then
         * remove it for both URL as well. */
        if(!strncasecmp(pQMsg->strCRange,"bytes=",sizeof("bytes=")-1))
        {
            CHAR*   _pStr;
            ccur_strlcpy(_strTmpBuff,
                    pQMsg->strCRange,sizeof(_strTmpBuff));
            /* Remove any '/' if exists, we don't need it */
            _pStr = strchr(_strTmpBuff,'/');
            if(_pStr)
                *_pStr = '\0';
            ccur_strlcpy(pQMsg->strCRange,
                    _strTmpBuff+sizeof("bytes=")-1,
                    sizeof(pQMsg->strCRange));
        }
        /* 1. Get Response payload type */
        if(pQMsg->bIsHttpRgReq)
            _eMsgPyldType = tcSimHttpMsgRespPyldType206;
        else
            _eMsgPyldType = tcSimHttpMsgRespPyldTypeOK;
        /* 2. Figure out cache key mapping is needed or not and
         * Get Range type and Request type */
        _bIsCkeyMap = _tcSimIsCkeyMap(pQMsg->strOptions);
        if(_bIsCkeyMap)
        {
            _pCkeyMap = _tcSimGetPktInfoFromCkeyMapTable(
                                        pCntx,pQMsg->strCId);
            if(NULL == _pCkeyMap)
            {
                _eRgType = _tcSimGetRangeType(
                        pQMsg->strCRange,
                        sizeof(pQMsg->strCRange),
                        pQMsg->strCMisc,
                        sizeof(pQMsg->strCMisc),
                        pQMsg->bIsHttpRgReq);
                if (tcRangeTypeUnknown == _eRgType)
                {
                    _eReqType   = tcSimReqTypeError;
                    break;
                }
                if(FALSE == _tcSimCkValidRgReq(
                        pQMsg->strCRange,
                        sizeof(pQMsg->strCRange),
                        pQMsg->bIsHttpRgReq,
                        _eRgType))
                {
                    _eReqType = tcSimReqTypeMiss;
                    _pStrDynId  = pQMsg->strCId;
                    _pStrSId   = _pStrDynId;
                    break;
                }
                /* Add message into the cache key entry */
                if(ESUCCESS != _tcSimCreateCkeyMapEntry(
                        pCntx,pQMsg))
                {
                    _eReqType   = tcSimReqTypeError;
                    break;
                }
                _eReqType   = tcSimReqTypeCkeyMap;
                _pStrDynId  = pQMsg->strCId;
                _pStrSId    = _pStrDynId;
            }
            else
            {
                if('\0' == _pCkeyMap->tPktInfo.strCId[0])
                {
                    _eReqType = tcSimReqTypeMiss;
                    _pStrDynId  = pQMsg->strCId;
                    _pStrSId   = _pStrDynId;
                    break;
                }
                else
                {
                    _eRgType = _tcSimGetRangeType(
                            pQMsg->strCRange,
                            sizeof(pQMsg->strCRange),
                            pQMsg->strCMisc,
                            sizeof(pQMsg->strCMisc),
                            pQMsg->bIsHttpRgReq);
                    if (tcRangeTypeUnknown == _eRgType)
                    {
                        _eReqType   = tcSimReqTypeError;
                        break;
                    }
                    _eReqType = tcSimUtilGetReqType(_eRgType);
                    _pStrDynId   = pQMsg->strCId;
                    _pStrSId     = _pCkeyMap->tPktInfo.strCId;
                }
            }
        }
        else
        {
            _eRgType = _tcSimGetRangeType(
                    pQMsg->strCRange,
                    sizeof(pQMsg->strCRange),
                    pQMsg->strCMisc,
                    sizeof(pQMsg->strCMisc),
                    pQMsg->bIsHttpRgReq);
            if (tcRangeTypeUnknown == _eRgType)
            {
                _eReqType   = tcSimReqTypeError;
                break;
            }
            _eReqType = tcSimUtilGetReqType(_eRgType);
            _pStrDynId  = pQMsg->strCId;
            _pStrSId    = _pStrDynId;
        }
    }while(FALSE);

    /* 3. Handle request per type specified */
    switch(_eReqType)
    {
        /* Get content length through HEAD request for
         * the case of "digits"- or -"digits" */
        case tcSimReqTypeGetContentLen:
            /* content-length should be '\0' (unknown) */
            pCntx->nHttpHEADReq++;
            _result = _tcSimSendReqToSimsndQ(
                    pCntx,_pStrSId,_pStrDynId,
                    _eReqType,_eRgType,_eMsgPyldType,pQMsg);
            if(ESUCCESS != _result)
                pCntx->nErrNoMemSimToSimSnd++;
            break;
        /* Calculate checksum key and log result once content-length
         * value is known either from one of the following scenario:
         * 1. Range request range value ("digits"-"digits",... case)
         * 2. HEAD request content-length value ("digits"- or -"digits" case) */
        case tcSimReqTypeCkeyMap:
            /* content-length can be '\0' or some value */
            pCntx->nHttpGETReq++;
            _result = _tcSimSendReqToSimsndQ(
                    pCntx,_pStrSId,_pStrDynId,
                    _eReqType,_eRgType,_eMsgPyldType,pQMsg);
            if(ESUCCESS != _result)
                pCntx->nErrNoMemSimToSimSnd++;
            break;
        /* log result by calculating range value
        * in the case of "digits"-"digits" */
        case tcSimReqTypeLog:
            _result = tcSimUtilGetReqContentLen(
                    pQMsg->strCRange,_eRgType,
                    _strHttpContentLen,
                    sizeof(_strHttpContentLen));
            if(ESUCCESS != _result)
                break;
            /* content-length musn't be '\0' */
            if('\0' == _strHttpContentLen[0])
            {
                _result = EFAILURE;
                break;
            }
            _result = _tcSimW3cLog(
                    pCntx,_pStrSId,
                    _strHttpContentLen,
                    _eMsgPyldType,pQMsg);
            break;
        case tcSimReqTypeMiss:
            /* TODO: will queue the miss packets in the future... */
            _result = EFAILURE;
            break;
        case tcSimReqTypeError:
        default:
            _result = EFAILURE;
            break;
    }

    return _result;
}

/***************************************************************************
 * function: _tcSimProcessSimSndMsg
 *
 * description: Process message from sim/send thread, which contains the
 * static and dynamic cache key id.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimProcessSimSndMsg(tc_sim_thread_ctxt_t * pCntx,
        tc_qmsgtbl_simsndtosim_t * pQMsg)
{
    tresult_t               _result;
    tc_sim_ckeymap_t*       _pCkeyMap;
    tc_qmsgtbl_pptosim_t*   _pPktInfo;

    CCURASSERT(pCntx);
    CCURASSERT(pQMsg);

    do
    {
        _result = ESUCCESS;
        _pCkeyMap = _tcSimGetPktInfoFromCkeyMapTable(pCntx,pQMsg->strDynId);
        if(NULL == _pCkeyMap)
            break;
        _pPktInfo = &(_pCkeyMap->tPktInfo);
        ccur_strlcpy(_pPktInfo->strCId,
                pQMsg->strSId,sizeof(_pPktInfo->strCId));
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcReadCfg
 *
 * description: Reload config from background thread when events come in
 * from background thread.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcReadCfg(tc_sim_thread_ctxt_t* pCntx)
{
    tresult_t                _result;
    tc_ldcfg_conf_t*         _pNewConfigYamlCfg;
    tc_shared_cfgmsg_t*     _pConfigYamlCfgDesc;

    _pConfigYamlCfgDesc = tcShDGetCfgYamlDesc();
    if(tcShProtectedGetSigCfgYamlLoadSts(tcTRCompTypeSimMgr))
    {
        pthread_mutex_lock(&(_pConfigYamlCfgDesc->tCfgMutex));
        if(_pConfigYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgConfigYamlSimMgr])
        {
            _pNewConfigYamlCfg    = &(_pConfigYamlCfgDesc->tNewConfig);
            _result = tcSimProcInitLoadableRes(pCntx,_pNewConfigYamlCfg);
            if(ESUCCESS == _result)
            {
                evLogTrace(
                        pCntx->pQSimToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "New config.yaml config loaded successfully by Sim Prc thread TID#%x",pCntx->tid);
            }
            else
            {
                evLogTrace(pCntx->pQSimToBkgrnd,
                        evLogLvlError,
                        &(pCntx->tLogDescSys),
                        "New config.yaml config loading failure by Sim Prc thread TID#%x",pCntx->tid);
            }
            _pConfigYamlCfgDesc->bReloadCompTbl[tcTRCompTypeCfgConfigYamlSimMgr] = FALSE;
        }
        pthread_mutex_unlock(&(_pConfigYamlCfgDesc->tCfgMutex));
        tcShProtectedSetSigCfgYamlLoadSts(tcTRCompTypeSimMgr,TRUE);
    }
}

/***************************************************************************
 * function: _tcSimReadPktPrcQ
 *
 * description: Read messages from Packet processing thread.
 ***************************************************************************/
CCUR_PRIVATE(BOOL)
_tcSimReadPktPrcQ(tc_sim_thread_ctxt_t * pCntx,
        tc_qmsgtbl_pptosim_t * pQMsg)
{
    tc_qmsgtbl_pptosim_t*               _pQMsg;
    BOOL                                _bMsgAvail;

    _pQMsg = (tc_qmsgtbl_pptosim_t*)
            lkfqRead(pCntx->pQHttpProcToSim);
    if(_pQMsg)
    {
        memcpy(pQMsg,_pQMsg,sizeof(tc_qmsgtbl_pptosim_t));
        lkfqReadRelease(
            pCntx->pQHttpProcToSim,(lkfq_data_p)_pQMsg);
        _bMsgAvail = TRUE;
    }
    else
        _bMsgAvail = FALSE;

    return _bMsgAvail;
}

/***************************************************************************
 * function: _tcSimReadSimsendQ
 *
 * description: Read messages from Sim/Send Worker threads
 ***************************************************************************/
CCUR_PRIVATE(BOOL)
_tcSimReadSimsendQ(tc_sim_thread_ctxt_t * pCntx,
        tc_qmsgtbl_simsndtosim_t * pQMsg, U16 nQIdx)
{
    tc_qmsgtbl_simsndtosim_t*           _pQMsg;
    BOOL                                _bMsgAvail;

    pCntx->nQSimSendWToSimWrCnt++;
    _pQMsg = (tc_qmsgtbl_simsndtosim_t*)
            lkfqRead(pCntx->pQSimSendWToSimTbl[nQIdx]);
    if(_pQMsg)
    {
        memcpy(pQMsg,_pQMsg,sizeof(tc_qmsgtbl_simsndtosim_t));
        lkfqReadRelease(
            pCntx->pQSimSendWToSimTbl[nQIdx],(lkfq_data_p)_pQMsg);
        _bMsgAvail = TRUE;
    }
    else
    {
        pCntx->nQSimSendWToSimWrCnt = 0;
        _bMsgAvail = FALSE;
    }
    if(200 == pCntx->nQSimSendWToSimWrCnt)
    {
        pCntx->nQSimSendWToSimWrCnt = 0;
        _bMsgAvail = FALSE;
    }

    return _bMsgAvail;
}

/***************************************************************************
 * function: tcSimHtableTimeoutCheck
 *
 * description: Search for timed out session GET requests within
 * hash table.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcSimHtableTimeoutCheck(tc_sim_thread_ctxt_t * pCntx)
{
    /* TODO: Handle time rollback */
    tc_gd_time_t        _pTimeNow;

    CCURASSERT(pCntx);

    if(pCntx->nCkeyEntryTimeoutCnt >=
            TRANSC_SIMCKEY_TTL_CNT)
    {
        tUtilUTCTimeGet(&_pTimeNow);
        _tcSimHtableFindTimeout(
                pCntx,
                &_pTimeNow,
                TRANSC_SIMCKEY_CTLSRCENTRIES_NUM);
        pCntx->nCkeyEntryTimeoutCnt = 0;
    }
    pCntx->nCkeyEntryTimeoutCnt++;
}

/***************************************************************************
 * function: tcSimCkeyMapMpoolDestroy
 *
 * description: Destroy Cache key Mapping memory pool
 ***************************************************************************/
CCUR_PROTECTED(void)
tcSimCkeyMapMpoolDestroy(tc_sim_thread_ctxt_t * pCntx)
{
    if(pCntx->pCkeyMapMpool)
        cp_mempool_destroy(pCntx->pCkeyMapMpool);
}

/***************************************************************************
 * function: tcSimCkeyMapMpoolCreate
 *
 * description: Create cache key memory pool
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcSimCkeyMapMpoolCreate(tc_sim_thread_ctxt_t * pCntx)
{
    CCURASSERT(pCntx);

    pCntx->pCkeyMapMpool =
            cp_mempool_create_by_option(COLLECTION_MODE_NOSYNC,
                sizeof(tc_sim_ckeymap_t), 10);
    if(!pCntx->pCkeyMapMpool)
	return ENOMEM;
    return ESUCCESS;
}

/***************************************************************************
 * function: tcSimProcInitLoadableRes
 *
 * description: Init sim thread context at runtime based on new configuration
 * from background thread.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcSimProcInitLoadableRes(tc_sim_thread_ctxt_t * pCntx, tc_ldcfg_conf_t * pLdCfg)
{
    U32                         _i;
    tresult_t                   _result;

    _result = ESUCCESS;
    evLogTrace(
          pCntx->pQSimToBkgrnd,
          evLogLvlWarn,
          &(pCntx->tLogDescSys),
          "Warning, Sim Mgr is down, reloading config!");
    if(0 == ccur_strcasecmp(pLdCfg->strCmdArgSimBwSimMode,"true"))
        pCntx->bBwSimMode = TRUE;
    else
        pCntx->bBwSimMode = FALSE;
    evLogTrace(
            pCntx->pQSimToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "Sim Proc thd sim mode active:%s",pLdCfg->strCmdArgSimBwSimMode);
    /* Value not being used by Sim thread. This
     * value should be passed to sim thread
     * and broadcasted to all sim/simsnd threads
     */
    ccur_strlcpy(pCntx->strSimBwOutIntf,
            pLdCfg->strCmdArgSimBwOutIntf,
            sizeof(pCntx->strSimBwOutIntf));
    /* Tell Sim Chldren to load all the configurations */
    for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
        pCntx->bLoadSimSndCfgTbl[_i] = TRUE;
    tcShProtectedDSetCompSts(tcTRCompTypeSimMgr,tcTrStsUp);
    tcShProtectedDSetCompSts(tcTRCompTypeSimMgrChldrn,tcTrStsUp);
    evLogTrace(
          pCntx->pQSimToBkgrnd,
          evLogLvlWarn,
          &(pCntx->tLogDescSys),
          "Warning, Sim Mgr is Up, Finished reloading config!");
    return _result;
}

/***************************************************************************
 * function: _tcSimLogStats
 *
 * description: dump statistics based on time specified.
 ***************************************************************************/
CCUR_PRIVATE(void)
tcSimProcStatsDump(tc_sim_thread_ctxt_t * pCntx, U32 * pnStatsDump,
        tc_gd_time_t * pNowTime, tc_gd_time_t * pOldTime)
{
    tc_gd_time_t                        _tUpdDiff;

    (*pnStatsDump)++;
    if(TRANSC_SIM_DUMPSTATS_CNTR == *pnStatsDump)
    {
        tUtilUTCTimeGet(pNowTime);
        if( pNowTime->nSeconds <= pOldTime->nSeconds)
        {
            _tUpdDiff.nSeconds = 1;
        }
        else
        {
            tUtilUTCTimeDiff(
                    &_tUpdDiff,
                    pNowTime,
                    pOldTime
                    );
        }
        if( _tUpdDiff.nSeconds >= TRANSC_SIM_DUMPSTATS_TIME_SEC )
        {
            *pOldTime = *pNowTime;
            _tcSimLogStats(pCntx);
        }
        *pnStatsDump = 0;
    }
}

/***************************************************************************
 * function: _tcSimProcesReq
 *
 * description: Process requests coming from pkt proc thread and sim/send
 * worker threads through queue. It also sends message to Sim/send worker
 * threads and logging thread.
 * - Pkt proc -> Sim thread : message contains request information.
 * - Sim/Send Worker -> Sim thread : message containing dynamic and static
 * ckey id for Sim thread to keep track.
 * - Sim -> Sim/Send thread : GET or HEAD message request for Sim/Send to
 * handle.
 * - Sim -> Logging thread : message to be logged into file system.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcSimProcesReq(tc_sim_thread_ctxt_t * pCntx)
{
    tresult_t                           _result;
    U16                                 _i;
    tc_gd_time_t                        _tOldTime;
    tc_gd_time_t                        _tNowTime;
    tc_qmsgtbl_pptosim_t                _tQPPtoSimMsg;
    tc_qmsgtbl_simsndtosim_t            _tQSimSndMsgToSimMsg;
    U32                                 _nStatsDump;
    BOOL                                _bQMsg1Avail;
    BOOL                                _bQMsg2Avail;

    CCURASSERT(pCntx);

    _nStatsDump = 0;
    tUtilUTCTimeGet(&_tOldTime);
    _tNowTime = _tOldTime;
    while(!pCntx->bExit)
    {
        /* Read config to see if there are any changes */
        _tcReadCfg(pCntx);

        _bQMsg1Avail =
                _tcSimReadPktPrcQ(pCntx,&_tQPPtoSimMsg);
        if(_bQMsg1Avail)
        {
            _result = _tcSimProcessPktPrcMsg(pCntx,&_tQPPtoSimMsg);
            if(EFAILURE == _result)
                pCntx->nErrCount++;
            else if(EIGNORE == _result)
                pCntx->nIgnCount++;
        }
        tcSimHtableTimeoutCheck(pCntx);
        /* Read from n-th number of queues */
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
        {
            do
            {
                _bQMsg2Avail =
                        _tcSimReadSimsendQ(pCntx,&_tQSimSndMsgToSimMsg,_i);
                if(_bQMsg2Avail)
                {
                    _result = _tcSimProcessSimSndMsg(pCntx,&_tQSimSndMsgToSimMsg);
                    if(EFAILURE == _result)
                        pCntx->nErrCount++;
                    else if(EIGNORE == _result)
                        pCntx->nIgnCount++;
                }
            }while(_bQMsg2Avail);
        }
        if(!_bQMsg1Avail)
        {
            if(pCntx->bBwSimMode)
                usleep(1000);
            else
                sleep(2);
        }
        tcSimProcStatsDump(pCntx,&_nStatsDump,&_tNowTime,&_tOldTime);
    }
}

/***************************************************************************
 * function: tcSimProcThreadEntry
 *
 * description: Entry point sim thread.
 ***************************************************************************/
CCUR_PROTECTED(mthread_result_t)
tcSimProcThreadEntry(void* pthdArg)
{
    tresult_t                           _result;
    time_t                              _tnow;
    tc_sim_thread_ctxt_t*               _pCntx;
    U32                                 _i;
    _pCntx = (tc_sim_thread_ctxt_t*)pthdArg;

    CCURASSERT(_pCntx);

    /****** 1. Thread Common Init ********/
    _tnow = time(0);
    _pCntx->tGMUptime =
            gmtime(&_tnow);
    tUtilUTCTimeGet(&(_pCntx->tUptime));
    _pCntx->tid = (U32)pthread_self();
    /* Sync Queues */
    lkfqSyncQ(&(_pCntx->pQSimToBkgrnd->tLkfq));
    lkfqSyncQ(_pCntx->pQHttpProcToSim);
    for(_i=0;_i<_pCntx->nSimSendWThreadsNum;_i++)
    {
        lkfqSyncQ(_pCntx->pQSimToSimSendWTbl[_i]);
        lkfqSyncQ(_pCntx->pQSimSendWToSimTbl[_i]);
    }
    /****** 2. Thread Resource Init  ********/
    _pCntx->nResCkeyMapBinMax =
            TRANSC_SIMCKEY_DFLTBIN_CNT;
    _result = tcSimCkeyMapMpoolCreate(_pCntx);
    if(ESUCCESS != _result)
    {
        _pCntx->bExit = TRUE;
        evLogTrace(
                _pCntx->pQSimToBkgrnd,
                evLogLvlError,
                &(_pCntx->tLogDescSys),
                "Sim Proc TID#%x Initialization Error, unable to allocate memory!",
                _pCntx->tid);
    }
    /****** 3. Thread Synchronization  ********/
    _tcReadCfg(_pCntx);
    _tcSimLogStats(_pCntx);
    tcShProtectedDMsgSetCompInitRdy(tcTRCompTypeSimMgr,TRUE);
    /****** 4. Thread Process ********/
    /* Block here until signaled by main thread,
     * This will allow other thread, Queue
     * to be initialized. */
    /* Init Sim processing */
    while (!(_pCntx->bExit))
    {
        if (ESUCCESS == mSemCondVarSemWaitTimed(
                      &(_pCntx->tCondVarSem),
                      TRANSC_SIM_WAITTIMEOUT_MS
                      ))
            break;
    }
    _result = ESUCCESS;
    if(FALSE == _pCntx->bExit)
    {
        evLogTrace(
                _pCntx->pQSimToBkgrnd,
                evLogLvlInfo,
                &(_pCntx->tLogDescSys),
                "Sim Proc Thd is running with TID#%x",_pCntx->tid);
        _tcSimProcesReq(_pCntx);
        tUtilUTCTimeGet(&(_pCntx->tDowntime));

        /* Dump temporary stats */
        _tcSimLogStats(_pCntx);
        /* Dump summary stats */
        _tcSimLogStatsSummary(_pCntx);
    }
    tcShProtectedDSetCompSts(tcTRCompTypeSimMgr,tcTrStsDown);
    tcShProtectedDSetCompSts(tcTRCompTypeSimMgrChldrn,tcTrStsDown);
    /****** 5. Thread Resource Destroy ********/
    tcSimCkeyMapMpoolDestroy(_pCntx);
    /****** 6. Exit application ********/
    tcShProtectedDSetAppExitSts(_pCntx->bExit);

    return _result;
}
