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
 * function: _tcSimUtilSendHttpReq
 *
 * description: cURL send http request to origin server. This function blocks
 * until the response arrives. It uses default libcurl settings except
 * connection timeout settings.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimUtilSendHttpReq(
        tc_simutil_curlerr_t*   pErrCntr,
        tc_simutil_curlmsg_t*   pCurlHeadBuff,
        tc_simutil_curlmsg_t*   pCurlBodyBuff,
        CHAR*                   strOutIntf,
        tc_qmsgtbl_pptosim_t*   pMsg,
        CHAR*                   strOptCkeyRg,
        void*                   fCallback)
{
    tresult_t           _result;
    CURLcode            _code;
    CURL*               _pCurlHandle;
    CHAR                _strIpAddr[64];
    CHAR                _strTmpBuff[TRANSC_SIMCKEY_URL_MAX_SIZE];
    struct curl_slist*  _pChunk = NULL;

    CCURASSERT(pErrCntr);
    CCURASSERT(pCurlHeadBuff);
    CCURASSERT(strOutIntf);
    CCURASSERT(pMsg);
    CCURASSERT(fCallback);

    do
    {
        _result = EFAILURE;
        /*curl_global_init(CURL_GLOBAL_ALL);*/

        /* init the curl session */
        _pCurlHandle = curl_easy_init();
        if(NULL == _pCurlHandle)
            break;
        if(strOptCkeyRg)
        {
            if('\0' != strOptCkeyRg[0])
            {
                snprintf(_strTmpBuff,sizeof(_strTmpBuff),
                            "Range: bytes=%s",strOptCkeyRg);
                _pChunk = curl_slist_append(_pChunk, _strTmpBuff);
                if(NULL == _pChunk)
                    break;
            }
            else
                break;
        }
        snprintf(_strTmpBuff,sizeof(_strTmpBuff),
                "User-Agent: %s",pMsg->strUAgent);
        _pChunk = curl_slist_append(_pChunk, _strTmpBuff);
        if(NULL == _pChunk)
            break;
        _pChunk = curl_slist_append(_pChunk, "Connection: close");
        if(NULL == _pChunk)
            break;
        if(TCUTIL_IS_URLABSPATH(pMsg->strUrl))
        {
            snprintf(_strTmpBuff,sizeof(_strTmpBuff),
                    "http://%s%s",pMsg->strHostName,pMsg->strUrl);
            curl_easy_setopt(_pCurlHandle, CURLOPT_URL, _strTmpBuff);
        }
        else
        {
            if(TCUTIL_IS_URLHTTPSTRING(pMsg->strUrl))
            {
                _strIpAddr[0] = '\0';
                tcUtilIPAddrtoAscii(&(pMsg->tDstIP),_strIpAddr,sizeof(_strIpAddr));
                if('\0' != _strIpAddr[0])
                {
                    snprintf(_strTmpBuff,sizeof(_strTmpBuff),
                            "%s:%d",_strIpAddr,pMsg->nDstPort);
                    curl_easy_setopt(_pCurlHandle, CURLOPT_PROXY,_strTmpBuff);
                    snprintf(_strTmpBuff,sizeof(_strTmpBuff),"%s",pMsg->strUrl);
                    curl_easy_setopt(_pCurlHandle, CURLOPT_URL, _strTmpBuff);
                }
                else
                    break;
            }
            else
                break;
        }

        /* Set specifying your preferred size (in bytes)
         * for the receive buffer in libcurl.*/
        curl_easy_setopt(_pCurlHandle, CURLOPT_BUFFERSIZE,
                         TRANSC_LDCFG_BWSIM_BODYPYLDLEN);

        /* No signal and connection timeout */
        curl_easy_setopt(_pCurlHandle, CURLOPT_NOSIGNAL,1L);
        curl_easy_setopt(_pCurlHandle, CURLOPT_CONNECTTIMEOUT_MS,
                TRANSC_SIM_CURLTIMEOUT_MS);

        if(strOutIntf && '\0' != strOutIntf[0])
            curl_easy_setopt(_pCurlHandle, CURLOPT_INTERFACE, strOutIntf);

        /* no progress meter please */
        curl_easy_setopt(_pCurlHandle, CURLOPT_NOPROGRESS, 1L);
#ifdef TRANSC_DEBUG
        curl_easy_setopt(_pCurlHandle, CURLOPT_VERBOSE, 1L);
#endif
        if(NULL == pCurlBodyBuff)
          curl_easy_setopt(_pCurlHandle, CURLOPT_NOBODY, 1L);

        curl_easy_setopt(_pCurlHandle, CURLOPT_HTTPHEADER, _pChunk);

        /* send all data to this function  */
        curl_easy_setopt(_pCurlHandle, CURLOPT_WRITEFUNCTION, fCallback);

        /* we want the headers be written to this file handle */
        curl_easy_setopt(_pCurlHandle, CURLOPT_HEADERDATA, pCurlHeadBuff);

        /* we want the body be written to this memory handle instead of stdout */
        if(pCurlBodyBuff)
            curl_easy_setopt(_pCurlHandle, CURLOPT_WRITEDATA, pCurlBodyBuff);

        /* get it! */
        _code = curl_easy_perform(_pCurlHandle);
        if(_code)
        {
            pErrCntr->eCurlErrcode = _code;
            switch(_code)
            {
               case CURLE_OPERATION_TIMEDOUT:
                   pErrCntr->nCurlEtimeoutErr++;
                   break;
               case CURLE_COULDNT_CONNECT:
                   pErrCntr->nCurlServerConnErr++;
                   break;
               default:
                   if(pCurlBodyBuff)
                   {
                       if(pCurlBodyBuff->nTotalWritten <
                               pCurlBodyBuff->nstrContentBuff-1)
                           pErrCntr->nCurlOtherErr++;
                   }
                   else
                       pErrCntr->nCurlOtherErr++;
                   break;
            }
            break;
        }
        _result = ESUCCESS;
    }while(FALSE);

    if(_pChunk)
        curl_slist_free_all(_pChunk);

    /* cleanup curl stuff */
    if(_pCurlHandle)
        curl_easy_cleanup(_pCurlHandle);

    return _result;

}

