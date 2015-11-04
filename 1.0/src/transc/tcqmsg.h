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

#ifndef TCQMSG_H
#define TCQMSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "helper.h"
#include "tcpktparse.h"

/*************************************Q SIZES*************************************/
#ifdef TRANSC_DEBUG
/********************Q Health -> BKGRND ***************************/
#define TRANSC_HEALTH_TO_BKGRND_Q_SZ            600
#define TRANSC_HEALTH_TO_BKGRND_MEM_SZ          200
#define TRANSC_HEALTH_TO_BKGRND_MEM_GROWCNT     3
/********************Q PKT PROC -> BKGRND ***************************/
#define TRANSC_PKTPRC_TO_BKGRND_Q_SZ            6000
#define TRANSC_PKTPRC_TO_BKGRND_MEM_SZ          2000
#define TRANSC_PKTPRC_TO_BKGRND_MEM_GROWCNT     3
/********************Q PKT PROC -> MIB ***************************/
#define TRANSC_PKTPRC_TO_MIB_Q_SZ               5000
#define TRANSC_PKTPRC_TO_MIB_MEM_SZ             2500
#define TRANSC_PKTPRC_TO_MIB_MEM_GROWCNT        2
/********************Q HTTP PROC -> BKGRND ***************************/
#define TRANSC_HTTPRC_TO_BKGRND_Q_SZ            6000
#define TRANSC_HTTPRC_TO_BKGRND_MEM_SZ          2000
#define TRANSC_HTTPRC_TO_BKGRND_MEM_GROWCNT     3
/********************Q HTTP PROC -> MIB ***************************/
#define TRANSC_HTTPPRC_TO_MIB_Q_SZ              5000
#define TRANSC_HTTPPRC_TO_MIB_MEM_SZ            2500
#define TRANSC_HTTPPRC_TO_MIB_MEM_GROWCNT       2
/********************Q PKT GEN -> BKGRND ***************************/
#define TRANSC_PKTGEN_TO_BKGRND_Q_SZ            2000
#define TRANSC_PKTGEN_TO_BKGRND_MEM_SZ          1000
#define TRANSC_PKTGEN_TO_BKGRND_MEM_GROWCNT     2
/********************Q PKT GEN -> MIB ***************************/
/* No Queue - the same thread as http proc */
/********************Q PKT PROC -> HTTP PROC ***************************/
#define TRANSC_PKTPRC_TO_HTTPRC_Q_SZ            5000
#define TRANSC_PKTPRC_TO_HTTPRC_MEM_SZ          2500
#define TRANSC_PKTPRC_TO_HTTPRC_MEM_GROWCNT     2
/********************Q HTTP PROC -> PKT GEN ***************************/
/* No Queue - the same thread as http proc */
/********************Q SIM PROC -> BKGRND PROC ***************************/
#define TRANSC_SIMPRC_TO_BKGRND_Q_SZ            4000
#define TRANSC_SIMPRC_TO_BKGRND_MEM_SZ          2000
#define TRANSC_SIMPRC_TO_BKGRND_MEM_GROWCNT     2
/********************Q HTTP PROC -> SIM PROC ***************************/
#define TRANSC_HTTPPRC_TO_SIM_Q_SZ              4000
#define TRANSC_HTTPPRC_TO_SIM_MEM_SZ            2000
#define TRANSC_HTTPPRC_TO_SIM_MEM_GROWCNT       2
/********************Q SIM PROC -> SIM/SND PROC ***************************/
#define TRANSC_SIMPRC_TO_SIMSNDW_Q_SZ           4000
#define TRANSC_SIMPRC_TO_SIMSNDW_MEM_SZ         2000
#define TRANSC_SIMPRC_TO_SIMSNDW_MEM_GROWCNT    2
/********************Q SIM/SND PROC -> SIM PROC ***************************/
#define TRANSC_SIMSNDW_TO_SIM_Q_SZ              4000
#define TRANSC_SIMSNDW_TO_SIM_MEM_SZ            2000
#define TRANSC_SIMSNDW_TO_SIM_MEM_GROWCNT       2
/********************Q SIM/SND PROC -> BKGRND PROC ***************************/
#define TRANSC_SIMSNDW_TO_BKGRND_Q_SZ           4000
#define TRANSC_SIMSNDW_TO_BKGRND_MEM_SZ         1000
#define TRANSC_SIMSNDW_TO_BKGRND_MEM_GROWCNT    4
#else
/********************Q Health -> BKGRND ***************************/
#define TRANSC_HEALTH_TO_BKGRND_Q_SZ            60
#define TRANSC_HEALTH_TO_BKGRND_MEM_SZ          20
#define TRANSC_HEALTH_TO_BKGRND_MEM_GROWCNT     3
/********************Q PKT PROC -> BKGRND ***************************/
#define TRANSC_PKTPRC_TO_BKGRND_Q_SZ            600
#define TRANSC_PKTPRC_TO_BKGRND_MEM_SZ          200
#define TRANSC_PKTPRC_TO_BKGRND_MEM_GROWCNT     3
/********************Q PKT PROC -> MIB ***************************/
#define TRANSC_PKTPRC_TO_MIB_Q_SZ               500
#define TRANSC_PKTPRC_TO_MIB_MEM_SZ             250
#define TRANSC_PKTPRC_TO_MIB_MEM_GROWCNT        2
/********************Q HTTP PROC -> BKGRND ***************************/
#define TRANSC_HTTPRC_TO_BKGRND_Q_SZ            600
#define TRANSC_HTTPRC_TO_BKGRND_MEM_SZ          200
#define TRANSC_HTTPRC_TO_BKGRND_MEM_GROWCNT     3
/********************Q HTTP PROC -> MIB ***************************/
#define TRANSC_HTTPPRC_TO_MIB_Q_SZ              500
#define TRANSC_HTTPPRC_TO_MIB_MEM_SZ            250
#define TRANSC_HTTPPRC_TO_MIB_MEM_GROWCNT       2
/********************Q PKT GEN -> BKGRND ***************************/
#define TRANSC_PKTGEN_TO_BKGRND_Q_SZ            200
#define TRANSC_PKTGEN_TO_BKGRND_MEM_SZ          100
#define TRANSC_PKTGEN_TO_BKGRND_MEM_GROWCNT     2
/********************Q PKT GEN -> MIB ***************************/
/* No Queue - the same thread as http proc */
/********************Q PKT PROC -> HTTP PROC ***************************/
#define TRANSC_PKTPRC_TO_HTTPRC_Q_SZ            500
#define TRANSC_PKTPRC_TO_HTTPRC_MEM_SZ          250
#define TRANSC_PKTPRC_TO_HTTPRC_MEM_GROWCNT     2
/********************Q HTTP PROC -> PKT GEN ***************************/
/* No Queue - the same thread as http proc */
/********************Q SIM PROC -> BKGRND PROC ***************************/
#define TRANSC_SIMPRC_TO_BKGRND_Q_SZ            400
#define TRANSC_SIMPRC_TO_BKGRND_MEM_SZ          200
#define TRANSC_SIMPRC_TO_BKGRND_MEM_GROWCNT     2
/********************Q HTTP PROC -> SIM PROC ***************************/
#define TRANSC_HTTPPRC_TO_SIM_Q_SZ              400
#define TRANSC_HTTPPRC_TO_SIM_MEM_SZ            200
#define TRANSC_HTTPPRC_TO_SIM_MEM_GROWCNT       2
/********************Q SIM PROC -> SIM/SND PROC ***************************/
#define TRANSC_SIMPRC_TO_SIMSNDW_Q_SZ           400
#define TRANSC_SIMPRC_TO_SIMSNDW_MEM_SZ         200
#define TRANSC_SIMPRC_TO_SIMSNDW_MEM_GROWCNT    2
/********************Q SIM/SND PROC -> SIM PROC ***************************/
#define TRANSC_SIMSNDW_TO_SIM_Q_SZ              400
#define TRANSC_SIMSNDW_TO_SIM_MEM_SZ            200
#define TRANSC_SIMSNDW_TO_SIM_MEM_GROWCNT       2
/********************Q SIM/SND PROC -> BKGRND PROC ***************************/
#define TRANSC_SIMSNDW_TO_BKGRND_Q_SZ           400
#define TRANSC_SIMSNDW_TO_BKGRND_MEM_SZ         100
#define TRANSC_SIMSNDW_TO_BKGRND_MEM_GROWCNT    4
#endif

