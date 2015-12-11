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

#ifndef  TCLDCFG_H
#define  TCLDCFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tcgendefn.h"

/* Sim Outgoing interface length */
#define TRANSC_LDCFG_BWSIM_MODELEN                    8
#define TRANSC_LDCFG_BWSIM_WORKERSLEN                 16
#define TRANSC_LDCFG_BWSIM_OUTINTFLEN                 64
#define TRANSC_LDCFG_BWSIM_OUTINTFMACADDR             256
#define TRANSC_LDCFG_BWSIM_BODYPYLDLEN                1024

/* Sites max limits */
#define TRANSC_LDCFG_SITE_MAXURITYPE_LST              16
#define TRANSC_LDCFG_SITE_MAXSESSTYPE_LST             4
#define TRANSC_LDCFG_SITE_MAXSITES_LST                64
#define TRANSC_LDCFG_SITE_STRHOSTYPELEN               256
#define TRANSC_LDCFG_SITE_STRREFTYPELEN               256
#define TRANSC_LDCFG_SITE_STRSITESLEN                 256
#define TRANSC_LDCFG_SITE_STRHTTPHDRMATCHLEN          256
#define TRANSC_LDCFG_SITE_STROPTLEN                   64
#define TRANSC_LDCFG_SITE_STRTYPELEN                  64
#define TRANSC_LDCFG_SITE_STRSESSLEN                  1024
#define TRANSC_LDCFG_SITE_STRHTTPFLDLEN               256

/* URL max limits */
#define TRANSC_LDCFG_URL_STRURILEN                    1024
#define TRANSC_LDCFG_URL_STRMATCHSZLEN                128
#define TRANSC_LDCFG_URL_STRCKEYLEN                   1024
#define TRANSC_LDCFG_URL_STRCTXLEN                    1024
#define TRANSC_LDCFG_URL_STRCKEYRGLEN                 1024

/* Misc tables */
#define TRANSC_LDCFG_REDIRRATEMAX                     32
#define TRANSC_LDCFG_BLKLISTTBL_SZ                    64
#define TRANSC_LDCFG_SITESRVRTBL_SZ                   64
#define TRANSC_LDCFG_STRIPADDRLEN                     128

/***********************HTTP SESS****************************************/
struct _tc_ldcfg_httpsess_s
{
    /* /services/<session>/signature */
    CHAR                            strCfgArgSessSig[TRANSC_LDCFG_SITE_STRSESSLEN];
    /* /services/<session>/matchsize */
    CHAR                            strCfgArgMatchSz[TRANSC_LDCFG_URL_STRMATCHSZLEN];
    /* /services/<session>/contextid */
    CHAR                            strCfgArgCtxId[TRANSC_LDCFG_URL_STRCTXLEN];
    /* /services/<session>/cachekeyid */
    CHAR                            strCfgArgCKeyId[TRANSC_LDCFG_URL_STRCKEYLEN];
    /* /services/<session>/cachekeyrange */
    CHAR                            strCfgArgCKeyRange[TRANSC_LDCFG_URL_STRCKEYRGLEN];
    /* /services/<session>/cachekeymisc */
    CHAR                            strCfgArgCKeyMisc[TRANSC_LDCFG_URL_STRCKEYLEN];
};
typedef struct _tc_ldcfg_httpsess_s
               tc_ldcfg_httpsess_t;

/***********************HTTP URI****************************************/
struct _tc_ldcfg_httpuri_s
{
    /* /services/<uri>/signature */
    CHAR                            strCfgArgUriSig[TRANSC_LDCFG_URL_STRURILEN];
    /* /services/<uri>/matchsize */
    CHAR                            strCfgArgMatchSz[TRANSC_LDCFG_URL_STRMATCHSZLEN];
    /* /services/<uri>/contextid */
    CHAR                            strCfgArgCtxId[TRANSC_LDCFG_URL_STRCTXLEN];
    /* /services/<uri>/cachekeyid */
    CHAR                            strCfgArgCKeyId[TRANSC_LDCFG_URL_STRCKEYLEN];
    /* /services/<uri>/cachekeyrange */
    CHAR                            strCfgArgCKeyRange[TRANSC_LDCFG_URL_STRCKEYRGLEN];
    /* /services/<uri>/cachekeymisc */
    CHAR                            strCfgArgCKeyMisc[TRANSC_LDCFG_URL_STRCKEYLEN];
};
typedef struct _tc_ldcfg_httpuri_s
               tc_ldcfg_httpuri_t;