/***************************************************************************
 * function: tcSimUtilCleanContentLength
 *
 * description: clean content length string by removing any unecessary
 * trailing and leading strings.
 ***************************************************************************/
CCUR_PRIVATE(U32)
tcSimUtilCleanContentLength(
        CHAR*   strContentLen,
        U32     nStrContentLen)
{
    tresult_t   _result;
    CHAR*       _strS;
    BOOL        _bWr;
    BOOL        _bPrsAfterSlash;
    U32         _nWr;
    U16         _i;
    U32         _len;
    CHAR        _strContentLen[TRANSC_SIM_CONTLENBUFF];
    do
    {
        _result = EFAILURE;
        _len  = strlen(strContentLen);
        if(_len > nStrContentLen ||
          (sizeof("bytes=") > sizeof(_strContentLen)))
            break;
        /* bytes= */
        if(!strncasecmp(strContentLen,
                "bytes=",sizeof("bytes=")-1))
            ccur_strlcpy(_strContentLen+sizeof("bytes=")-1,
                         strContentLen,
                         sizeof(_strContentLen)-sizeof("bytes=")+1);
        else
            ccur_strlcpy(_strContentLen,
                    strContentLen,sizeof(_strContentLen));
        strContentLen[0] = '\0';
        _nWr = 0;
        _bPrsAfterSlash = FALSE;
        _strS = _strContentLen;
        for(_i=0;_i<_len;_i++)
        {
            /* Attempt to copy everything
             * that is (digits) and -
             */
            _bWr = FALSE;
            if((isdigit(*_strS)) ||
               ('-'  ==  *_strS))
            {
                if('-' == *_strS && _i > 0)
                {
                    /* Error when not (digits) after
                     * or before '-' */
                    if(!isdigit(_strS[_i-1]) ||
                       !isdigit(_strS[_i+1]))
                    {
                        _result = EFAILURE;
                        break;
                    }
                    else
                        _bWr = TRUE;
                }
                else
                    _bWr = TRUE;
                if(_bWr)
                {
                    if(_nWr >= nStrContentLen)
                    {
                        _result = EFAILURE;
                        break;
                    }
                    strContentLen[_i] = *_strS;
                    _nWr++;
                    _result = ESUCCESS;
                }
            }
            else if('*' == *_strS)
            {
                _bPrsAfterSlash = TRUE;
                break;
            }
            else if('/' == *_strS)
            {
                _bPrsAfterSlash = TRUE;
                break;
            }
            _strS++;
        }
        if(ESUCCESS != _result)
            break;
        if(_bPrsAfterSlash)
        {
            for(_i=0;_i<_len;_i++)
            {
                if(isdigit(*_strS))
                {
                    if(_nWr  >= nStrContentLen)
                    {
                        _result = EFAILURE;
                        break;
                    }
                    strContentLen[_i] = *_strS;
                    _nWr++;
                    _result = ESUCCESS;
                }
                _strS++;
            }
        }
    }while(FALSE);

    if(ESUCCESS != _result)
    {
        strContentLen[0] = '\0';
        _nWr = 0;
    }
    else
    {
        if(_nWr+1 < nStrContentLen)
            strContentLen[_nWr+1] = '\0';
    }

    return _nWr;
}

