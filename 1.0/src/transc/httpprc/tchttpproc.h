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

#ifndef  TCHTTPPROC_H
#define  TCHTTPPROC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cprops/mempool.h>

/**********************Thread context******************************/

struct _tc_httpprc_stats_s
{
    /* Resource Limits */
    /* Counters Resource */
    /* Statistics counters */
    U32                     nPktsToClients;
    U32                     nPktsToClientsSz;
    U32                     nVideoFileReq;
};
typedef struct _tc_httpprc_stats_s
               tc_httpprc_stats_t;

struct _tc_httpprc_ckeyq_s
{
    CHAR                            strKey[TRANSC_HTTPPRC_URL_STRCKEYSIGLEN];
    tc_regex_t                      tReCkey;
};
typedef struct _tc_httpprc_ckeyq_s
               tc_httpprc_ckeyq_t;

struct _tc_httpprc_httphdmatch_s
{
    CHAR                strFieldName[64];
    tc_httpprc_ckeyq_t  tFieldValue;
};
typedef struct _tc_httpprc_httphdmatch_s
               tc_httpprc_httphdmatch_t;

/**********************HTTP Session & URI *******************************/
struct _tc_httpprc_httpsess_s
{
    /*/services/<session>/matchsize */
    S32                 nMatchSz;
    /*/services/<session>/signature */
    tc_httpprc_ckeyq_t   tSig;
    /*/services/<session>/cachekeyid */
    tc_httpprc_ckeyq_t   tCKeyId[TRANSC_HTTPPRC_URL_CKEYARGS_LST];
    U16                 nCKeyId;
    /*/services/<session>/contextid */
    tc_httpprc_ckeyq_t   tCtxId[TRANSC_HTTPPRC_URL_CKEYARGS_LST];
    U16                 nCtxId;
    /*/services/<url>/cachekeyrange */
    tc_httpprc_ckeyq_t   tCKeyRange;
    /*/services/<url>/cachekeymisc */
    tc_httpprc_ckeyq_t   tCkeyMisc[TRANSC_HTTPPRC_URL_CKEYARGS_LST];
    U16                 nCKeyMisc;
};
typedef struct _tc_httpprc_httpsess_s
               tc_httpprc_httpsess_t;

struct _tc_httpprc_httpuri_s
{
    /*/services/<url>/matchsize */
    S32                 nMatchSz;
    /*/services/<url>/signature */
    tc_httpprc_ckeyq_t   tSig;
    /*/services/<url>/cachekeyid */
    tc_httpprc_ckeyq_t   tCKeyId[TRANSC_HTTPPRC_URL_CKEYARGS_LST];
    U16                 nCKeyId;
    /*/services/<session>/contextid */
    tc_httpprc_ckeyq_t   tCtxId[TRANSC_HTTPPRC_URL_CKEYARGS_LST];
    U16                 nCtxId;
    /*/services/<url>/cachekeyrange */
    tc_httpprc_ckeyq_t   tCKeyRange;
    /*/services/<url>/cachekeymisc */
    tc_httpprc_ckeyq_t   tCkeyMisc[TRANSC_HTTPPRC_URL_CKEYARGS_LST];
    U16                 nCKeyMisc;
    BOOL                bSessRel;
};
typedef struct _tc_httpprc_httpuri_s
               tc_httpprc_httpuri_t;

/**********************HTTP HOST *******************************/
struct _tc_httpprc_httphost_s
{
    tc_regex_t         tReHost;
    CHAR               strHostSig[TRANSC_HTTPPRC_SITE_STRHOSTSIGLEN];
};
typedef struct _tc_httpprc_httphost_s
               tc_httpprc_httphost_t;

/**********************HTTP SITES *******************************/
struct _tc_httpprc_httpsites_s
{
    /*/services/target */
    CHAR                    strPktPrcArgSites[TRANSC_LDCFG_SITE_STRSITESLEN];
    /*/services/host[] list */
    tc_httpprc_httphost_t    tHost[TRANSC_HTTPPRC_SITE_MAXHOSTYPE_LST];
    U16                     nHost;
    /*/services/referrer[] list */
    tc_httpprc_ckeyq_t       tReferrer[TRANSC_HTTPPRC_SITE_MAXREFTYPE_LST];
    U16                     nReferrer;
    /*/services/httprangefield */
    tc_httpprc_ckeyq_t       tHttpRgFld;
    /*/services/httpsessionfield */
    tc_httpprc_ckeyq_t       tHttpSessFld;
    /*/services/option */
    CHAR                    strPktPrcArgOpt[TRANSC_LDCFG_SITE_STROPTLEN];
    /*/services/type */
    CHAR                    strPktPrcArgType[TRANSC_LDCFG_SITE_STRTYPELEN];
    /*/services/session[] list*/
    tc_httpprc_httpsess_t    tHttpSess[TRANSC_LDCFG_SITE_MAXSESSTYPE_LST];
    U16                     nHttpSess;
    /*/services/url[] list */
    tc_httpprc_httpuri_t     tHttpURI[TRANSC_LDCFG_SITE_MAXURITYPE_LST];
    U16                     nHttpURI;
    /*/services/HttpHdrMatch[] list */
    tc_httpprc_httphdmatch_t tHttpHdrMatch[TRANSC_HTTPPRC_SITE_MAXHTTPHDRMATCH_LST];
    U16                     nHttpHdrMatch;
};
typedef struct _tc_httpprc_httpsites_s
               tc_httpprc_httpsites_t;


