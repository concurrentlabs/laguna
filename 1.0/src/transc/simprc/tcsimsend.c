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

#define TRANSC_URL_REWRITE_NTFLX TRUE

/***************************************************************************
 * function: _tcSimSendWriteHeadDataCb
 *
 * description: cURL write call back for HEAD request.
 ***************************************************************************/
CCUR_PRIVATE(size_t)
_tcSimSendWriteHeadDataCb(void *ptr, size_t size, size_t nmemb, void *stream)
{
    I32                     written;
    tc_simutil_curlmsg_t*   pBuff;

    if(stream)
    {
        pBuff = (tc_simutil_curlmsg_t*)stream;
        if((pBuff->nTotalWritten+size*nmemb) < pBuff->nstrContentBuff)
        {
            memcpy(&(pBuff->strContentBuff[pBuff->nTotalWritten]),ptr,size*nmemb);
            pBuff->nTotalWritten += size*nmemb;
        }
    }
    written = size*nmemb;

    return written;
}

/***************************************************************************
 * function: _tcSimSendWriteGetDataCb
 *
 * description: cURL write call back for GET request.
 ***************************************************************************/
CCUR_PRIVATE(size_t)
_tcSimSendWriteGetDataCb(void *ptr, size_t size, size_t nmemb, void *stream)
{
    I32                     written;
    U32                     nSpaceLeft;
    tc_simutil_curlmsg_t*   pBuff;

    written = 0;
    if(stream)
    {
        pBuff = (tc_simutil_curlmsg_t*)stream;
        nSpaceLeft = pBuff->nstrContentBuff-pBuff->nTotalWritten;
        if(nSpaceLeft)
        {
            if(size*nmemb >= nSpaceLeft)
            {
                memcpy(&(pBuff->strContentBuff[pBuff->nTotalWritten]),ptr,nSpaceLeft);
                pBuff->nTotalWritten += nSpaceLeft;
                written = nSpaceLeft;
            }
            else
            {
                memcpy(&(pBuff->strContentBuff[pBuff->nTotalWritten]),ptr,size*nmemb);
                pBuff->nTotalWritten += size*nmemb;
                written = size*nmemb;
            }
        }
#if 0
        if(size*nmemb < 10240)
        {
            memcpy(g_buff,ptr,size*nmemb);
            g_buff[size*nmemb] = '\0';
            printf("wr:%s\n",g_buff);
        }
        else
            printf("error,no space to wrire\n");
#endif
    }

    return written;
}

