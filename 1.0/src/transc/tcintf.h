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

#ifndef  TCINTF_H
#define  TCINTF_H

#ifdef __cplusplus
extern "C" {
#endif

enum _tc_intf_ptype_e
{
    tcIntfPTypeNone=0,
    tcIntfPTypeRedirAddr
};
typedef enum _tc_intf_ptype_e
             tc_intf_ptype_e;

struct _tc_intf_intf_s
{
    CHAR                    strIntfName[32];
    CHAR                    strIntfVal[32];
    U16                     nIntfIdx;
    BOOL                    bIntfIdxSet;
    BOOL                    bIntfRdy;
};
typedef struct _tc_intf_intf_s
               tc_intf_intf_t;

struct _tc_intf_linkintf_s
{
    CHAR                    strIntfName[32];
    U16                     nRefCnt;
};
typedef struct _tc_intf_linkintf_s
               tc_intf_linkintf_t;

struct _tc_intf_config_s
{
    BOOL                        bActiveMode;
    CHAR                        strPktPrcArgRuleset[1024]; /* orig */
    CHAR                        strPktPrcArgMonIntf[512]; /* orig */
    CHAR                        strPktPrcArgOutIntf[64]; /* orig */
    CHAR                        strPktPrcArgMapIntf[256]; /* orig */
    CHAR                        strPktPrcArgRedirTarget[512];
    evlog_t*                    pEvLog;
    evlog_desc_t*               pLogDescSysX;
};
typedef struct _tc_intf_config_s
               tc_intf_config_t;

#ifdef __cplusplus
}
#endif
#endif
