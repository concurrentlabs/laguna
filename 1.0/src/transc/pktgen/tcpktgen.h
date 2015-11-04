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

#ifndef TCPKTGEN_H
#define TCPKTGEN_H

#ifdef __cplusplus
extern "C" {
#endif

enum _tc_pktgen_tcpflgproto_e
{
    tcTcpFlagProto = 0x0,
    tcTcpFlagProtoSyn = 0x1,
    tcTcpFlagProtoFin = 0x2,
    tcTcpFlagProtoFinAck = 0x4,
    tcTcpFlagProtoRst = 0x8,
    tcTcpFlagProtoRstAck = 0x16
};
typedef enum _tc_pktgen_tcpflgproto_e
             tc_pktgen_tcpflgproto_e;


/*#if (TRANSC_PKTGEN_OUTPUT_PYLDSIZE+TRANSC_PKTGEN_OUTPUT_L1L4 > PKTGEN_OUTPUT_FRAMESIZE)
#error "(TRANSC_PKTGEN_OUTPUT_PYLDSIZE+TRANSC_PKTGEN_OUTPUT_L1L4 > PKTGEN_OUTPUT_FRAMESIZE)"
#endif
*/
struct _tc_pktgen_pkt_s
{
    U8*     pMsgStrt;
    U32     nCaplen;
    S16     nEthOffset;
    S16     nL3Offset;
    S16     nL4Offset;
    S16     nPayloadOffset;
    S16     nVlanOffset;
};
typedef struct _tc_pktgen_pkt_s
               tc_pktgen_pkt_t;

struct _tc_pktgen_ept_s {
    U32       ipv4;
    U16       ipv6[8];
    U8        mac[6];
    U32       nPort;
    U32       tcp_seq;
    U32       tcp_ack;
};
typedef struct _tc_pktgen_ept_s
               tc_pktgen_ept_t;

struct _tc_pktgen_pktinfo_s
{
    BOOL                    bInsertMplsTag;
    tc_pktgen_pkt_t         tPktPcap;
    tc_pktgen_ept_t         tFwd;
    tc_pktgen_ept_t         tRev;
    struct timeval          reftime;
    tc_pktgen_tcpflgproto_e eFlagProto;
    U16                     ethType;
    /* Tcp payload info */
    U8                      TcpPayldBuff[TRANSC_PKTGEN_OUTPUT_PYLDSIZE];
    U32                     TcpPayldLen;
    S32                     nEthFramesize;
};
typedef struct _tc_pktgen_pktinfo_s
               tc_pktgen_pktinfo_t;

struct _tc_pktgen_pktinj_s
{
    U8                  pkt[TRANSC_PKTGEN_OUTPUT_L1L4];  /* other than payload */
    U32                 pktlen;                  /* packet length */
};
typedef struct _tc_pktgen_pktinj_s
               tc_pktgen_pktinj_t;

struct _tc_pktgen_s
{
    /* Pkt info data input and injection output */
    tc_pktgen_pktinfo_t     tPktInfo; /* Input pkt info data */
    tc_pktgen_pktinj_t      tPktInj;  /* Output Pkt injection data */
    /* packet stats */
    U32                     npackets;
    U32                     nRawPacketsSent;
    U32                     nRawBytesSent;
};
typedef struct _tc_pktgen_s
                tc_pktgen_t;

struct _tc_pktgen_thread_ctxt_s
{
    /****** Thread Init ****************/
    U32                         tid;
    BOOL                        bExit;
#if PKTGEN_THD
    msem_t                      tCondVarSem;
    BOOL                        bCondVarSet;
    mthread_t                   tThd;
#endif
    struct tm*                  tGMUptime;
    tc_gd_time_t                tUptime;
    tc_gd_time_t                tDowntime;
    /****** Periodic time  ****************/
    tc_gd_time_t                tRedirReqDownTime;
    tc_gd_time_t                tStatsDumpCkTime;
    tc_gd_time_t                tStatsUpdShValCkTime;
    /****** config and init Flags ****************/
    tc_tr_sts_e            bTrSts;
    BOOL                        bBlockTraffic;
    BOOL                        bSendHealthReport;
    BOOL                        bNonIntfCfgLoadSuccess;
    BOOL                        bIntfCfgLoadSuccess;
    BOOL                        bRedirectCORSReq;
    BOOL                        bOutIntfInsertMplsTag;
    BOOL                        bRedirCapReached;
    /****** Interface Management ****************/
    tc_outintf_intfd_t          tIntfX;
    tc_intf_config_t            tIntfCfg;
    /****** Statistics counters ****************/
    U32                         nPktRedirErr;
    U32                         nTotalRedirCntr;
    U32                         nRedirRate;
    U32                         nReqSampleCnt;
    U32                         nRedirReqRateCap;
    U32                         nRedirReqCapCntr;
    U32                         nTotalMonPktRedirected;
    U32                         nTotalActvPktRedirected;
    U32                         nOutIntfMapInjectionError;
    U32                         nTCPRst;
    U32                         nTCPFin;
    U32                         nTCPRedir;
    U32                         nInjectCntr;
    /****** QUEUES Management ****************/
    /* PP -> Bkgrnd thread */
    evlog_desc_t                tLogDescSys;
    evlog_desc_t                tLogDescSvc;
    evlog_t*                    pQPktGenToBkgrnd;
    /****** temporary variables Management */
    /* ... */
    /****** Message Passing Management ****************/
    tc_mtxtbl_stspgentohlth_t   tTmpPktGenToHlthSts;
    tc_mtxtbl_stspgentohlth_t   tOldTmpPktGenToHlthSts;
    tc_mtxtbl_epgentohlth_t     tTmpPktGenToHlthOutIntf;
    tc_mtxtbl_mpgentohlth_t     tTmpPktGenToHlthMapIntf;
    tc_mtxtbl_mhlthtopktgen_t   tTmpHlthToPKtGenMapIntf;

};
typedef struct _tc_pktgen_thread_ctxt_s
               tc_pktgen_thread_ctxt_t;

CCUR_PROTECTED(void)
tcPktGenReadCfg(tc_pktgen_thread_ctxt_t*  pCntx,BOOL bForceUpdShVal);

CCUR_PROTECTED(tresult_t)
tcPktGenLogStats(
        tc_pktgen_thread_ctxt_t* pCntx);

CCUR_PROTECTED(tresult_t)
tcPktGenLogStatsSummary(
        tc_pktgen_thread_ctxt_t* pCntx);

CCUR_PROTECTED(void)
tcPktGenProcessPktGen(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_g_qmsghptopg_t*          pInjMsg,
        tc_pktdesc_t*               pPktDesc,
        hlpr_httpsite_hndl          eSite);

CCUR_PROTECTED(tc_gd_time_t*)
tcPktGenCheckRedirPerSecMax(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_gd_time_t*               pUpdDiff,
        tc_gd_time_t*               pNowTime,
        tc_gd_time_t*               pOldTime,
        U32*                        nReqCnt);

CCUR_PROTECTED(void)
tcPktGenStatsDump(
        tc_pktgen_thread_ctxt_t*   pCntx,
        tc_gd_time_t*              pUpdDiff);

CCUR_PROTECTED(void)
tcPktGenUpdateSharedValues(
        tc_pktgen_thread_ctxt_t* pCntx, BOOL bRead);

CCUR_PROTECTED(void)
tcPktGenTimedUpdateSharedValues(
        tc_pktgen_thread_ctxt_t*   pCntx,
        tc_gd_time_t*              pUpdDiff);

#ifdef __cplusplus
}
#endif
#endif
