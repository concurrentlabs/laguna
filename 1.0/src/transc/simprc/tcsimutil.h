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

#ifndef tcsimutil_H
#define tcsimutil_H

#ifdef __cplusplus
extern "C" {
#endif


#define TRANSC_SIM_ISNO_RANGE_CK(x) (('0' == x[0]) && \
                                    ('-' == x[1]) && \
                                    ('\0' == x[2]))

struct _tc_simutil_curlerr_s
{
    U16     eCurlErrcode;
    U32     nCurlReqErr;
    U32     nCurlEtimeoutErr;
    U32     nCurlServerConnErr;
    U32     nCurlOtherErr;
    /* Non Curl Errors but HTTP GET/HEAD processing
     * errors*/
    U32     n302RespErr;
    U32     nBadRespErr;
    U32     nBadContentLenParseErr;
};
typedef struct _tc_simutil_curlerr_s
               tc_simutil_curlerr_t;

struct _tc_simutil_curlmsg_s
{
    U8*     strContentBuff;
    U32     nstrContentBuff;
    U32     nTotalWritten;
};
typedef struct _tc_simutil_curlmsg_s
               tc_simutil_curlmsg_t;

CCUR_PROTECTED(tc_sim_reqtype_e)
tcSimUtilGetReqType(
        tc_sim_rangetype_e     eRgValType);

CCUR_PROTECTED(tresult_t)
tcSimUtilGetReqContentLen(
        CHAR*                   strCRange,
        tc_sim_rangetype_e      eRgValType,
        CHAR*                   pStrContentLen,
        U32                     nStrContentLen);

CCUR_PROTECTED(tresult_t)
tcSimUtilStrRangeToU32Ranges(
        U32*    pRgLenLowBnd,
        U32*    RgLenUPBnd,
        CHAR*   strRange);

CCUR_PROTECTED(CHAR*)
tcSimUtilRangeToContentLen(
        CHAR*   strContentLen,
        U32     nStrContentLen,
        CHAR*   strRange);

CCUR_PROTECTED(CHAR*)
tcSimUtilMultiPartRangeToContentLen(
        CHAR*   strContentLen,
        U32     nStrContentLen,
        CHAR*   strRange);

CCUR_PROTECTED(BOOL)
tcSimUtilIsMultiDigitsRange(
        CHAR*  strRange);

CCUR_PROTECTED(BOOL)
tcSimUtilIsRangeWithinContentLen(
        CHAR* strRange,
        CHAR* strContentLen);

CCUR_PROTECTED(BOOL)
tcSimUtilIsRangeWithinContentLenU32(
        CHAR* strRange,
        U32   nContentLen);

CCUR_PROTECTED(tresult_t)
tcSimUtilCalculateCkSum(
        CHAR*   StrStaticId,
        U32     nStrStaticId,
        U8*     strBodyPyload,
        U32     nCkSumLen);

CCUR_PROTECTED(tresult_t)
tcSimUtilSendHttpHEADReq(
        tc_simutil_curlmsg_t*   pCurlHeadBuff,
        tc_simutil_curlerr_t*   pErrCntr,
        CHAR*                   strContentLen,
        U32                     nStrContentLen,
        CHAR*                   strOutIntf,
        tc_qmsgtbl_pptosim_t*   pMsg,
        void*                   fCallback);

CCUR_PROTECTED(U32)
tcSimUtilSendHttpGETReq(
        tc_simutil_curlmsg_t*   pCurlHeadBuff,
        tc_simutil_curlmsg_t*   pCurlBodyBuff,
        tc_simutil_curlerr_t*   pErrCntr,
        CHAR*                   strOutIntf,
        tc_qmsgtbl_pptosim_t*   pMsg,
        CHAR*                   strOptCkeyRg,
        void*                   fCallback);

#ifdef __cplusplus
}
#endif
#endif /* tcsimutil_H */
