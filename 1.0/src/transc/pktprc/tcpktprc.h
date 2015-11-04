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

#ifndef TCPKTPRC_H
#define TCPKTPRC_H

#ifdef __cplusplus
extern "C" {
#endif

#define TRANSC_PKTPRC_REQTYPE_OPT      "OPTIONS"
#define TRANSC_PKTPRC_REQTYPE_GET      "GET"
#define TRANSC_PKTPRC_REQTYPE_HEAD     "HEAD"
#define TRANSC_PKTPRC_REQTYPE_POST     "POST"
#define TRANSC_PKTPRC_REQTYPE_PUT      "PUT"
#define TRANSC_PKTPRC_REQTYPE_DELETE   "DELETE"
#define TRANSC_PKTPRC_REQTYPE_TRACE    "TRACE"
#define TRANSC_PKTPRC_REQTYPE_CONNECT  "CONNECT"
#define TRANSC_PKTPRC_GET_ETHTYPE(pyld)  ccur_nptrtohs(pyld+12)

enum _tc_pktprc_msgpyldtype_e
{
    tcHttpMsgPyldTypeUNKNOWN=0,
    /*req*/
    tcHttpMsgReqPyldTypeGET,
    tcHttpMsgReqPyldTypeOPTION,
    tcHttpMsgReqPyldTypeHEAD,
    tcHttpMsgReqPyldTypePOST,
    tcHttpMsgReqPyldTypePUT,
    tcHttpMsgReqPyldTypeDELETE,
    tcHttpMsgReqPyldTypeTRACE,
    tcHttpMsgReqPyldTypeCONNECT,
    /*resp*/
    tcHttpMsgRespPyldTypeOK,
    tcHttpMsgRespPyldType302,
    tcHttpMsgRespPyldType303
};
typedef enum _tc_pktprc_msgpyldtype_e
             tc_pktprc_msgpyldtype_e;

struct _tc_pktprc_ckeyq_s
{
    CHAR                            strKey[TRANSC_PKTPRC_URL_STRCKEYSIGLEN];
    tc_regex_t                      tReCkey;
};
typedef struct _tc_pktprc_ckeyq_s
               tc_pktprc_ckeyq_t;

/**********************Thread stats *******************************/
struct _tc_pktprc_httpsess_s
{
    /*/services/<session>/matchsize */
    S32                 nMatchSz;
    /*/services/<session>/signature */
    tc_pktprc_ckeyq_t   tSig;
};
typedef struct _tc_pktprc_httpsess_s
               tc_pktprc_httpsess_t;

struct _tc_pktprc_httpuri_s
{
    /*/services/<url>/matchsize */
    S32                 nMatchSz;
    /*/services/<url>/signature */
    tc_pktprc_ckeyq_t   tSig;
};
typedef struct _tc_pktprc_httpuri_s
               tc_pktprc_httpuri_t;

/**********************HTTP SITES *******************************/
struct _tc_pktprc_httpsites_s
{
    /*/services/session[] list*/
    tc_pktprc_httpsess_t    tHttpSessTbl[TRANSC_LDCFG_SITE_MAXSESSTYPE_LST];
    U16                     nHttpSessTbl;
    /*/services/url[] list */
    tc_pktprc_httpuri_t     tHttpURITbl[TRANSC_LDCFG_SITE_MAXURITYPE_LST];
    U16                     nHttpURITbl;
};
typedef struct _tc_pktprc_httpsites_s
               tc_pktprc_httpsites_t;

/**********************Thread context******************************/
struct _tc_pktprc_thread_ctxt_s
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
    /****** Periodic time  ****************/
    tc_gd_time_t            tStatsDumpCkTime;
    tc_gd_time_t            tStatsFlushQueues;
    /****** config and init Flags ****************/
    tc_tr_sts_e        bTrSts;
    BOOL                    bSendHealthReport;
    BOOL                    bNonIntfCfgLoadSuccess;
    BOOL                    bIntfCfgLoadSuccess;
    BOOL                    bProcessProxyReq;
    /****** Interface Management ****************/
    tc_monintf_intfd_t      tIntfX;
    tc_intf_config_t        tIntfCfg;
    /****** Config Loaded Services Management ****************/
    tc_pktprc_httpsites_t   tPPSitesTbl[TRANSC_LDCFG_SITE_MAXSITES_LST];
    S32                     nPPSitesTbl;
    /****** Statistics counters ****************/
    U32                     nPcapParseErr;
    U32                     nPcapParseIgnored;
    U32                     nPcapPktTotal;
    U32                     nPcapPktIgnored;
    U32                     nPcapIgnGetReqFrmCacheSrvr;
    U32                     nGetRedirPop;
    U32                     nGetReqErr;
    U32                     pHashIpAddrBlkListTbl[TRANSC_LDCFG_BLKLISTTBL_SZ];
    U32                     nHashIpAddrBlkListTbl;
    /****** QUEUES Management ****************/
    /* PP -> Bkgrnd thread */
    evlog_desc_t            tLogDescSys;
    evlog_t*                pQPktProcToBkgrnd;
    /* PP -> Http Proc thread */
    lkfq_tc_t*              pQPPktProcToHttpProc;
    /* PP -> Mib thread */
    lkfq_tc_t*              pQPktProcToMib;
    tc_qmsgtbl_comptomib_t  tGenMsg;
    S32                     nGenMsgBuffOflw;
    /****** temporary variables Management */
    U8                      PktBuffer[TRANSC_PKTPRC_BUFFCAPLEN];
    /****** Message Passing Management ****************/
    tc_mtxtbl_ipptohlth_t   tTmpPPtoHlthMonIntf;
};
typedef struct _tc_pktprc_thread_ctxt_s
               tc_pktprc_thread_ctxt_t;

CCUR_PROTECTED(tresult_t)
tcPktPrcLogStats(
        tc_pktprc_thread_ctxt_t* pCntx);

CCUR_PROTECTED(void)
tcPktPrcParseGetDomainName(
        tc_pktprc_thread_ctxt_t* pCntx,
        CHAR*                    strDomainName,
        U16                      nStrDomainName,
        const CHAR*              data,
        S32                      len);

CCUR_PROTECTED(tresult_t)
tcPktPrcFlushMibTable(
        tc_pktprc_thread_ctxt_t*        pCntx);

CCUR_PROTECTED(tresult_t)
tcPktPrcWriteMibQTraffic(
        tc_pktprc_thread_ctxt_t*        pCntx,
        CHAR*                           strDomainName,
        tc_iphdr_ipaddr_t*              pClientIpAddr);

CCUR_PROTECTED(void)
tcPktPrcReadCfg(tc_pktprc_thread_ctxt_t* pCntx, BOOL bForceUpdShVal);

CCUR_PROTECTED(BOOL)
tcPktPrcIsRequestServiceAvail(
        tc_pktprc_thread_ctxt_t*        pCntx,
        tc_g_svcdesc_t*                 pSvcType,
        CHAR*                           pPyld,
        S32                             nPyldLen);

CCUR_PROTECTED(tresult_t)
tcPktPrcToHttpProcQueueMsg(
        tc_pktprc_thread_ctxt_t*         pCntx,
        tc_monintf_map_t*            pMapIntf,
        tc_g_svcdesc_t*                  pSvcType,
        tc_g_qmsgpptohp_t*               pPktDescQMsg);

CCUR_PROTECTED(mthread_result_t)
tcPktPrcThreadEntry(void* pthdArg);

#ifdef __cplusplus
}
#endif
#endif /* TCPKTPRC_H */