/***************************************************************************
 * function: _tcSimSendOptRgToHttpRg
 *
 * description: transform cachkey map option value range to http range value.
 * Example:'ytb-ckeymap-mp2ts-0_256', the cache key map range is therefore
 * 0-256 in http notation.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimSendOptRgToHttpRg(
        tc_simsnd_thread_ctxt_t*    pCntx,
        CHAR*                       strHttpRg,
        U32                         nStrHttpRg,
        CHAR*                       strCkeyMapRange)
{
    tresult_t   _result;
    CHAR        _tmpBuff[128];
    CHAR*       _arg;
    CHAR*       _endStr;
    CHAR*       _chr;
    U16         _i;

    CCURASSERT(strHttpRg);
    CCURASSERT(strCkeyMapRange);

    _result = EFAILURE;
    _tmpBuff[0] = '\0';
    ccur_strlcpy(_tmpBuff,strCkeyMapRange,sizeof(_tmpBuff));
    _arg = strtok_r(
            _tmpBuff,"-",&_endStr);
    _i=0;
    while(_arg)
    {
        if(3 == _i)
        {
            _chr = strchr(_arg,'_');
            if(_chr && isdigit(*(_chr+1)))
            {
                ccur_strlcpy(
                        strHttpRg,_arg,nStrHttpRg);
                _chr = strchr(strHttpRg,'_');
                *_chr = '-';
                _result = ESUCCESS;
            }
            else
                break;
        }
        else if (_i > 3)
            break;
        _arg = strtok_r( NULL, "-" ,&_endStr);
        _i++;
    }

    return _result;
}

/***************************************************************************
 * function: _tcSimSendW3cLog
 *
 * description: Construct and send message to logging thread.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimSendW3cLog(
        tc_simsnd_thread_ctxt_t*     pCntx,
        CHAR*                        pStrStaticId,
        CHAR*                        pStrContentLen,
        tc_sim_msgpyldtype_e         eMsgPyldType,
        tc_qmsgtbl_pptosim_t*        pMsg)
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

    CCURASSERT(pMsg);
    do
    {
        _result = EFAILURE;
        if('\0' == pStrContentLen[0] ||
           '\0' == pStrStaticId[0])
            break;

        if('\0' != pMsg->strHostName[0])
            _pHostName = pMsg->strHostName;
        else
        {
            tcUtilIPAddrtoAscii(
                &(pMsg->tDstIP),
                _strDstddr,
                sizeof(_strDstddr));
            _pHostName = _strDstddr;
        }
        tcUtilIPAddrtoAscii(
            &(pMsg->tSrcIP),
            _strSrcddr,
            sizeof(_strSrcddr));
        /*The common logfile format is as follows:
        remotehost rfc931 authuser [date] "request" status bytes */
        time( &_tNowTime );
        _pNowTm = localtime(&(_tNowTime));
        strftime(_strtmbuf, sizeof(_strtmbuf), "%d/%b/%Y:%T %z",_pNowTm);
        _strUrlBuf[0] = '\0';
        _nStrUrl = strlen(pMsg->strUrl);
        if(_nStrUrl < sizeof(_strUrlBuf)-1)
        {
            strncpy(_strUrlBuf,pMsg->strUrl,_nStrUrl);
            _strUrlBuf[_nStrUrl] = '\0';
        }
        else
            ccur_strlcpy(_strUrlBuf,pMsg->strUrl,sizeof(_strUrlBuf)-1);
        if(tcSimHttpMsgRespPyldType206 == eMsgPyldType)
            strcpy(_strRespType,"206");
        else
            strcpy(_strRespType,"200");
        if('\0' == pMsg->strUAgent[0])
        {
            evLogTrace(
                    pCntx->pQSimSendWToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSvc),
                    "TID:%x %s %s - [%s] \"GET %s/ccur/%s/%s/tcshost/%s/tcskey/%s/%s_%s/tcsopt/%s/tcsosig%s HTTP/1.1\" %s %s \"-\" \"-\"\n",
                    pCntx->tid,
                    _strSrcddr,
                    _pHostName,
                    _strtmbuf,
                    pMsg->strRedirTarget,
                    pMsg->strSvcType,
                    pMsg->strSvcName,
                    pMsg->strHostName,
                    pStrStaticId,
                    pMsg->strCRange,
                    pMsg->strCMisc,
                    pMsg->strOptions,
                    _strUrlBuf,
                    _strRespType,
                    pStrContentLen
                    );
        }
        else
        {
            evLogTrace(
                    pCntx->pQSimSendWToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSvc),
                    "TID:%x %s %s - [%s] \"GET %s/ccur/%s/%s/tcshost/%s/tcskey/%s/%s_%s/tcsopt/%s/tcsosig%s HTTP/1.1\" %s %s \"-\" \"%s\"\n",
                    pCntx->tid,
                    _strSrcddr,
                    _pHostName,
                    _strtmbuf,
                    pMsg->strRedirTarget,
                    pMsg->strSvcType,
                    pMsg->strSvcName,
                    pMsg->strHostName,
                    pStrStaticId,
                    pMsg->strCRange,
                    pMsg->strCMisc,
                    pMsg->strOptions,
                    _strUrlBuf,
                    _strRespType,
                    pStrContentLen,
                    pMsg->strUAgent
                    );
        }
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcSimSendLogStatsSummary
 *
 * description: statistic summary dump.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcSimSendLogStatsSummary(
        tc_simsnd_thread_ctxt_t* pCntx)
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
            pCntx->pQSimSendWToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "****Sim/SimSnd TID#%x Summary****\n"
            "Uptime:%s\n"
            "Total Running time:%ld mins\n"
            "Total HTTP processed/sec:%ld\n",
            pCntx->tid,
            _pTNowbuf,
            _tDiffTime.nSeconds/60,
            (pCntx->nHttpSuccessHEADreq+pCntx->nHttpSuccessGETreq)/_nRunTimeSecs
            );
}