/***************************************************************************
 * function: _tcSimUtilGetHttpHeaderContentLen
 *
 * description: Get http header content length information.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcSimUtilGetHttpHeaderContentLen(
        CHAR*                   strContentLen,
        U32                     nStrContentLen,
        const U8*               strContentBuff,
        U32                     nTotalWritten)
{
    tresult_t         _result;
    const CHAR*       _p;
    U32               _i;
    U32               _len;

    _result = EFAILURE;
    strContentLen[0] = '\0';
    /* check for Minimum http header packet + content length */
    if(nTotalWritten > sizeof("GET / HTTP/1.0\r\n\r\n")+
                       sizeof("Content-Length:"))
    {
        _p  = (CHAR*)strContentBuff;
        for(_i=0; _i < nTotalWritten-sizeof("\r\n"); _i++)
        {
            if (_p[_i] == '\r' && _p[_i+1] == '\n')
            {
                _i+=2;
                if(!ccur_strncasecmp(&(_p[_i]),"Content-Length:",sizeof("Content-Length:")-1))
                {
                    /* Copy the Content-Length all the way till \r or \n. */
                    _len = 0;
                    _i+=sizeof("Content-Length:");
                   while((_p[_i] != '\r' && _p[_i] != '\n') &&
                         (_len < nStrContentLen-1))
                   {
                       strContentLen[_len] = _p[_i];
                       _i++;
                       _len++;
                   }
                   strContentLen[_len] = '\0';
                   break;
                }
            }
        }
        _result = ESUCCESS;
    }
    return _result;
}

/***************************************************************************
 * function: tcSimUtilGetReqType
 *
 * description: Get request type of 4 possibilities:
 *               * pQMsg->strCRange:
 *               * 1. (digits)-
 *               * 2. -(digits)
 *               * 3. (digits)-(digits)
 *               * 4. (digits)-(digits),(digits)-(digits),...
 ***************************************************************************/
CCUR_PROTECTED(tc_sim_reqtype_e)
tcSimUtilGetReqType(
        tc_sim_rangetype_e     eRgValType)
{

    tc_sim_reqtype_e        _eReqType;
    switch(eRgValType)
    {
        case tcRangeTypeDigitsAndDash:
            /* case1&2: "- and (digits)" possibility. */
            /* "- and (digits)" means we need to look at the head request */
            _eReqType        = tcSimReqTypeGetContentLen;
            break;
        case tcRangeTypeDigitsToDigits:
            /* case3: "(digits)-digits" possibility. */
            /* "(digits)-(digits)" means we dont need to look at head request */
            _eReqType       = tcSimReqTypeLog;
            break;
        case tcRangeTypeMultiPartRanges:
            /* case4: (digits)-(digits),(digits)-(digits),... */
            /* "(digits)-(digits),..." means we dont need to look at head request */
            _eReqType       = tcSimReqTypeLog;
            break;
        default:
            _eReqType       = tcSimReqTypeError;
            break;
    }

    return _eReqType;
}