/*************************************Q MSGS*************************************/
/********************Q MSG PKT PROC -> BKGRND *******************************/
/* See event logging evlog.h */
/********************Q MSG HTTP PROC -> BKGRND ******************************/
/* See event logging evlog.h */
/********************Q MSG PKT GEN -> BKGRND ********************************/
/* See event logging evlog.h */

/********************Q MSG PKT-PROC/HTTP PROC/PKT-GEN -> MIB ****************/
#define TRANSC_PPTOMIB_MSG_TABLE_SZ         50
#define TRANSC_PPTOMIB_TRAFMSG_TABLE_SZ     50
#define TRANSC_PPTOMIB_REDIRMSG_TABLE_SZ    50

#if (TRANSC_PPTOMIB_MSG_TABLE_SZ < TRANSC_PPTOMIB_TRAFMSG_TABLE_SZ)
#error "TRANSC_PPTOMIB_MSG_TABLE_SZ must be bigger than TRANSC_PPTOMIB_TRAFMSG_TABLE_SZ"
#endif

#if (TRANSC_PPTOMIB_MSG_TABLE_SZ < TRANSC_PPTOMIB_REDIRMSG_TABLE_SZ)
#error "TRANSC_PPTOMIB_MSG_TABLE_SZ must be bigger than TRANSC_PPTOMIB_REDIRMSG_TABLE_SZ"
#endif