/***************************************************************************
 * function: _tcSimSendLogStats
 *
 * description: statistic dump.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcSimSendLogStats(
        tc_simsnd_thread_ctxt_t* pCntx)
{
    CCURASSERT(pCntx);

    if (TRANSC_ZLOGCATEGORY_BITMAP_CK(pCntx->tLogDescSys,evLogLvlInfo))
        return;

    evLogTrace(
            pCntx->pQSimSendWToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "****Sim/SimSnd TID#%x Proc Info****\n"
            "    HTTP Requests Total:%lu\n"
            "       HTTP GET Success:%lu\n"
            "       HTTP HEAD Success:%lu\n"
            "       HTTP Errors:%lu\n"
            "           No Q Memory SimSnd->Sim Error:%lu\n"
            "           Curl 302 resp Error:%lu\n"
            "           Curl Not 200,206 or 302 resp Error:%lu\n"
            "           Curl Processing Error:%lu\n"
            "               Curl Errno:28/Timeout was reached:%lu\n"
            "               Curl Errno:7/Couldn't connect to server:%lu\n"
            "               Curl other Error:%lu\n"
            "           ContentLen parse Error:%lu\n"
            ,
            pCntx->tid,
            pCntx->nHttpReq,
            pCntx->nHttpSuccessGETreq,
            pCntx->nHttpSuccessHEADreq,
            pCntx->nErrCount,
                pCntx->nErrNoQMemSimSndToSim,
                pCntx->tCurlErr.n302RespErr,
                pCntx->tCurlErr.nBadRespErr,
                pCntx->tCurlErr.nCurlReqErr,
                    pCntx->tCurlErr.nCurlEtimeoutErr,
                    pCntx->tCurlErr.nCurlServerConnErr,
                    pCntx->tCurlErr.nCurlOtherErr,
                pCntx->tCurlErr.nBadContentLenParseErr
            );
}

/***************************************************************************
 * function: _tcSimSendHttpSendHEADReq
 *
 * description: Sending HEAD request to origin server using cURL library and
 * log the response report.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimSendHttpSendHEADReq(
        tc_simsnd_thread_ctxt_t*        pCntx,
        tc_qmsgtbl_simtosimsnd_t*       pQMsg)
{
    tresult_t               _result;
    CHAR                    _strContentLen[TRANSC_SIM_CONTLENBUFF];
    U8                      _strHeadContent[TRANSC_SIMCKEY_HTTPBUFF_MAX_SIZE];
    tc_simutil_curlmsg_t    _tCurlHeadBuff;

    /* The entire file */
    _strContentLen[0] = '\0';
    _strHeadContent[0] = '\0';
    _tCurlHeadBuff.strContentBuff    =  _strHeadContent;
    _tCurlHeadBuff.nstrContentBuff   =  sizeof(_strHeadContent)-1;
    _tCurlHeadBuff.nTotalWritten     =  0;
    _result =
            tcSimUtilSendHttpHEADReq(
                    &_tCurlHeadBuff,
                    &(pCntx->tCurlErr),
                    _strContentLen,
                    sizeof(_strContentLen),
                    pCntx->strSimBwOutIntf,
                    &(pQMsg->tMsg),
                    _tcSimSendWriteHeadDataCb);
    if(ESUCCESS == _result)
    {
        if('\0' != _strContentLen[0])
            _result =  _tcSimSendW3cLog(pCntx,
                    pQMsg->tMsg.strCId,_strContentLen,
                    pQMsg->ePyldType,&(pQMsg->tMsg));
        else
            _result = EFAILURE;
    }
    else
    {
        if(('\0' != pQMsg->tMsg.strUrl[0]) &&
           ('\0' != pQMsg->tMsg.strHostName[0]))
        {
            if(_tCurlHeadBuff.nTotalWritten)
            {
                evLogTrace(
                        pCntx->pQSimSendWToBkgrnd,
                        evLogLvlDebug,
                        &(pCntx->tLogDescSys),
                        "Err Desc: HEAD request error\n"
                        "Request: HEAD http://%s%s \nResponse:%s",
                        pQMsg->tMsg.strHostName,
                        pQMsg->tMsg.strUrl,
                        _tCurlHeadBuff.strContentBuff);
            }
            else
            {
                evLogTrace(
                        pCntx->pQSimSendWToBkgrnd,
                        evLogLvlDebug,
                        &(pCntx->tLogDescSys),
                        "Err Desc: HEAD request error\n"
                        "Request: HEAD http://%s%s",
                        pQMsg->tMsg.strHostName,
                        pQMsg->tMsg.strUrl);
            }
        }
    }

    return _result;
}

/***************************************************************************
 * function: _tcSimSendSendReqToSimQ
 *
 * description: Sending message to from Sim/Snd Worker to Simulation
 * thread Queue.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimSendSendReqToSimQ(
        tc_simsnd_thread_ctxt_t*     pCntx,
        CHAR*                        pStrSId,
        CHAR*                        pStrDynId)
{
    tresult_t                               _result;
    tc_qmsgtbl_simsndtosim_t*               _pQMsg;

    _result = ESUCCESS;
    if(pCntx->nMsgOflw <= 0)
    {
        _pQMsg = (tc_qmsgtbl_simsndtosim_t*)
                lkfqMalloc(pCntx->pQSimSendWToSim);
        if(_pQMsg)
        {
            /* Write message */
            ccur_strlcpy(_pQMsg->strDynId,pStrDynId,sizeof(_pQMsg->strDynId));
            ccur_strlcpy(_pQMsg->strSId,pStrSId,sizeof(_pQMsg->strSId));
            lkfqWrite(pCntx->pQSimSendWToSim,(lkfq_data_p)_pQMsg);
        }
        else
        {
            _result = ENOBUFS;
            evLogTrace(
                    pCntx->pQSimSendWToBkgrnd,
                    evLogLvlWarn,
                    &(pCntx->tLogDescSys),
                    "Sim/send thread bpool out of msg buffer, "
                    "stop queing msg for %d iterations",
                    TRANSC_SIMSNDPRC_RDROVFLW_CNT);
            pCntx->nMsgOflw =
                    TRANSC_SIMSNDPRC_RDROVFLW_CNT;
        }
    }
    else
        pCntx->nMsgOflw--;

    return _result;
}

