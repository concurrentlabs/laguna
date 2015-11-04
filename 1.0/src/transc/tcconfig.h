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

#ifndef  TCCONFIG_H
#define  TCCONFIG_H

/* Remove this once we have real http packet */
#define TRANSC_PROCNAME                     "transc"
#define TRANSC_MAIN_CONF_VER                "1.4"
#define TRANSC_HTTP_MINPYLD_SZ              75
#define TRANSC_PIDFILE                      "/var/run/"TRANSC_PROCNAME".pid"
#define TRANSC_CCUR_REDIR_PREPEND           "/ccur"
/* Mon Interface and pcap-filter max */
#define TRANSC_PKTPRC_INTERFACE_MAX         10
#define TRANSC_PKTPRC_INTERFACE_FLTR_MAX    10
/* Default Number of simulation send worker threads */
#define TRANSC_SIMSENDW_THD_DFLT_NUM        15
/* Maximum Number of simulation threads */
#define TRANSC_SIM_THD_MAX                  256
/* Maximum cap for redirection rate */
#define TRANSC_REDIRRATE_MAX                5000
/* calculate pkt gen injection time */
#define TRANSC_STRMINJTIME                  FALSE
/* Simulation mode code included */
#define TRANSC_TCSIM                        TRUE
#define TRANSC_INTERFACE_MAX                10
#define TRANSC_INTERFACE_FLTR_MAX           TRANSC_INTERFACE_MAX
/* Thread Stacks */
#define TRANSC_PKTPRC_THD_STACK             MTHREAD_DEFAULT_STACK_SIZE
#define TRANSC_HEALTH_THD_STACK             MTHREAD_DEFAULT_STACK_SIZE
#define TRANSC_HTTPPRC_THD_STACK            MTHREAD_DEFAULT_STACK_SIZE
#define TRANSC_SIM_THD_STACK                MTHREAD_DEFAULT_STACK_SIZE
#define TRANSC_SIMSNDW_THD_STACK            MTHREAD_DEFAULT_STACK_SIZE
/* Logging logical names */
#define TRANSC_LOGCLASS_COMP_BACKGRND       "tr_comp_sys"
#define TRANSC_LOGCLASS_COMP_PKTPRC         "tr_comp_pktprc"
#define TRANSC_LOGCLASS_COMP_SIMPRC         "tr_comp_sim_"
#define TRANSC_LOGCLASS_COMP_HTTPPRC        "tr_comp_httpprc"
#define TRANSC_LOGCLASS_COMP_PKTGEN         "tr_comp_pktgen"
#define TRANSC_LOGCLASS_COMP_MIB            "tr_comp_mib"
#define TRANSC_LOGCLASS_COMP_HEALTH         "tr_comp_hlth"
#define TRANSC_LOGCLASS_SVC_HTTPROC         "tr_svc_pktprc"
#define TRANSC_LOGCLASS_SVC_PKTGEN          "tr_svc_pktgen"
/* tr_svc_sim_main, tr_svc_sim_worker_1...6 */
#define TRANSC_LOGCLASS_SVC_SIMPRC          "tr_svc_sim_"

/* Available TR (Transparent Routing) Components:
 * -Pkt processing
 * -Http Processing
 * -Pkt generation
 * -Simulation and its workers
 * -Monitor (background)
 * -Mib (background)
 * -Logging (background) */
enum _tc_tr_comptype_e
{
    tcTRCompTypeLog,
    tcTRCompTypePktPrc,
    tcTRCompTypeHttpPrc,
    tcTRCompTypePktGen,
    tcTRCompTypeSimMgrChldrn,
    tcTRCompTypeSimMgr,
    tcTRCompTypeMib,
    tcTRCompTypeHealth,
    tcTRCompTypeMax
};
typedef enum _tc_tr_comptype_e
             tc_tr_comptype_e;

/* TCS status */
enum _tc_tr_sts_e
{
    tcTrStsDown = 0x0,
    tcTrStsUp
};
typedef enum _tc_tr_sts_e
             tc_tr_sts_e;

enum _tc_tr_tcplanesnmpcomptype_e
{
    tcTRPlaneSnmpCompTypePktPrc,
    tcTRPlaneSnmpCompTypeHttpPrc,
    tcTRPlaneSnmpCompTypeMax
};
typedef enum _tc_tr_tcplanesnmpcomptype_e
             tc_tr_tcplanesnmpcomptype_e;

#define TRANSC_MIB_QUEUE_MAX            tcTRPlaneSnmpCompTypeMax

/* Third party libraries minimal compatibility versions */
#define TRANSC_COMPAT_PFRING_VER        0x060002
#define TRANSC_COMPAT_CZMQ_VER          20200
#define TRANSC_COMPAT_ZMQ_VER           40004

#endif /*TCCONFIG_H*/