struct _tc_qmsg_comptomib_s
{
    tc_iphdr_ipaddr_t      tIpAaddr;
    CHAR                   strDomain[64];
    CHAR                   strSvcName[64];
};
typedef struct _tc_qmsg_comptomib_s
               tc_qmsg_comptomib_t;

struct _tc_qmsgtbl_comptomib_s
{
    BOOL                   bIsRedir;
    U32                    nHttpMsgInfo;
    tc_qmsg_comptomib_t    tHttpMsgInfo[TRANSC_PPTOMIB_MSG_TABLE_SZ];
};
typedef struct _tc_qmsgtbl_comptomib_s
               tc_qmsgtbl_comptomib_t;

/********************Q MSG PKT PROC -> HTTP PROC***************************/
enum _tc_g_svctype_e
{
    tcHttpParseSvcTypeUnknown,
    tcHttpParseSvcTypeUrl,
    tcHttpParseSvcTypeSess,
};
typedef enum _tc_g_svctype_e
             tc_g_svctype_e;

struct _tc_g_svcdesc_s
{
    tc_g_svctype_e              svcType;
    U16                         nSvcTypeIdx;
    hlpr_httpsite_hndl          ePossibleSvcHostIdx;
};
typedef struct _tc_g_svcdesc_s
               tc_g_svcdesc_t;

/* Communication message
 * from pkt proc to http proc */
struct _tc_g_qmsgpptohp_s
{
    U32                 nHttpPktTotal;
    tc_g_svcdesc_t      svcType;
    CHAR                strOutIntfName[32];
    tc_pktdesc_t        pktDesc;
};
typedef struct _tc_g_qmsgpptohp_s
               tc_g_qmsgpptohp_t;

/********************Q MSG HTTP PROC -> PKT GEN ***************************/
#define TRANSC_QMSG_SVCNAMELEN         64
#define TRANSC_QMSG_HOSTNAMELEN        256
#define TRANSC_QMSG_STROPTLEN          64
#define TRANSC_QMSG_ORIGLEN            128
#define TRANSC_QMSG_IDLEN              256
#define TRANSC_QMSG_RGLEN              512
#define TRANSC_QMSG_MISCLEN            512
#define TRANSC_QMSG_UAGENTLEN          256
#define TRANSC_QMSG_SIMRGLEN           256
/* No Queue Yet */
/* Communication message
 * from http proc to pktgen */
struct _tc_g_qmsghptopg_s
{
    CHAR                            strHostName[TRANSC_QMSG_HOSTNAMELEN];
    CHAR                            strOptions[TRANSC_QMSG_STROPTLEN];
    CHAR                            strCId[TRANSC_QMSG_IDLEN];
    CHAR                            strCRange[TRANSC_QMSG_RGLEN];
    CHAR                            strCOrigin[TRANSC_QMSG_ORIGLEN];
    CHAR                            strCMisc[TRANSC_QMSG_MISCLEN];
    CHAR                            strSvcName[TRANSC_QMSG_SVCNAMELEN];
    CHAR                            strUAgent[TRANSC_QMSG_UAGENTLEN];
    CHAR                            strCSimRange[TRANSC_QMSG_SIMRGLEN];
    CHAR                            strOutIntfName[32];
    CHAR*                           pUrl; /* Must be change to buffer if being passed to Q */
    U32                             nUrlLen;
};
typedef struct _tc_g_qmsghptopg_s
               tc_g_qmsghptopg_t;