#if TRANSC_URL_REWRITE_NTFLX
/***************************************************************************
 * function: _tcSimSendRewriteURLRange
 *
 * description: Replace old range within the URL with new range value.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimSendRewriteURLRange(
        tc_simsnd_thread_ctxt_t*        pCntx,
        CHAR*                           strUrl,
        U32                             nstrUrl,
        CHAR*                           strUrlRange,
        CHAR*                           strCkeyRg)
{
    tresult_t   _result;
    CHAR*       _startStr;
    CHAR*       _endStr;
    CHAR        _tmpBuff[TRANSC_SIMCKEY_URL_MAX_SIZE];

    CCURASSERT(pCntx);
    CCURASSERT(strCkeyRg);
    CCURASSERT(strUrl);

    _result     = EFAILURE;
    ccur_strlcpy(_tmpBuff,strUrl,sizeof(_tmpBuff));
    _startStr   = _tmpBuff;
    _endStr     = strstr(_tmpBuff, strUrlRange);
    if(_endStr)
    {
        *_endStr = '\0';
        ccur_strlcpy(strUrl,_startStr,nstrUrl);
        ccur_strlcat(strUrl,strCkeyRg,nstrUrl);
        _startStr =_endStr+strlen(strUrlRange);
        ccur_strlcat(strUrl,_startStr,nstrUrl);
        _result = ESUCCESS;
    }

    return _result;
}

/***************************************************************************
 * function: _tcSimSendIsNtflxMp4Container
 *
 * description: check to see if the option value is ntflx-ckeymap-mp4.
 ***************************************************************************/
CCUR_PRIVATE(BOOL)
_tcSimSendIsNtflxMp4Container(
        tc_simsnd_thread_ctxt_t*        pCntx,
        CHAR*                           strOpt,
        U32                             nstrOpt)
{
    BOOL        _bValid;

    CCURASSERT(strOpt);

    _bValid = FALSE;
    if(!ccur_strncasecmp(strOpt,"ntflx-ckeymap-mp4",sizeof("ntflx-ckeymap-mp4")-1))
        _bValid = TRUE;

    return _bValid;
}
#endif /* TRANSC_URL_REWRITE_NTFLX */
#if 0
/***************************************************************************
 * function: _tcSimSendIsMp4Container
 *
 * description: check to see if the option value is mp4.
 ***************************************************************************/
CCUR_PRIVATE(BOOL)
_tcSimSendIsMp4Container(
        tc_simsnd_thread_ctxt_t*        pCntx,
        CHAR*                           strOpt,
        U32                             nstrOpt)
{
    BOOL        _bValid;
    CHAR*       _arg;
    CHAR*       _endStr;
    CHAR        _strOptBuff[128];

    CCURASSERT(strOpt);

    _bValid = FALSE;
    ccur_strlcpy(_strOptBuff,strOpt,sizeof(_strOptBuff));
    if('\0' != _strOptBuff[0])
    {
       _arg = strtok_r(
               _strOptBuff,"-",&_endStr);
       while(_arg)
       {
           if(!strcmp(_arg,"mp4"))
           {
               _bValid = TRUE;
               break;
           }
           _arg = strtok_r( NULL, "-" ,&_endStr);
       }
    }

    return _bValid;
}
#endif

/***************************************************************************
 * function: _tcSimSendHttpGetCkeyMapContentLen
 *
 * description: get actual request content length.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimSendHttpGetCkeyMapContentLen(
       tc_simsnd_thread_ctxt_t*        pCntx,
       tc_qmsgtbl_simtosimsnd_t*       pQMsgC)
{
    tresult_t               _result;
    tc_qmsgtbl_pptosim_t*   _pQMsg;
    CHAR*                   _strRange;
    CHAR                    _strHttpContentLen[TRANSC_SIMSND_CONTLENBUFF];

    _result = EFAILURE;
    _pQMsg = &(pQMsgC->tMsg);
    /* case "digits"- or -"digits" */
    switch(pQMsgC->eRgType)
    {
        case tcRangeTypeDigitsAndDash:
            /* Get request is used to calculate the checksum */
            _result = _tcSimSendHttpSendHEADReq(pCntx,pQMsgC);
            if(ESUCCESS != _result)
            {
                pCntx->nErrCount++;
#ifdef TRANSC_DEBUGX
                evLogTrace(
                        pCntx->pQSimSendWToBkgrnd,
                        evLogLvlDebug,
                        &(pCntx->tLogDescSys),
                        "Curl Err Code:%d/%s",pCntx->tCurlErr.eCurlErrcode,
                        curl_easy_strerror(
                                (CURLcode)(pCntx->tCurlErr.eCurlErrcode)));
#endif
            }
            else
                pCntx->nHttpSuccessHEADreq++;
            break;
        case tcRangeTypeDigitsToDigits:
        case tcRangeTypeMultiPartRanges:
            /* case "digits"-"digits",... */
            if('\0' !=_pQMsg->strCRange[0])
                _strRange = _pQMsg->strCRange;
            else
                _strRange = _pQMsg->strOptions;
            if('\0' == _strRange[0])
            {
                _result = EFAILURE;
                break;
            }
            _result = tcSimUtilGetReqContentLen(
                    _pQMsg->strCRange,
                    pQMsgC->eRgType,
                    _strHttpContentLen,
                    sizeof(_strHttpContentLen));
            if(ESUCCESS == _result)
            {
                /* content-length musn't be '\0' */
                if('\0' != _strHttpContentLen[0])
                {
                    _result = _tcSimSendW3cLog(
                            pCntx, _pQMsg->strCId,
                            _strHttpContentLen,
                            pQMsgC->ePyldType,_pQMsg);
                    if(ESUCCESS != _result)
                        break;
                }
                else
                {
                    _result = EFAILURE;
                    break;
                }
            }
            break;
        default:
            break;
    }

    return _result;
}