/***************************************************************************
 * function: tcSimUtilGetReqContentLen
 *
 * description: get the content length based on range value type.
 *
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcSimUtilGetReqContentLen(
        CHAR*                   strCRange,
        tc_sim_rangetype_e      eRgValType,
        CHAR*                   pStrContentLen,
        U32                     nStrContentLen)
{
    tresult_t   _result = ESUCCESS;
    /* There are only four range possibilities for
                 * pQMsg->strCRange:
                 * 1. (digits)-
                 * 2. -(digits)
                 * 3. (digits)-(digits)
                 * 4. (digits)-(digits),(digits)-(digits),...
                 */
    switch(eRgValType)
    {
        case tcRangeTypeDigitsAndDash:
            /* case1&2: "- and (digits)" possibility. */
            /* "- and (digits)" means we need to look at the head request */
            pStrContentLen[0] = '\0';
            break;
        case tcRangeTypeDigitsToDigits:
            /* case3: "(digits)-digits" possibility. */
            /* "(digits)-(digits)" means we dont need to look at head request */
            tcSimUtilRangeToContentLen(
                    pStrContentLen,nStrContentLen,
                    strCRange);
            if('\0' == pStrContentLen[0])
                break;
            break;
        case tcRangeTypeMultiPartRanges:
            /* case4: (digits)-(digits),(digits)-(digits),... */
            /* "(digits)-(digits),..." means we dont need to look at head request */
            tcSimUtilMultiPartRangeToContentLen(
                    pStrContentLen,nStrContentLen,
                    strCRange);
            if('\0' == pStrContentLen[0])
                break;
            break;
        default:
            _result = EFAILURE;
            break;
    }

    return _result;
}

/***************************************************************************
 * function: tcSimUtilStrRangeToU32Ranges
 *
 * description: Convert HTTP  String Range to unsigned integer range
 * in lower and upper boundaries.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcSimUtilStrRangeToU32Ranges(
        U32*    pRgLenLowBnd,
        U32*    RgLenUPBnd,
        CHAR*   strRange)
{
    tresult_t   _result;
    BOOL        _bValid;
    CHAR*       _arg;
    CHAR*       _endStr;
    U16         _i;
    CHAR        _tmpBuff[TRANSC_SIM_RGBUFF];
    U32         _RgLenLowBnd;
    U32         _RgLenUPBnd;

    CCURASSERT(pRgLenLowBnd);
    CCURASSERT(RgLenUPBnd);
    CCURASSERT(strRange);

    _bValid     = FALSE;
    _result     = EFAILURE;
    _RgLenLowBnd = 0;
    _RgLenUPBnd  = 0;
    ccur_strlcpy(_tmpBuff,strRange,sizeof(_tmpBuff));
    if('\0' != _tmpBuff[0])
    {
        _arg = strtok_r(
                _tmpBuff,"-",&_endStr);
        _i=0;
        while(_arg)
        {
            if(0 == _i)
            {
                _RgLenLowBnd = strtol(_arg,(char **)NULL, 10);
                _bValid   = TRUE;
            }
            else if(1 == _i)
            {
                _RgLenUPBnd = strtol(_arg,(char **)NULL, 10);
                _bValid   = TRUE;
            }
            else
            {
                _bValid   = FALSE;
                break;
            }
            _arg = strtok_r( NULL, "-" ,&_endStr);
            _i++;
        }
        if(_bValid)
        {
            if(_RgLenUPBnd > _RgLenLowBnd)
            {
                *pRgLenLowBnd = _RgLenLowBnd;
                *RgLenUPBnd   = _RgLenUPBnd;
                _result = ESUCCESS;
            }
        }
    }

    return _result;
}

/***************************************************************************
 * function: tcSimUtilRangeToContentLen
 *
 * description: calculate HTTP string range boundary and give result
 * in range size.
 ***************************************************************************/