/***********************HTTP Sites****************************************/
struct _tc_ldcfg_httpsites_s
{
    /*/services/target site */
    CHAR                            strCfgArgSites[TRANSC_LDCFG_SITE_STRSITESLEN];
    BOOL                            bstrCfgArgSiteskip;
    /*/services/options */
    CHAR                            strCfgArgOpt[TRANSC_LDCFG_SITE_STROPTLEN];
    BOOL                            bstrCfgArgOptSkip;
    /*/services/Host */
    CHAR                            strCfgArgHost[TRANSC_LDCFG_SITE_STRHOSTYPELEN];
    BOOL                            bstrCfgArgHostSkip;
    /*/services/referrer */
    CHAR                            strCfgArgReferrer[TRANSC_LDCFG_SITE_STRREFTYPELEN];
    BOOL                            bstrCfgArgReferrerSkip;
    /*/services/url[] List */
    tc_ldcfg_httpuri_t              tCfgArgURI[TRANSC_LDCFG_SITE_MAXURITYPE_LST];
    BOOL                            bCfgArgURISkip;
    U16                             nCfgArgURI;
    /*/services/session */
    tc_ldcfg_httpsess_t             tCfgArgSess[TRANSC_LDCFG_SITE_MAXSESSTYPE_LST];
    BOOL                            btCfgArgSessSkip;
    U16                             nCfgArgSess;
    /*/services/type */
    CHAR                            strCfgArgType[TRANSC_LDCFG_SITE_STRTYPELEN];
    BOOL                            bstrCfgArgTypeSkip;
    /*/services/httprangefield */
    CHAR                            strCfgArgHttpRangeField[TRANSC_LDCFG_SITE_STRHTTPFLDLEN];
    BOOL                            bstrCfgArgHttpRangeFieldSkip;
    /*/services/httphdrmatch */
    CHAR                            strCfgArgHttpHdrMatch[TRANSC_LDCFG_SITE_STRHTTPHDRMATCHLEN];
    BOOL                            bstrCfgArgHttpHdrMatchSkip;
};
typedef struct _tc_ldcfg_httpsites_s
               tc_ldcfg_httpsites_t;