/***************************************************************************
 * function: _tcSimSendHttpSendGETReq
 *
 * description: Sending GET request to origin server using cURL library and
 * log the response report.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimSendHttpSendGETReq(
        tc_simsnd_thread_ctxt_t*        pCntx,
        tc_qmsgtbl_simtosimsnd_t*       pQMsgC)
{
    tresult_t               _result;
    U32                     _nBodyPyload;
    U8*                     _pstrBodyPyload;
    U32                     _nCkeyRgLenLowBnd;
    U32                     _nCkeyRgLenUPBnd;
    U8                      _strHeadContent[TRANSC_SIMCKEY_HTTPBUFF_MAX_SIZE];
    CHAR                    _strCkeyRg[64];
    tc_qmsgtbl_pptosim_t*   _pQMsg;
    tc_simutil_curlmsg_t    _tCurlHeadBuff;
    tc_simutil_curlmsg_t    _tCurlBodyBuff;

    CCURASSERT(pQMsgC);

    do
    {
        _pQMsg = &(pQMsgC->tMsg);
        _result      = EFAILURE;
        _strHeadContent[0]      = '\0';
        pCntx->strBodyPyload[0] = '\0';
        _pstrBodyPyload = (U8*)pCntx->strBodyPyload;
        _result = _tcSimSendOptRgToHttpRg(pCntx,
                _strCkeyRg,
                sizeof(_strCkeyRg),
                _pQMsg->strOptions);
        if(ESUCCESS != _result)
            break;
#if TRANSC_URL_REWRITE_NTFLX
        /* Rewrite URL only when Range is located in URL */
        if(FALSE == _pQMsg->bIsHttpRgReq)
        {
            /* Hack solution, prefer way is to know whether we
             * can replace the range or not. In case of HLS mp2ts streams
             * we can't.
             */
#if 0
            if(_tcSimSendIsMp4Container(pCntx,
                    _pQMsg->strOptions,sizeof(_pQMsg->strOptions)))
#else
                if(_tcSimSendIsNtflxMp4Container(pCntx,
                        _pQMsg->strOptions,sizeof(_pQMsg->strOptions)))
#endif
            {
                _result = _tcSimSendRewriteURLRange(pCntx,
                        _pQMsg->strUrl,sizeof(_pQMsg->strUrl),
                        _pQMsg->strCRange,_strCkeyRg);
                if(ESUCCESS != _result)
                    break;
            }
        }
#endif /* TRANSC_URL_REWRITE_NTFLX */
        _tCurlHeadBuff.strContentBuff    =  _strHeadContent;
        _tCurlHeadBuff.nstrContentBuff   =  sizeof(_strHeadContent)-1;
        _tCurlHeadBuff.nTotalWritten     =  0;
        _tCurlBodyBuff.strContentBuff    =  _pstrBodyPyload;
        _tCurlBodyBuff.nstrContentBuff   =  sizeof(pCntx->strBodyPyload)-1;
        _tCurlBodyBuff.nTotalWritten     =  0;
        /* Get HTTP Body Payload and Body length */
        _nBodyPyload =
                tcSimUtilSendHttpGETReq(
                        &_tCurlHeadBuff,
                        &_tCurlBodyBuff,
                        &(pCntx->tCurlErr),
                        pCntx->strSimBwOutIntf,
                        _pQMsg,
                        _strCkeyRg,
                        _tcSimSendWriteGetDataCb);
        if(0 == _nBodyPyload)
        {
            if(('\0' != _pQMsg->strUrl[0]) &&
               ('\0' != _pQMsg->strHostName[0]))
            {
                if(_tCurlHeadBuff.nTotalWritten)
                {
                    evLogTrace(
                            pCntx->pQSimSendWToBkgrnd,
                            evLogLvlDebug,
                            &(pCntx->tLogDescSys),
                            "Err Desc: GET request error\n"
                            "Request: GET Range: %s | http://%s%s\nResponse:%s",
                            _strCkeyRg,
                            _pQMsg->strHostName,
                            _pQMsg->strUrl,
                            _tCurlHeadBuff.strContentBuff);
                }
                else
                {
                    evLogTrace(
                            pCntx->pQSimSendWToBkgrnd,
                            evLogLvlDebug,
                            &(pCntx->tLogDescSys),
                            "Err Desc: GET request error\n"
                            "Request: GET Range: %s | http://%s%s",
                            _strCkeyRg,
                            _pQMsg->strHostName,
                            _pQMsg->strUrl);
                }
            }
            _result = EFAILURE;
            break;
        }
        /* Get ckey specified range of what need to be checksum */
        _result = tcSimUtilStrRangeToU32Ranges(
                &_nCkeyRgLenLowBnd,&_nCkeyRgLenUPBnd,
                _strCkeyRg);
        if(ESUCCESS != _result)
            break;
        /* Makesure ckey option value is within content length body */
        if(_nCkeyRgLenLowBnd >= _nCkeyRgLenUPBnd)
        {
            _result = EFAILURE;
            break;
        }
        if ((_nCkeyRgLenLowBnd > _nBodyPyload) ||
            (_nCkeyRgLenUPBnd > _nBodyPyload))
        {
            _result = EFAILURE;
            break;
        }
        /* Calculate cache key based on ckey option value range */
        _result =
                tcSimUtilCalculateCkSum(
                    _pQMsg->strCId,sizeof(_pQMsg->strCId),
                    _pstrBodyPyload,_nCkeyRgLenUPBnd-_nCkeyRgLenLowBnd);
        if(ESUCCESS != _result)
            break;
        _result = _tcSimSendSendReqToSimQ(pCntx,
                    _pQMsg->strCId,pQMsgC->strDynId);
        if(ESUCCESS != _result)
        {
            pCntx->nErrNoQMemSimSndToSim++;
            break;
        }
        _result = ESUCCESS;
    }while(FALSE);

    if(ESUCCESS == _result)
    {
        _result =
                _tcSimSendHttpGetCkeyMapContentLen(
                        pCntx,pQMsgC);
    }

    return _result;
}

