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

#ifndef  TCMAPINTF_H
#define  TCMAPINTF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tcldcfg.h"
#include "tcintf.h"
#include "pktprc/tcmonintf.h"
#include "pktgen/tcoutintf.h"

CCUR_PROTECTED(U32)
tcIntfInitIntfKeyTable(
        tc_monintf_intfd_t*          pIngressIntf,
        tc_outintf_intfd_t*          pEgressIntf,
        tc_utils_keyvalue_t*          pIntfKeyValTbl,
        U16                          nIntfKeyValTbl);

CCUR_PROTECTED(U32)
tcIntfInitIntfTable(
        tc_monintf_intfd_t*          pIngressIntf,
        tc_outintf_intfd_t*          pEgressIntf,
        tc_utils_keyvalue_t*          pIntfKeyValTbl,
        U16                          nIntfKeyValTbl);


CCUR_PROTECTED(void)
tcIntfCkIntf(
        tc_intf_config_t*            pIntfCfg,
        tc_ldcfg_conf_t*             pLdCfg,
        BOOL*                        bRxLoad,
        BOOL*                        bTxLoad,
        BOOL*                        bMapLoad,
        BOOL*                        bRedirLoad);

#ifdef __cplusplus
}
#endif
#endif