CCUR_PROTECTED(CHAR*)
tcSimUtilRangeToContentLen(
        CHAR*   strContentLen,
        U32     nStrContentLen,
        CHAR*   strRange)
{
    tresult_t   _result;
    U32         _RgLenLowBnd;
    U32         _RgLenUPBnd;
    U32         _nContentLen;

    CCURASSERT(strRange);
    CCURASSERT(strContentLen);

    strContentLen[0] = '\0';
    _nContentLen = 0;
    _RgLenLowBnd = 0;
    _RgLenUPBnd  = 0;
    _result = tcSimUtilStrRangeToU32Ranges(
            &_RgLenLowBnd,&_RgLenUPBnd,strRange);
    if(ESUCCESS == _result)
    {
        if(_RgLenUPBnd > _RgLenLowBnd)
        {
            _nContentLen = _RgLenUPBnd - _RgLenLowBnd;
            snprintf(strContentLen,nStrContentLen,"%lu",_nContentLen);
        }
    }

    return strContentLen;
}

/***************************************************************************
 * function: tcSimUtilMultiPartRangeToContentLen
 *
 * description: calculate multiple HTTP string range boundaries and
 * give result in range size.
 ***************************************************************************/
CCUR_PROTECTED(CHAR*)
tcSimUtilMultiPartRangeToContentLen(
        CHAR*   strContentLen,
        U32     nStrContentLen,
        CHAR*   strRange)
{
    tresult_t   _result;
    CHAR*       _arg;
    CHAR*       _endStr;
    CHAR*       _pCh;
    U32         _nTotContentLen;
    CHAR        _strRgBuff[TRANSC_SIM_RGBUFF];

    CCURASSERT(strRange);
    CCURASSERT(strContentLen);

    strContentLen[0]   = '\0';
    _nTotContentLen    = 0;
    ccur_strlcpy(_strRgBuff,strRange,sizeof(_strRgBuff));
    if('\0' != _strRgBuff[0])
    {
        _arg = strtok_r(
                _strRgBuff,",",&_endStr);
        while(_arg)
        {
            _result     = EFAILURE;
            /* Quick Sanity test */
            _pCh = strchr(_arg,'-');
            if(isdigit(*(_pCh-1)) && isdigit(*(_pCh+1)))
            {
                tcSimUtilRangeToContentLen(
                        strContentLen,nStrContentLen,_arg);
                _nTotContentLen += strtol(strContentLen,NULL,10);
                _result     = ESUCCESS;
            }
            else
                break;
            _arg = strtok_r( NULL, "," ,&_endStr);
        }
        if(ESUCCESS == _result)
        {
            snprintf(strContentLen,nStrContentLen,"%lu",_nTotContentLen);
        }
    }

    return strContentLen;
}