/***************************************************************************
 * function: _tcReadCfg
 *
 * description: Reload config from background thread when events come in
 * from background thread.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcReadCfg(tc_simsnd_thread_ctxt_t* pCntx)
{
    tresult_t                _result;
    tc_ldcfg_conf_t*         _pNewConfigYamlCfg;
    tc_shared_cfgmsg_t*     _pConfigYamlCfgDesc;

    _pConfigYamlCfgDesc = tcShDGetCfgYamlDesc();
    if(pCntx->bLdCfgYaml)
    {
        pthread_mutex_lock(&(_pConfigYamlCfgDesc->tCfgMutex));
        if(_pConfigYamlCfgDesc->bReloadCompTblSimSndTbl[pCntx->cfgId])
        {
            _pNewConfigYamlCfg    = &(_pConfigYamlCfgDesc->tNewConfig);
            _result = tcSimSendProcInitLoadableRes(pCntx,_pNewConfigYamlCfg);
            if(ESUCCESS == _result)
            {
                evLogTrace(
                        pCntx->pQSimSendWToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "New config.yaml config loaded successfully by Sim/Send Prc %lu thread TID#%x",
                        pCntx->cfgId,pCntx->tid);
            }
            else
            {
                evLogTrace(pCntx->pQSimSendWToBkgrnd,
                        evLogLvlError,
                        &(pCntx->tLogDescSys),
                       "New config.yaml config loading failure by Sim/Send Prc %lu thread TID#%x",
                       pCntx->cfgId,pCntx->tid);
            }
            _pConfigYamlCfgDesc->bReloadCompTblSimSndTbl[pCntx->cfgId] = FALSE;
        }
        pthread_mutex_unlock(&(_pConfigYamlCfgDesc->tCfgMutex));
    }
}

/***************************************************************************
 * function: _tcSimSendReadSimQ
 *
 * description: Read Queeu message from Sim -> Sim/Send Worker thread.
 ***************************************************************************/
