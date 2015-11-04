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

#ifndef  _TCMONINTF_H
#define  _TCMONINTF_H

#ifdef __cplusplus
extern "C" {
#endif

struct _tc_monintf_fltr_s
{
    CHAR                    strRuleset[128];
#ifdef  TRANSC_BUILDCFG_LIBPCAP
    pcap_t*                 g_pPcap;
#else
    pfring*                 pRingMonIntf;
#endif
    BOOL                    bBpfFilterSet;
};
typedef struct _tc_monintf_fltr_s
               tc_monintf_fltr_t;

struct _tc_monintf_mon_s
{
    /* interface definitions */
    tc_intf_intf_t          tIntf;
    U16                     nRefCnt;
    CHAR                    strRedirAddr[64];
    /* pfring mon defn */
    U16                     nFilterTotal;
    U16                     nFilterActv;
    tc_monintf_fltr_t       tFilterTbl[TRANSC_INTERFACE_FLTR_MAX];
    struct pollfd           tMonIntfPollFdTbl[TRANSC_INTERFACE_FLTR_MAX];
};
typedef struct _tc_monintf_mon_s
               tc_monintf_mon_t;

struct _tc_monintf_out_s
{
    tc_intf_linkintf_t      tLinkIntf;
};
typedef struct _tc_monintf_out_s
               tc_monintf_out_t;

struct _tc_monintf_map_s
{
    tc_monintf_mon_t*        pMonIntf;
    tc_monintf_out_t*        pOutIntfH;
    BOOL                     bLinked; /* nIngressdx and nEgressIdx values linked */
   /* BOOL                     bIsModeActv*/; /* Mode of operation - unused*/
};
typedef struct _tc_monintf_map_s
               tc_monintf_map_t;

struct _tc_monintf_intfd_s
{
    BOOL                        bIntfCfgChanged;
    tc_monintf_out_t            tOutIntfTbl[TRANSC_INTERFACE_MAX];
    U16                         nOutIntfTotal;
    tc_monintf_mon_t            tMonIntfTbl[TRANSC_INTERFACE_MAX];
    U16                         nMonIntfTotal;
    U16                         nMonIntfOpen;
    BOOL                        bMonIntfOpen;
    BOOL                        bMonIntfUp;
    tc_monintf_map_t            tIntfMapTbl[TRANSC_INTERFACE_MAX];
    U16                         nIntfMapTblTotal;
    U32                         nTotalMonMappedRings;
};
typedef struct _tc_monintf_intfd_s
               tc_monintf_intfd_t;

CCUR_PROTECTED(tresult_t)
tcMonIntfInitMapCheck(
        tc_monintf_intfd_t*        pIntfd,
        tc_intf_config_t*          pIntfdCfg);

CCUR_PROTECTED(tresult_t)
tcMonIntfConfigYamlInitAllInterfaceTbls(
        tc_monintf_intfd_t*          pIntfd,
        tc_intf_config_t*            pIntfdCfg,
        tc_ldcfg_conf_t*             pLdCfg,
        BOOL                         bRxLoad,
        BOOL                         bTxLoad,
        BOOL                         bMapLoad,
        BOOL                         bRedirLoad);


#ifdef __cplusplus
}
#endif
#endif