/***************************************************************************
 * function: tcSimUtilIsRangeWithinContentLenU32
 *
 * description: check to see if range is within unsigned int length
 * "0-content-length"
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcSimUtilIsRangeWithinContentLenU32(
        CHAR* strRange,
        U32   nContentLen)
{
    tresult_t   _result;
    U32         _RgLenLowBnd;
    U32         _RgLenUPBnd;
    BOOL        _bIsInRg;

    CCURASSERT(strRange);

    _bIsInRg = FALSE;
    _RgLenLowBnd = 0;
    _RgLenUPBnd  = 0;
    _result = tcSimUtilStrRangeToU32Ranges(
            &_RgLenLowBnd,&_RgLenUPBnd,strRange);
    if(ESUCCESS == _result)
    {
        if ((_RgLenLowBnd < nContentLen) && (_RgLenUPBnd <= nContentLen))
         _bIsInRg = TRUE;
    }

    return _bIsInRg;
}

/***************************************************************************
 * function: tcSimUtilIsRangeWithinContentLen
 *
 * description: check to see if range is within string length
 * "0-content-length"
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcSimUtilIsRangeWithinContentLen(
        CHAR* strRange,
        CHAR* strContentLen)
{
    tresult_t   _result;
    U32         _RgLenLowBnd;
    U32         _RgLenUPBnd;
    U32         _nContentLen;
    BOOL        _bIsInRg;

    CCURASSERT(strRange);
    CCURASSERT(strContentLen);

    _bIsInRg = FALSE;
    _nContentLen = 0;
    _RgLenLowBnd = 0;
    _RgLenUPBnd  = 0;
    _result = tcSimUtilStrRangeToU32Ranges(
           &_RgLenLowBnd,&_RgLenUPBnd,strRange);
    if(ESUCCESS == _result)
    {
        _nContentLen = strtol(strContentLen,(char **)NULL, 10);
        if ((_RgLenLowBnd < _nContentLen) && (_RgLenUPBnd <= _nContentLen))
            _bIsInRg = TRUE;
    }

    return _bIsInRg;
}

/***************************************************************************
 * function: tcSimUtilIsRangeWithinContentLen
 *
 * description: check to see if (digits)-(digits) range request.
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcSimUtilIsMultiDigitsRange(
        CHAR*  strRange)
{
    BOOL        _bIsValid;
    BOOL        _bIsFirstValid;
    BOOL        _bIsSecondValid;
    CHAR*       _StrS;
    CHAR*       _StrE;
    U32         _i;

    CCURASSERT(strRange);

    do
    {
        _bIsFirstValid = FALSE;
        _bIsSecondValid = FALSE;
        if(!isdigit(strRange[0]))
            break;
        /* Check the first portion */
        _StrS = strRange;
        _StrE = strchr(strRange,'-');
        if(NULL == _StrE)
            break;
        if(0 == _StrE-_StrS)
            break;
        /* Check the first portion */
        _bIsFirstValid = TRUE;
        _i=0;
        while('-' != *_StrS)
        {
            if(!isdigit(*_StrS) || (_i > 64))
            {
                _bIsFirstValid = FALSE;
                break;
            }
            _StrS++;
            _i++;
        }
        if(FALSE == _bIsFirstValid)
            break;
        _StrS = ++_StrE;
        if('\0' == *_StrS )
        {
            _bIsSecondValid = FALSE;
            break;
        }
        /* Check the second portion */
        _bIsSecondValid = TRUE;
        _i=0;
        while('\0' != *_StrS)
        {
            if(!isdigit(*_StrS) || (_i > 64))
            {
                _bIsSecondValid = FALSE;
                break;
            }
            _StrS++;
            _i++;
        }
        if(FALSE == _bIsSecondValid)
            break;
    }while(FALSE);

    if(_bIsFirstValid && _bIsSecondValid)
        _bIsValid = TRUE;
    else
        _bIsValid = FALSE;

    return _bIsValid;
}