CCUR_PRIVATE(BOOL)
_tcSimSendReadSimQ(
        tc_simsnd_thread_ctxt_t*        pCntx,
        tc_qmsgtbl_simtosimsnd_t*       pQMsg)
{
    tc_qmsgtbl_simtosimsnd_t*           _pQMsg;
    BOOL                                _bMsgAvail;

    CCURASSERT(pCntx);

    _pQMsg = (tc_qmsgtbl_simtosimsnd_t*)
            lkfqRead(pCntx->pQSimToSimSendW);
    if(_pQMsg)
    {
        /* Perform other copy operations here ... */
        /* Write message */
        /* Whole lot of copying...*/
        /*memcpy(pQMsg,_pQMsg,sizeof(tc_qmsgtbl_simtosimsnd_t));*/
        ccur_strlcpy(pQMsg->strDynId,
                _pQMsg->strDynId,sizeof(pQMsg->strDynId));
        pQMsg->eReqType = _pQMsg->eReqType;
        pQMsg->eRgType = _pQMsg->eRgType;
        pQMsg->ePyldType = _pQMsg->ePyldType;
        ccur_strlcpy(pQMsg->tMsg.strCId,
                _pQMsg->tMsg.strCId,
                sizeof(pQMsg->tMsg.strCId));
        ccur_strlcpy(pQMsg->tMsg.strUrl,
                _pQMsg->tMsg.strUrl,
                sizeof(pQMsg->tMsg.strUrl));
        ccur_strlcpy(pQMsg->tMsg.strCRange,
                _pQMsg->tMsg.strCRange,
                sizeof(pQMsg->tMsg.strCRange));
        pQMsg->tMsg.bIsHttpRgReq = _pQMsg->tMsg.bIsHttpRgReq;
        ccur_strlcpy(pQMsg->tMsg.strCMisc,
                _pQMsg->tMsg.strCMisc,
                sizeof(pQMsg->tMsg.strCMisc));
        pQMsg->tMsg.tSrcIP = _pQMsg->tMsg.tSrcIP;
        pQMsg->tMsg.tDstIP = _pQMsg->tMsg.tDstIP;
        pQMsg->tMsg.nSrcPort = _pQMsg->tMsg.nSrcPort;
        pQMsg->tMsg.nDstPort = _pQMsg->tMsg.nDstPort;
        ccur_strlcpy(pQMsg->tMsg.strUAgent,
                _pQMsg->tMsg.strUAgent,
                sizeof(pQMsg->tMsg.strUAgent));
        ccur_strlcpy(pQMsg->tMsg.strHostName,
                _pQMsg->tMsg.strHostName,
                sizeof(pQMsg->tMsg.strHostName));
        ccur_strlcpy(pQMsg->tMsg.strRedirTarget,
                _pQMsg->tMsg.strRedirTarget,
                sizeof(pQMsg->tMsg.strRedirTarget));
        /* Service type */
        ccur_strlcpy(pQMsg->tMsg.strSvcType,
                _pQMsg->tMsg.strSvcType,
                sizeof(pQMsg->tMsg.strSvcType));
        /* Service name */
        ccur_strlcpy(pQMsg->tMsg.strSvcName,
                _pQMsg->tMsg.strSvcName,
                sizeof(pQMsg->tMsg.strSvcName));
        ccur_strlcpy(pQMsg->tMsg.strOptions,
                _pQMsg->tMsg.strOptions,
                sizeof(pQMsg->tMsg.strOptions));
        pQMsg->bLdCfgYaml = _pQMsg->bLdCfgYaml;
        lkfqReadRelease(
            pCntx->pQSimToSimSendW,(lkfq_data_p)_pQMsg);
        _bMsgAvail = TRUE;
    }
    else
        _bMsgAvail = FALSE;

    return _bMsgAvail;
}

/***************************************************************************
 * function: _tcSimSendLogStats
 *
 * description: dump statistics based on time specified.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcSimSendProcStatsDump(
        tc_simsnd_thread_ctxt_t*    pCntx,
        U32*                        pnStatsDump,
        tc_gd_time_t*               pNowTime,
        tc_gd_time_t*               pOldTime)
{
    tc_gd_time_t                        _tUpdDiff;

    (*pnStatsDump)++;
    if(TRANSC_SIM_WKRDUMPSTATS_CNTR == *pnStatsDump)
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
        if( _tUpdDiff.nSeconds >= TRANSC_SIM_WKRDUMPSTATS_TIME_SEC )
        {
            *pOldTime = *pNowTime;
            _tcSimSendLogStats(pCntx);
        }
        *pnStatsDump = 0;
    }
}

/***************************************************************************
 * function: _tcSimSendProcesReq
 *
 * description: Process Request from sim thread. The requests can be
 * the following: GET or HEAD request to the origin server.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcSimSendProcesReq(
        tc_simsnd_thread_ctxt_t*   pCntx)
{
    tresult_t                           _result;
    tc_gd_time_t                        _tOldTime;
    tc_gd_time_t                        _tNowTime;
    tc_qmsgtbl_simtosimsnd_t            _tQMsg;
    U32                                 _nStatsDump;
    BOOL                                _bQMsgAvail;

    CCURASSERT(pCntx);

    _nStatsDump = 0;
    tUtilUTCTimeGet(&_tOldTime);
    _tNowTime = _tOldTime;
    while(!pCntx->bExit)
    {
        /* Read config to see if there are any changes */
        _tcReadCfg(pCntx);

        _bQMsgAvail =
                _tcSimSendReadSimQ(pCntx,&_tQMsg);
        if(_bQMsgAvail)
        {
            pCntx->bLdCfgYaml = _tQMsg.bLdCfgYaml;
            pCntx->nHttpReq++;
            switch(_tQMsg.eReqType)
            {
                case tcSimReqTypeCkeyMap:
                    /* Get request is used to calculate the checksum */
                    _result = _tcSimSendHttpSendGETReq(pCntx,&_tQMsg);
                    if(ESUCCESS != _result)
                    {
                        pCntx->nErrCount++;
#ifdef TRANSC_DEBUGX
                        evLogTrace(
                                pCntx->pQSimSendWToBkgrnd,
                                evLogLvlDebug,
                                &(pCntx->tLogDescSys),
                                "Curl Err Code:%d/%s",pCntx->tCurlErr.eCurlErrcode,
                                curl_easy_strerror(
                                        (CURLcode)(pCntx->tCurlErr.eCurlErrcode)));
#endif
                    }
                    else
                        pCntx->nHttpSuccessGETreq++;
                    break;
                case tcSimReqTypeGetContentLen:
                    _result = _tcSimSendHttpSendHEADReq(pCntx,&_tQMsg);
                    if(ESUCCESS != _result)
                    {
                        pCntx->nErrCount++;
#ifdef TRANSC_DEBUGX
                        evLogTrace(
                                pCntx->pQSimSendWToBkgrnd,
                                evLogLvlDebug,
                                &(pCntx->tLogDescSys),
                                "Curl Err Code:%d/%s",pCntx->tCurlErr.eCurlErrcode,
                                curl_easy_strerror(
                                        (CURLcode)(pCntx->tCurlErr.eCurlErrcode)));
#endif
                    }
                    else
                        pCntx->nHttpSuccessHEADreq++;
                    break;
                default:
                    pCntx->nErrCount++;
                    break;
            }
        }
        else
        {
            if(pCntx->bBwSimMode)
                usleep(1000);
            else
                sleep(2);
        }
        _tcSimSendProcStatsDump(pCntx,&_nStatsDump,&_tNowTime,&_tOldTime);
    }
}

