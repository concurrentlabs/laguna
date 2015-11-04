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

#ifndef  TCHTTPPARSE_H
#define  TCHTTPPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

enum _tc_httpparse_reqfieldtype_e
{
    tcHttpReqFieldypeNone=0,
    tcHttpReqFieldypeHost,
    tcHttpReqFieldypeRange,
    tcHttpReqFieldypeSession,
    tcHttpReqFieldypeReferer,
    tcHttpReqFieldypeOrigin,
    tcHttpReqFieldypeSimRange,
    tcHttpReqFieldypeUAgent,
    tcHttpReqFieldypeMax,
};
typedef enum _tc_httpparse_reqfieldtype_e
             tc_httpparse_reqfieldtype_e;

#define tcHttpParseSiteTypeUnknown  tcHttpParseSvcTypeUnknown

enum _tc_httpparse_hdrmatch_e
{
    tcHttpParseHdrMatchCheck,
    tcHttpParseHdrMatchProcess,
    tcHttpParseHdrMatchIgnore
};
typedef enum _tc_httpparse_hdrmatch_e
             tc_httpparse_hdrmatch_e;

struct _tc_httpparse_sessdata_s
{
    CHAR                                strCId[TRANSC_QMSG_IDLEN];
    CHAR                                strCRange[TRANSC_QMSG_RGLEN];
    CHAR                                strCMisc[TRANSC_QMSG_MISCLEN];
    U32                                 PktHash;
    hlpr_httpsite_hndl                  eSiteHost;
    U32                                 nBin;
    U16                                 nSessTimoutVal;
};
typedef struct _tc_httpparse_sessdata_s
               tc_httpparse_sessdata_t;

struct _tc_httpparse_sess_s
{
    cdll_node_t                         tStrmNode;
    tc_gd_time_t                        tLastUpdateTime;
    U32                                 nReq;
    tc_httpparse_sessdata_t             tSessData;
};
typedef struct _tc_httpparse_sess_s
               tc_httpparse_sess_t;

struct _tc_httpparse_calldef_s
{
    tc_httpprc_thread_ctxt_t*       pCntx;
    tc_httpparse_reqfieldtype_e     eFieldType;
    hlpr_httpsite_hndl              eSiteHost;
    hlpr_httpsite_hndl              ePossibleSiteHost;
    tc_g_qmsghptopg_t               tInjMsg;
    BOOL                            bParseErr;
    tc_httpparse_hdrmatch_e         eHttpHdrMatchType;
    U16                             nHttpHdrMatchIdx;
};
typedef struct _tc_httpparse_calldef_s
               tc_httpparse_calldef_t;

CCUR_PROTECTED(int)
tcHttpParseReqHdrFieldCb (
        http_parser *pParse, const CHAR *buf, size_t len);

CCUR_PROTECTED(int)
tcHttpParseReqHdrValueCb (
        http_parser *pParse, const CHAR *buf, size_t len);

CCUR_PROTECTED(int)
tcHttpParseReqUrlCb (http_parser *pParse, const CHAR *buf, size_t len);

CCUR_PROTECTED(BOOL)
tcHttpParserGetCacheKey(
        tc_httpprc_thread_ctxt_t*    pCntx,
        CHAR*                       pStrCid,
        U32                         nStrCidSz,
        tc_httpprc_ckeyq_t*          pKeyId,
        U16                         nKeyId,
        tc_httpparse_calldef_t*     pHttpMsg,
        CHAR*                       delim);

CCUR_PROTECTED(tresult_t)
tcHttpParserGetAllUrlCacheKeyId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

CCUR_PROTECTED(BOOL)
tcHttpParserGetUrlCacheKeyId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

CCUR_PROTECTED(BOOL)
tcHttpParserGetUrlCacheKeyRange(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

CCUR_PROTECTED(BOOL)
tcHttpParserGetUrlCacheKeyMisc(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

CCUR_PROTECTED(tresult_t)
tcHttpParserGetUrlHashKeyId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        U32*                        pHash,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

CCUR_PROTECTED(tresult_t)
tcHttpParserGetUrlHashCntxId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        U32*                        pHash,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

CCUR_PROTECTED(tresult_t)
tcHttpParserGetAllSessCacheKeyId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg,
        tc_httpparse_sess_t*      pSymStrm);

CCUR_PROTECTED(BOOL)
tcHttpParserGetSessCacheKeyId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

CCUR_PROTECTED(BOOL)
tcHttpParserGetSessCacheKeyRange(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

CCUR_PROTECTED(BOOL)
tcHttpParserGetSessCacheKeyMisc(
        tc_httpprc_thread_ctxt_t*    pCntx,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

CCUR_PROTECTED(tresult_t)
tcHttpParserGetSessHashCntxId(
        tc_httpprc_thread_ctxt_t*    pCntx,
        U32*                        pHash,
        tc_g_svcdesc_t*     pSvcType,
        tc_httpparse_calldef_t*     pHttpMsg);

#ifdef __cplusplus
}
#endif
#endif