#if 0
CCUR_PROTECTED(tresult_t)
tcSimUtilCrcTiger(U8* Key, U32 nKey,
        const U8* buffer, size_t size)
{
    tresult_t _result;
    MHASH td;
    unsigned char hash[32];
    unsigned char work_buff[64];
    unsigned char *s = work_buff;
    int i;

    do
    {
        _result = EFAILURE;
        td = mhash_init(MHASH_TIGER);
        if(td == MHASH_FAILED)
            break;
        mhash(td, buffer, size);
        mhash_deinit(td, hash);

        bzero(work_buff, sizeof(work_buff));
        for(i = 0; i < mhash_get_block_size(MHASH_TIGER); ++i)
        {
            sprintf((char*) s, "%.2x", hash[i]);
            s += 2;
        }
        if(nKey > sizeof(work_buff))
            memcpy(Key, work_buff, sizeof(work_buff));
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}
#endif

/***************************************************************************
 * function: tcSimUtilCalculateCkSum
 *
 * description: calculate checksum using glib checksum operation. This
 * function needs to be replaced with mhash and we do not need glib
 * gslice to the same 0-1024 limit buffer of checksum calculation.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcSimUtilCalculateCkSum(
        CHAR*   StrStaticId,
        U32     nStrStaticId,
        U8*     strBodyPyload,
        U32     nCkSumLen)
{

    tresult_t   _result;
    CHAR*       _strCkSum;

    CCURASSERT(StrStaticId);

    do
    {
        _result = EFAILURE;
        if(0 == nCkSumLen)
            break;
        /* Calculate Checksum */
        _strCkSum = g_compute_checksum_for_data(
                                    G_CHECKSUM_SHA1,
                                    strBodyPyload,
                                    nCkSumLen);
        if(NULL == _strCkSum)
            break;
        ccur_strlcpy(StrStaticId,_strCkSum,nStrStaticId);
        g_free(_strCkSum);
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: tcSimUtilSendHttpHEADReq
 *
 * description: Send HEAD request and cleanup response content length.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcSimUtilSendHttpHEADReq(
        tc_simutil_curlmsg_t*   pCurlHeadBuff,
        tc_simutil_curlerr_t*   pErrCntr,
        CHAR*                   strContentLen,
        U32                     nStrContentLen,
        CHAR*                   strOutIntf,
        tc_qmsgtbl_pptosim_t*   pMsg,
        void*                   fCallback)
{
    tresult_t               _result;
    U32                     _nTotalWritten;
    CHAR*                   _pHeadContent;
    tc_sim_msgpyldtype_e    _eMsgPyldType;

    CCURASSERT(pErrCntr);
    CCURASSERT(strContentLen);
    CCURASSERT(strOutIntf);
    CCURASSERT(pMsg);
    CCURASSERT(fCallback);

    do
    {
        _result = EFAILURE;
        if('\0' == pMsg->strCRange[0])
            break;
        /* Always sending Http Header request
         * even if the range is in URL.
         */
        _result = _tcSimUtilSendHttpReq(
                    pErrCntr,
                    pCurlHeadBuff,
                    NULL,
                    strOutIntf,
                    pMsg,
                    pMsg->strCRange,
                    fCallback);
        if(ESUCCESS != _result)
        {
            pErrCntr->nCurlReqErr++;
            break;
        }
        if(0 == pCurlHeadBuff->nTotalWritten)
        {
            pErrCntr->nCurlReqErr++;
            break;
        }
        if('\0' == pCurlHeadBuff->strContentBuff[0])
        {
            pErrCntr->nCurlReqErr++;
            break;
        }
        if(pCurlHeadBuff->nTotalWritten >= pCurlHeadBuff->nstrContentBuff)
        {
            pErrCntr->nCurlReqErr++;
            break;
        }
        pCurlHeadBuff->strContentBuff[pCurlHeadBuff->nTotalWritten] = '\0';
        _pHeadContent = (CHAR*)pCurlHeadBuff->strContentBuff;
        if(!strncmp(
                (CHAR*)_pHeadContent+sizeof("HTTP/1.X"),"200",sizeof("200")-1))
            _eMsgPyldType = tcSimHttpMsgRespPyldTypeOK;
        else if(!strncmp(
                (CHAR*)_pHeadContent+sizeof("HTTP/1.X"),"206",sizeof("206")-1))
            _eMsgPyldType = tcSimHttpMsgRespPyldType206;
        else if(!strncmp(
                (CHAR*)_pHeadContent+sizeof("HTTP/1.X"),"302",sizeof("302")-1))
            _eMsgPyldType = tcSimHttpMsgRespPyldType302;
        else
            _eMsgPyldType = tcSimHttpMsgPyldTypeUNKNOWN;
        /* Only accepts two conditions to 206 request:
         * 1. 200 response with range: bytes=0-
         * 2. 206 response
         * any other respose will be inaccurate. */
        if((tcSimHttpMsgRespPyldTypeOK == _eMsgPyldType &&
            TRANSC_SIM_ISNO_RANGE_CK(pMsg->strCRange)) ||
            tcSimHttpMsgRespPyldType206 == _eMsgPyldType )
        {
            _result = _tcSimUtilGetHttpHeaderContentLen(
                    strContentLen,nStrContentLen,
                    pCurlHeadBuff->strContentBuff,
                    pCurlHeadBuff->nTotalWritten);
            if(ESUCCESS != _result)
            {
                pErrCntr->nBadContentLenParseErr++;
                break;
            }
            if('\0' == strContentLen[0])
            {
                pErrCntr->nBadContentLenParseErr++;
                _result = EFAILURE;
                break;
            }
            _nTotalWritten = tcSimUtilCleanContentLength(
                    strContentLen,nStrContentLen);
            if(0 == _nTotalWritten)
            {
                pErrCntr->nBadContentLenParseErr++;
                _result = EFAILURE;
                break;
            }
        }
        else if(tcSimHttpMsgRespPyldType302 == _eMsgPyldType)
        {
            pErrCntr->n302RespErr++;
            _result = EFAILURE;
            break;
        }
        else
        {
            pErrCntr->nBadRespErr++;
            _result = EFAILURE;
            break;
        }
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: tcSimUtilSendHttpGETReq
 *
 * description: Send GET request, do not check cURL response coming back as
 * error value does not always mean an error in retrieving content.
 * some cases we are requesting and buffering more than we need while what
 * we really need is specified within option value.
 ***************************************************************************/
CCUR_PROTECTED(U32)
tcSimUtilSendHttpGETReq(
        tc_simutil_curlmsg_t*   pCurlHeadBuff,
        tc_simutil_curlmsg_t*   pCurlBodyBuff,
        tc_simutil_curlerr_t*   pErrCntr,
        CHAR*                   strOutIntf,
        tc_qmsgtbl_pptosim_t*   pMsg,
        CHAR*                   strOptCkeyRg,
        void*                   fCallback)
{
    U32                     _nTotalWritten;
    CHAR*                   _pHeadContent;
    tc_sim_msgpyldtype_e    _eMsgPyldType;

    CCURASSERT(pErrCntr);
    CCURASSERT(strOutIntf);
    CCURASSERT(pMsg);
    CCURASSERT(fCallback);

    do
    {
        _nTotalWritten = 0;
        if(/*TRANSC_SIM_ISNO_RANGE_CK(pMsg->strCRange) || */
           '\0' == pMsg->strCRange[0])
            break;
        /* Don't check results, just check total written */
        _tcSimUtilSendHttpReq(
                    pErrCntr,
                    pCurlHeadBuff,
                    pCurlBodyBuff,
                    strOutIntf,
                    pMsg,
                    strOptCkeyRg,
                    fCallback);
        if(0 == pCurlHeadBuff->nTotalWritten)
        {
            pErrCntr->nCurlReqErr++;
            break;
        }
        if(0 == pCurlBodyBuff->nTotalWritten )
        {
            pErrCntr->nCurlReqErr++;
            break;
        }
        if(pCurlHeadBuff->nTotalWritten >= pCurlHeadBuff->nstrContentBuff)
        {
            pErrCntr->nCurlReqErr++;
            break;
        }
        pCurlHeadBuff->strContentBuff[pCurlHeadBuff->nTotalWritten] = '\0';
        _pHeadContent = (CHAR*)(pCurlHeadBuff->strContentBuff);
        if(!strncmp(
                (CHAR*)_pHeadContent+sizeof("HTTP/1.X"),"200",sizeof("200")-1))
            _eMsgPyldType = tcSimHttpMsgRespPyldTypeOK;
        else if(!strncmp(
                (CHAR*)_pHeadContent+sizeof("HTTP/1.X"),"206",sizeof("206")-1))
            _eMsgPyldType = tcSimHttpMsgRespPyldType206;
        else if(!strncmp(
                (CHAR*)_pHeadContent+sizeof("HTTP/1.X"),"302",sizeof("302")-1))
            _eMsgPyldType = tcSimHttpMsgRespPyldType302;
        else
            _eMsgPyldType = tcSimHttpMsgPyldTypeUNKNOWN;
        if(tcSimHttpMsgRespPyldTypeOK == _eMsgPyldType ||
           tcSimHttpMsgRespPyldType206 == _eMsgPyldType )
        {
            _nTotalWritten = pCurlBodyBuff->nTotalWritten;
        }
        else if(tcSimHttpMsgRespPyldType302 == _eMsgPyldType)
        {
            pErrCntr->n302RespErr++;
            break;
        }
        else
        {
            pErrCntr->nBadRespErr++;
            break;
        }
    }while(FALSE);

    return _nTotalWritten;
}