/***************************************************************************
 * function: tcSimProcInitLoadableRes
 *
 * description: Init sim/snd thread context at runtime based on new
 * configuration from background thread.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcSimSendProcInitLoadableRes(
        tc_simsnd_thread_ctxt_t*     pCntx,
        tc_ldcfg_conf_t*             pLdCfg)
{
    tresult_t                   _result;

    _result = ESUCCESS;
    evLogTrace(
          pCntx->pQSimSendWToBkgrnd,
          evLogLvlWarn,
          &(pCntx->tLogDescSys),
          "Warning, Sim Chlrn#%lu is down, reloading config!",pCntx->tid);
    if( 0 == ccur_strcasecmp(pLdCfg->strCmdArgSimBwSimMode,"true"))
        pCntx->bBwSimMode = TRUE;
    else
        pCntx->bBwSimMode = FALSE;
    evLogTrace(
            pCntx->pQSimSendWToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "Sim/Send Proc thd mode active:%s",pLdCfg->strCmdArgSimBwSimMode);
    ccur_strlcpy(pCntx->strSimBwOutIntf,
            pLdCfg->strCmdArgSimBwOutIntf,
            sizeof(pCntx->strSimBwOutIntf));
    if('\0' != pCntx->strSimBwOutIntf[0])
        evLogTrace(
                pCntx->pQSimSendWToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "Sim/Send Proc thd outgoing interface:%s",pCntx->strSimBwOutIntf);
    else
        evLogTrace(
                pCntx->pQSimSendWToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "Sim/Send Proc thd outgoing interface: routing table based");

    evLogTrace(
          pCntx->pQSimSendWToBkgrnd,
          evLogLvlWarn,
          &(pCntx->tLogDescSys),
          "Warning, Sim Chlrn#%lu is Up!, Finished reloading config!",pCntx->tid);

    return _result;
}

/***************************************************************************
 * function: tcSimSendProcThreadEntry
 *
 * description: Entry point Sim/Send Worker thread.
 ***************************************************************************/
CCUR_PROTECTED(mthread_result_t)
tcSimSendProcThreadEntry(void* pthdArg)
{
    tresult_t                           _result;
    time_t                              _tnow;
    tc_simsnd_thread_ctxt_t*            _pCntx;
    _pCntx = (tc_simsnd_thread_ctxt_t*)pthdArg;

    CCURASSERT(_pCntx);

    /****** 1. Thread Common Init ********/
    _tnow = time(0);
    _pCntx->tGMUptime =
            gmtime(&_tnow);
    tUtilUTCTimeGet(&(_pCntx->tUptime));
    _pCntx->tid = (U32)pthread_self();
    /* Sync Queues */
    lkfqSyncQ(&(_pCntx->pQSimSendWToBkgrnd->tLkfq));
    lkfqSyncQ(_pCntx->pQSimSendWToSim);
    lkfqSyncQ(_pCntx->pQSimToSimSendW);
    /****** 2. Thread Resource Init  ********/
    /* ... */
    /****** 3. Thread Synchronization  ********/
    _tcSimSendLogStats(_pCntx);
    tcShProtectedDMsgSetCompInitRdy(
            tcTRCompTypeSimMgrChldrn,TRUE);
    /****** 4. Thread Process ********/
    /* Block here until signaled by main thread,
     * This will allow other thread, Queue
     * to be initialized. */
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
                _pCntx->pQSimSendWToBkgrnd,
                evLogLvlInfo,
                &(_pCntx->tLogDescSys),
                "Sim/simsnd Proc Thd is running with TID#%x",_pCntx->tid);
        _tcSimSendProcesReq(_pCntx);
        tUtilUTCTimeGet(&(_pCntx->tDowntime));
        /* Dump temporary stats */
        _tcSimSendLogStats(_pCntx);
        /* Dump summary stats */
        _tcSimSendLogStatsSummary(_pCntx);
    }
    /****** 5. Thread Resource Destroy ********/
    /* ... */
    /****** 6. Exit application ********/
    tcShProtectedDSetAppExitSts(_pCntx->bExit);

    return _result;
}