/********************Q MSG PKT PROC -> SIM ***************************/
#define TRANSC_SIM_URLBUFF      1536
#define TRANSC_SIM_RGBUFF       256
#define TRANSC_SIM_CIDBUFF      256
#define TRANSC_SIM_CMISCBUFF    512
#define TRANSC_SIM_CONTLENBUFF  128

struct _tc_qmsgtbl_pptosim_s
{
    CHAR                strCId[TRANSC_SIM_CIDBUFF];
    CHAR                strUrl[TRANSC_SIM_URLBUFF];
    CHAR                strCRange[TRANSC_SIM_RGBUFF];
    CHAR                strHttpHdrRange[TRANSC_SIM_RGBUFF];
    CHAR                strCMisc[TRANSC_SIM_CMISCBUFF];
    CHAR                strHostName[64];
    tc_iphdr_ipaddr_t   tSrcIP;
    U16                 nSrcPort;
    tc_iphdr_ipaddr_t   tDstIP;
    U16                 nDstPort;
    CHAR                strUAgent[256];
    CHAR                strRedirTarget[64];
    CHAR                strSvcType[16];
    CHAR                strSvcName[16];
    CHAR                strOptions[32];
    /* Redundant value no longer needed but used before */
    BOOL                bIsHttpRgReq;
};
typedef struct _tc_qmsgtbl_pptosim_s
               tc_qmsgtbl_pptosim_t;

/********************Q MSG SIM PROC -> SIM/SEND ***************************/
enum _tc_sim_rangetype_e
{
    tcRangeTypeUnknown = 0x0,
    tcRangeTypeDigitsToDigits,
    tcRangeTypeDigitsAndDash,
    tcRangeTypeMultiPartRanges
};
typedef enum _tc_sim_rangetype_e
             tc_sim_rangetype_e;

enum _tc_sim_reqtype_e
{
    tcSimReqTypeError = 0x0,
    tcSimReqTypeCkeyMap,
    tcSimReqTypeGetContentLen,
    tcSimReqTypeLog,
    tcSimReqTypeMiss
};
typedef enum _tc_sim_reqtype_e
             tc_sim_reqtype_e;

enum _tc_sim_msgpyldtype_e
{
    tcSimHttpMsgPyldTypeUNKNOWN=0,
    /*req*/
    tcSimHttpMsgReqPyldTypeGET,
    tcSimHttpMsgReqPyldTypeOPTION,
    tcSimHttpMsgReqPyldTypeHEAD,
    tcSimHttpMsgReqPyldTypePOST,
    tcSimHttpMsgReqPyldTypePUT,
    tcSimHttpMsgReqPyldTypeDELETE,
    tcSimHttpMsgReqPyldTypeTRACE,
    tcSimHttpMsgReqPyldTypeCONNECT,
    /*resp*/
    tcSimHttpMsgRespPyldTypeOK,
    tcSimHttpMsgRespPyldType206,
    tcSimHttpMsgRespPyldType302,
    tcSimHttpMsgRespPyldType303
};
typedef enum _tc_sim_msgpyldtype_e
             tc_sim_msgpyldtype_e;

struct _tc_qmsgtbl_simtosimsnd_s
{
    tc_qmsgtbl_pptosim_t    tMsg;
    CHAR                    strDynId[TRANSC_SIM_CIDBUFF];
    tc_sim_reqtype_e        eReqType;
    tc_sim_rangetype_e      eRgType;
    tc_sim_msgpyldtype_e    ePyldType;
    BOOL                    bLdCfgYaml;
};
typedef struct _tc_qmsgtbl_simtosimsnd_s
               tc_qmsgtbl_simtosimsnd_t;

struct _tc_qmsgtbl_simsndtosim_s
{
    CHAR                    strDynId[TRANSC_SIM_CIDBUFF];
    CHAR                    strSId[TRANSC_SIM_CIDBUFF];
};
typedef struct _tc_qmsgtbl_simsndtosim_s
               tc_qmsgtbl_simsndtosim_t;


#ifdef __cplusplus
}
#endif
#endif /* TCQMSG_H */