struct _tc_httpprc_thread_ctxt_s
{
    /****** Thread Init ****************/
    U32                     tid;
    BOOL                    bExit;
    msem_t                  tCondVarSem;
    BOOL                    bCondVarSet;
    mthread_t               tThd;
    struct tm*              tGMUptime;
    tc_gd_time_t            tUptime;
    tc_gd_time_t            tDowntime;
    BOOL                    bBwSimMode;
    /****** Periodic time  ****************/
    tc_gd_time_t            tInjDelayDiffTime;
    tc_gd_time_t            tStatsDumpCkTime;
    tc_gd_time_t            tTcpStrmHashTblCkTime;
    /****** config and init Flags ****************/
    tc_tr_sts_e        bTrSts;
    http_parser_settings    tSettings;
    http_parser             tParser;
    CHAR                    strHttpPrcRedirTarget[256];
    tc_httpprc_httpsites_t  tPPSitesTbl[TRANSC_LDCFG_SITE_MAXSITES_LST];
    S32                     nPPSitesTbl;
    /****** Statistics counters ****************/
    tc_httpprc_stats_t      tGenSiteTbl[TRANSC_LDCFG_SITE_MAXSITES_LST];
    U32                     nPackets;
    U32                     nHdrMatchIgnore;
    U32                     nHttpPktDump;
    U32                     nHttpPktTotal; /* Passed from pkt prc */
    U32                     nHttpPktProcessed;
    U32                     nGetReqProcessed;
    U32                     nErrQNoMemHttpProcToSim;
    U32                     nHttpPktIgnored;
    U32                     nIgnGetReqSvcUnknown;
    U32                     nHttpPktProcErr;
    U32                     nGetReqErr;
    U32                     nInjDelayCntr;
    S32                     nSimMsgOflw;
    /****** video stream session ****************/
    cdll_hdnode_t           shTcpStrmSessSitesTbl[TRANSC_LDCFG_SITE_MAXSITES_LST]\
                                              [TRANSC_HTTPPRC_TCPSTRM_DFLTBIN_CNT];
    cp_mempool *            pTcpStrmSessMpool;
    U32                     nResTcpStrmSessBinMax;
    U32                     nResTcpStrmSessMax;
    U32                     nResTcpStrmSessGrowSz;
    U32                     nTcpStrmSessActive;
    U32                     nTcpStrmSessHtblBin;
    /****** QUEUES Management ****************/
    /* Http Proc -> Logging thread  */
    evlog_t*                pQHttpProcToBkgrnd;
    lkfq_tc_t*              pQPPktProcToHttpProc;
    evlog_desc_t            tLogDescSys;
    evlog_desc_t            tLogDescSvc;
    /* Http Proc -> Mib thread */
    lkfq_tc_t*              pQHttpProcToMib;
    tc_qmsgtbl_comptomib_t  tRedirMsg;
    S32                     nRedirMsgOflw;
    /* Http Proc -> Sim thread */
    lkfq_tc_t*              pQHttpProcToSim;
    /****** temporary variables Management */
    tc_g_qmsgpptohp_t       tPktDescMsg;
    /****** Message Passing Management ****************/
};
typedef struct _tc_httpprc_thread_ctxt_s
               tc_httpprc_thread_ctxt_t;

CCUR_PROTECTED(tresult_t)
tcHttpProcFlushMibTable(
        tc_httpprc_thread_ctxt_t*        pCntx);

CCUR_PROTECTED(void)
tcHttpProcReadCfg(
        tc_httpprc_thread_ctxt_t* pCntx, BOOL bForcedLdShVal);

CCUR_PROTECTED(mthread_result_t)
tcHttpProcThreadEntry(void* pthdArg);

#ifdef __cplusplus
}
#endif
#endif /* TCHTTPPROC_H */
