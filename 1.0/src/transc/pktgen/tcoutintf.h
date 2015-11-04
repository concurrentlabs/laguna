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

#ifndef  _TCOUTINTF_H
#define  _TCOUTINTF_H

#ifdef __cplusplus
extern "C" {
#endif

enum _tc_outintf_ptype_e
{
    tcOutIntfPTypeNone=0,
    tcOutIntfPTypeSMAC,
    tcOutIntfPTypeDMAC,
};
typedef enum _tc_outintf_ptype_e
             tc_outintf_ptype_e;

struct _tc_outintf_out_s
{
    /* interface definitions */
    tc_intf_intf_t          tIntf;
    U16                     nRefCnt;
    BOOL                    bIsRouter;
    U8                      pCmdArgSmacAddr[6];
    BOOL                    bSrcMACNotAutomatic;
    U8                      pCmdArgDmacAddr[6];
    BOOL                    bDstMACNotAutomatic;
    /* pfring out defn */
    pfring*                 pRingOutIntf;

};
typedef struct _tc_outintf_out_s
               tc_outintf_out_t;

struct _tc_outintf_mon_s
{
    tc_intf_linkintf_t      tLinkIntf;
    CHAR                    strRedirAddr[64];
};
typedef struct _tc_outintf_mon_s
               tc_outintf_mon_t;

struct _tc_outintf_map_s
{
    tc_outintf_mon_t*         pMonIntfH;
    tc_outintf_out_t*         pOutIntf;
    BOOL                      bLinked; /* nIngressdx and nEgressIdx values linked */
    BOOL                      bIsModeActv; /* Mode of operation */
};
typedef struct _tc_outintf_map_s
               tc_outintf_map_t;

struct _tc_outintf_intfd_s
{
    BOOL                        bIntfCfgChanged;
    tc_outintf_out_t            tOutIntfTbl[TRANSC_INTERFACE_MAX];
    U16                         nOutIntfTotal;
    U16                         nOutIntfOpen;
    BOOL                        bOutIntfOpen;
    BOOL                        bOutIntfUp;
    tc_outintf_mon_t            tMonIntfTbl[TRANSC_INTERFACE_MAX];
    U16                         nMonIntfTotal;
    tc_outintf_map_t            tIntfMapTbl[TRANSC_INTERFACE_MAX];
    U16                         nIntfMapTblTotal;
};
typedef struct _tc_outintf_intfd_s
               tc_outintf_intfd_t;

CCUR_PROTECTED(tresult_t)
tcOutIntfInitMapCheck(
        tc_outintf_intfd_t*        pIntfd,
        tc_intf_config_t*          pIntfdCfg);

CCUR_PROTECTED(void)
tcOutIntfSysYamlInitMACAddresses(
        tc_outintf_intfd_t*          pIntfd,
        tc_intf_config_t*            pIntfdCfg,
        tc_ldsyscfg_conf_t*          pLdCfg);

CCUR_PROTECTED(void)
tcOutIntfConfigYamlInitMACAddresses(
        tc_outintf_intfd_t*          pIntfd,
        tc_intf_config_t*            pIntfdCfg,
        tc_ldcfg_conf_t*             pLdCfg);

CCUR_PROTECTED(tresult_t)
tcOutIntfConfigYamlInitAllInterfaceTbls(
        tc_outintf_intfd_t*          pIntfd,
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

