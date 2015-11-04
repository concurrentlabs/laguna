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

#ifndef TCUTILS_H
#define TCUTILS_H

#ifdef __cplusplus
extern "C" {
#endif


struct _tc_utils_keyvalue_s
{
    CHAR                    strKey[128];
    CHAR                    strValue[256];
};
typedef struct _tc_utils_keyvalue_s
               tc_utils_keyvalue_t;

CCUR_PROTECTED(CHAR*)
tcUtilsGetPfringVer(
        CHAR* strBuff,I32 version);

CCUR_PROTECTED(tresult_t)
tcUtilsDaemonize();

CCUR_PROTECTED(void)
tcUtilsSignalHandlerSetup(void);

CCUR_PROTECTED(void)
tcUtilsSignalHandler(
        I32 nSignalId);

CCUR_PROTECTED(void)
tcUtilsPopulateKeyValuePair(
        tc_utils_keyvalue_t*          pKeyValTbl,
        U16*                         nKeyValTblActive,
        U16                          nKeyValTblMax,
        CHAR*                        strCmdArgMonIntf);

CCUR_PROTECTED(BOOL)
tcUtilsValidConfigVersion(CHAR* strVer);

CCUR_PROTECTED(void)
tcUtilsLogW3cFormat(
        U32                         tid,
        evlog_loglvl_e              lvl,
        tc_g_qmsghptopg_t*          pInjMsg,
        tc_pktdesc_t*               pPktDesc,
        evlog_t*                    pEvLog,
        evlog_desc_t*               pLogDescSys);

#ifdef __cplusplus
}
#endif
#endif /* TCUTILS_H */