/***********************Ldable config.yaml****************************************/
struct _tc_ldcfg_conf_s
{
    /* /version */
    CHAR                    strVer[32];
    BOOL                    bStrVerSkip;
    /* /pcap-filter */
    CHAR                    strCmdArgPcapFilterRules[1024];
    BOOL                    bstrCmdArgPcapFilterRulesSkip;
    /* /redirectaddress */
    CHAR                    strCmdArgRedirTarget[512];
    BOOL                    bStrCmdArgRedirTargetSkip;
    /* /monitoringinterface */
    CHAR                    strCmdArgMonIntf[256];
    BOOL                    bStrCmdArgMonIntfSkip;
    /* /outgoinginterface */
    CHAR                    strCmdArgOutIntf[256];
    BOOL                    bstrCmdArgOutIntfSkip;
    /* /modeofoperation */
    CHAR                    strCmdArgModeOfOperation[32];
    BOOL                    bstrCmdArgModeOfOperationSkip;
    /* /ipblacklist */
    CHAR                    strCmdArgIpBlackList[1024];
    BOOL                    bstrCmdArgIpBlackListSkip;
    /* /bwsimulationmode */
    CHAR                    strCmdArgSimBwSimMode[TRANSC_LDCFG_BWSIM_MODELEN];
    BOOL                    bstrCmdArgSimBwSimModeSkip;
    /* /bwsimulationworkers */
    CHAR                    strCmdArgSimBwThreadsNum[TRANSC_LDCFG_BWSIM_WORKERSLEN];
    BOOL                    bstrCmdArgSimBwThreadsNumSkip;
    /* /bwsimulationoutgoinginterface */
    CHAR                    strCmdArgSimBwOutIntf[TRANSC_LDCFG_BWSIM_OUTINTFLEN];
    BOOL                    bstrCmdArgSimBwOutIntfSkip;
    /* /outgoinginterfacedestmac */
    CHAR                    strCmdArgOutIntfDestMacAddr[TRANSC_LDCFG_BWSIM_OUTINTFMACADDR];
    BOOL                    bstrCmdArgOutIntfDestMacAddrSkip;
    /* /outgoinginterfacesrcmac */
    CHAR                    strCmdArgOutIntfSrcMacAddr[TRANSC_LDCFG_BWSIM_OUTINTFMACADDR];
    BOOL                    bstrCmdArgOutIntfSrcMacAddrSkip;
    /* /outgoingredirreqratemax */
    CHAR                    strCmdArgOutRedirReqRateMax[TRANSC_LDCFG_REDIRRATEMAX];
    BOOL                    bstrCmdArgOutRedirReqRateMaxSkip;
    /* /outgoinginterfaceaddmplstag */
    CHAR                    strCmdArgOutIntfMplsLabel[16];
    BOOL                    bstrCmdArgOutIntfMplsLabelSkip;
    /* /ProcessProxyRequest */
    CHAR                    strCmdArgProcessProxyReq[16];
    BOOL                    bstrCmdArgProcessProxyReqSkip;
    /* /ProcessCORSRequest */
    CHAR                    strCmdArgIgnoreCORSReq[16];
    BOOL                    bstrCmdArgIgnoreCORSReqSkip;
    /* /mapinterface */
    CHAR                    strCmdArgMapIntf[256];
    BOOL                    bstrCmdArgMapIntfSkip;
    /*****Config FILE loading****/
    /* Sites */
    /* Reserve site idx:0 for unknown */
    /* /services/ */
    tc_ldcfg_httpsites_t    tSites[TRANSC_LDCFG_SITE_MAXSITES_LST];
    S32                     nSites;
};
typedef struct _tc_ldcfg_conf_s
               tc_ldcfg_conf_t;

/***********************Ldable sys.yaml****************************************/
struct _tc_sysldcfg_conf_s
{
    CHAR                    strVer[32];
    CHAR                    strModeOfOperation[32];
    CHAR                    strIntfType[1024];
    CHAR                    strSrcMACAddr[1024];
    CHAR                    strDestMACAddr[1024];
};
typedef struct _tc_sysldcfg_conf_s
               tc_ldsyscfg_conf_t;

/***********************main config****************************************/
struct _tc_ldcfg_s
{
    /* Hardcoded config values */
    BOOL                    bLockQ;
    int32_t                 healthCheck;
    BOOL                    bBlockTraffic;
    BOOL                    bDaemonize;
    FILE*                   pPidFile;
    CHAR                    strCmdArgSysYamlLoc[512];
    CHAR                    strCmdArgConfigYamlLoc[512];
    CHAR                    strCmdArgLogConfLoc[512];
    /* Loadable config.yaml */
    yaml_parser_t           tConfigYamlParser;
    tc_ldcfg_conf_t         tConfigYamlLdCfg;
    /* Loadable sys.yaml */
    yaml_parser_t           tSysYamlParser;
    tc_ldsyscfg_conf_t      tSysYamlLdCfg;
};
typedef struct _tc_ldcfg_s
               tc_ldcfg_t;

CCUR_PROTECTED(tresult_t)
tcLoadUnmarshallConfigYaml(
        evlog_t*                        pEvLogQ,
        evlog_desc_t*                   pLogDescSys,
        CHAR*                           strRdConfigLoc,
        tc_ldcfg_conf_t*                pLdCfg,
        yaml_parser_t*                  pYmlParser);

CCUR_PROTECTED(tresult_t)
tcLoadUnmarshallSysYaml(
        evlog_t*                        pEvLogQ,
        evlog_desc_t*                   pLogDescSys,
        CHAR*                           strRdConfigLoc,
        tc_ldsyscfg_conf_t*             pLdCfg,
        yaml_parser_t*                  pYmlParser);

#ifdef __cplusplus
}
#endif
#endif
