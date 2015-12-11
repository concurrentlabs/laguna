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

#include <czmq.h>
#include "tcinit.h"

#if (RING_VERSION_NUM < TRANSC_COMPAT_PFRING_VER)
#error "Invalid pfring library version! RING_VERSION_NUM < TRANSC_COMPAT_PFRING_VER"
#endif

#if (CZMQ_VERSION < TRANSC_COMPAT_CZMQ_VER)
#error "Invalid czmq library version! CZMQ_VERSION < TRANSC_COMPAT_CZMQ_VER"
#endif

#if (ZMQ_VERSION < TRANSC_COMPAT_ZMQ_VER)
#error "Invalid czmq library version! ZMQ_VERSION < TRANSC_COMPAT_ZMQ_VER"
#endif
/* Context values passed into each thread */
static tc_gd_thread_ctxt_t       g_ThdCntx;
extern void * tcplane_snmp(void * context);

/**************** PRIVATE Functions **********************/
/***************************************************************************
 * function: _tcInitSharedQ
 *
 * description: Init shared queue between threads.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitSharedQ(tc_gd_thread_ctxt_t*     pCntx)
{
    tresult_t                   _result;
    U16                         _ibkgrnd;
    U16                         _i;

    do
    {
        _result = EFAILURE;
        /*----------------Logging Queues------------------------*/
        /* Create health -> Bkgrnd thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQLogTbl[tcBkgrndCompTypeHealth].tLkfq),
                sizeof(evlog_strblk_t),
                TRANSC_HEALTH_TO_BKGRND_Q_SZ,
                TRANSC_HEALTH_TO_BKGRND_MEM_SZ,
                TRANSC_HEALTH_TO_BKGRND_MEM_GROWCNT,
                pCntx->tHealthThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlFatal,
                    "unable to create Q Health to Bkgrnd\n");
            break;
        }
        /* Create Pkt proc -> Bkgrnd thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQLogTbl[tcBkgrndCompTypePktPrc].tLkfq),
                sizeof(evlog_strblk_t),
                TRANSC_PKTPRC_TO_BKGRND_Q_SZ,
                TRANSC_PKTPRC_TO_BKGRND_MEM_SZ,
                TRANSC_PKTPRC_TO_BKGRND_MEM_GROWCNT,
                pCntx->tMibThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlFatal,
                    "unable to create Q pkt prc to Bkgrnd\n");
            break;
        }
        /* Create Http Proc -> Bkgrnd thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQLogTbl[tcBkgrndCompTypeHttpPrc].tLkfq),
                sizeof(evlog_strblk_t),
                TRANSC_HTTPRC_TO_BKGRND_Q_SZ,
                TRANSC_HTTPRC_TO_BKGRND_MEM_SZ,
                TRANSC_HTTPRC_TO_BKGRND_MEM_GROWCNT,
                pCntx->tMibThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlFatal,
                    "unable to create Q http prc to Bkgrnd\n");
            break;
        }
        /* Create PktGen -> Bkgrnd thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQLogTbl[tcBkgrndCompTypePktGen].tLkfq),
                sizeof(evlog_strblk_t),
                TRANSC_PKTGEN_TO_BKGRND_Q_SZ,
                TRANSC_PKTGEN_TO_BKGRND_MEM_SZ,
                TRANSC_PKTGEN_TO_BKGRND_MEM_GROWCNT,
                pCntx->tMibThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlFatal,
                    "unable to create Q Pkt gen to Bkgrnd\n");
            break;
        }
        /* Create Mib -> Bkgrnd thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQLogTbl[tcBkgrndCompTypeMib].tLkfq),
                sizeof(evlog_strblk_t),
                TRANSC_PKTGEN_TO_BKGRND_Q_SZ,
                TRANSC_PKTGEN_TO_BKGRND_MEM_SZ,
                TRANSC_PKTGEN_TO_BKGRND_MEM_GROWCNT,
                pCntx->tMibThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlFatal,
                    "unable to create Q Mib to Bkgrnd\n");
            break;
        }
#if TRANSC_TCSIM
        /* Create Sim -> Bkgrnd Event Log thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQLogTbl[tcBkgrndCompTypeSimMgr].tLkfq),
                sizeof(evlog_strblk_t),
                TRANSC_SIMPRC_TO_BKGRND_Q_SZ,
                TRANSC_SIMPRC_TO_BKGRND_MEM_SZ,
                TRANSC_SIMPRC_TO_BKGRND_MEM_GROWCNT,
                pCntx->tMibThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlFatal,
                    "unable to create Sim to Bkgrnd Event Log Queue\n");
            break;
        }
        /* Create sim\Sim-send-> Bkgrnd thread Event Log Queue */
        for(_i=tcBkgrndCompTypeMax;_i<pCntx->tBkGnThd.nQEvLogTbl;_i++)
        {
            _result = lkfqCrQueue(
                    &(pCntx->tQLogTbl[_i].tLkfq),
                    sizeof(evlog_strblk_t),
                    TRANSC_SIMSNDW_TO_BKGRND_Q_SZ,
                    TRANSC_SIMSNDW_TO_BKGRND_MEM_SZ,
                    TRANSC_SIMSNDW_TO_BKGRND_MEM_GROWCNT,
                    pCntx->tMibThd.tConfig.bLockQ);
            if(ESUCCESS != _result)
            {
                evLogTraceSys(evLogLvlFatal,
                        "unable to create sim to Bkgrnd Event Log Queue\n");
                break;
            }
        }
#endif /* TRANSC_TCSIM */
        /*----------------MIB Queues------------------------*/
        /* Create Pkt prc -> Mib thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQMibTbl[tcTRPlaneSnmpCompTypePktPrc]),
                sizeof(tc_qmsgtbl_comptomib_t),
                TRANSC_PKTPRC_TO_MIB_Q_SZ,
                TRANSC_PKTPRC_TO_MIB_MEM_SZ,
                TRANSC_PKTPRC_TO_MIB_MEM_GROWCNT,
                pCntx->tMibThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlFatal,
                    "unable to create Bpool Pkt Proc to mib Prc\n");
            break;
        }
        /* Create Http prc -> Mib thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQMibTbl[tcTRPlaneSnmpCompTypeHttpPrc]),
                sizeof(tc_qmsgtbl_comptomib_t),
                TRANSC_HTTPPRC_TO_MIB_Q_SZ,
                TRANSC_HTTPPRC_TO_MIB_MEM_SZ,
                TRANSC_HTTPPRC_TO_MIB_MEM_GROWCNT,
                pCntx->tMibThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlFatal,
                    "unable to create Bpool Http Proc to mib Prc\n");
            break;
        }
        /* No pkt Gen -> Mib Thread Queue */
        /* Create Pkt prc -> http proc thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQPPktProcToHttpProc),
                sizeof(tc_g_qmsgpptohp_t),
                TRANSC_PKTPRC_TO_HTTPRC_Q_SZ,
                TRANSC_PKTPRC_TO_HTTPRC_MEM_SZ,
                TRANSC_PKTPRC_TO_HTTPRC_MEM_GROWCNT,
                pCntx->tMibThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlFatal,
                    "unable to create Bpool Pkt Proc to Http Prc\n");
            break;
        }
#if TRANSC_TCSIM
        /* Create Htt Proc -> sim thread Queue */
        _result = lkfqCrQueue(
                &(pCntx->tQHttpProcToSim),
                sizeof(tc_qmsgtbl_pptosim_t),
                TRANSC_HTTPPRC_TO_SIM_Q_SZ,
                TRANSC_HTTPPRC_TO_SIM_MEM_SZ,
                TRANSC_HTTPPRC_TO_SIM_MEM_GROWCNT,
                pCntx->tMibThd.tConfig.bLockQ);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlFatal,
                    "unable to create Pkt Prc to Sim Queue\n");
            break;
        }
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
        {
            /* Create Sim -> sim\sim-send threads Queue */
            _result = lkfqCrQueue(
                    &(pCntx->tQSimToSimSendWTbl[_i]),
                    sizeof(tc_qmsgtbl_simtosimsnd_t),
                    TRANSC_SIMPRC_TO_SIMSNDW_Q_SZ,
                    TRANSC_SIMPRC_TO_SIMSNDW_MEM_SZ,
                    TRANSC_SIMPRC_TO_SIMSNDW_MEM_GROWCNT,
                    pCntx->tMibThd.tConfig.bLockQ);
            if(ESUCCESS != _result)
            {
                evLogTraceSys(evLogLvlFatal,
                        "unable to create sim to sim send Workers Queue\n");
                break;
            }
            /* Create sim\sim-send threads -> sim thread Queue */
            _result = lkfqCrQueue(
                    &(pCntx->tQSimSendWToSimTbl[_i]),
                    sizeof(tc_qmsgtbl_simsndtosim_t),
                    TRANSC_SIMSNDW_TO_SIM_Q_SZ,
                    TRANSC_SIMSNDW_TO_SIM_MEM_SZ,
                    TRANSC_SIMSNDW_TO_SIM_MEM_GROWCNT,
                    pCntx->tMibThd.tConfig.bLockQ);
            if(ESUCCESS != _result)
            {
                evLogTraceSys(evLogLvlFatal,
                        "unable to create sim send Workers to sim Queue\n");
                break;
            }
        }

#endif /* TRANSC_TCSIM */
        /* Setup Queues */
        /* Pkt Proc -> Http Proc */
        pCntx->tPktPrcThd.pQPPktProcToHttpProc  =
                &(pCntx->tQPPktProcToHttpProc);
        pCntx->tHttpPrcThd.pQPPktProcToHttpProc =
                &(pCntx->tQPPktProcToHttpProc);
        /* Pkt Proc -> Mib */
        pCntx->tPktPrcThd.pQPktProcToMib  =
                &(pCntx->tQMibTbl[tcTRPlaneSnmpCompTypePktPrc]);
        pCntx->tMibThd.pQMib[tcTRPlaneSnmpCompTypePktPrc] =
                &(pCntx->tQMibTbl[tcTRPlaneSnmpCompTypePktPrc]);
        /* Http Proc -> Mib */
        pCntx->tHttpPrcThd.pQHttpProcToMib  =
                &(pCntx->tQMibTbl[tcTRPlaneSnmpCompTypeHttpPrc]);
        pCntx->tMibThd.pQMib[tcTRPlaneSnmpCompTypeHttpPrc] =
                &(pCntx->tQMibTbl[tcTRPlaneSnmpCompTypeHttpPrc]);
        /* Health -> Bkgrnd Proc */
        pCntx->tHealthThd.pQHealthToBkgrnd =
                &(pCntx->tQLogTbl[tcBkgrndCompTypeHealth]);
        pCntx->tBkGnThd.pQEvLogTbl[tcBkgrndCompTypeHealth] =
                &(pCntx->tQLogTbl[tcBkgrndCompTypeHealth]);
        /* Pkt Proc -> Bkgrnd Proc */
        pCntx->tPktPrcThd.pQPktProcToBkgrnd =
                &(pCntx->tQLogTbl[tcBkgrndCompTypePktPrc]);
        pCntx->tBkGnThd.pQEvLogTbl[tcBkgrndCompTypePktPrc] =
                &(pCntx->tQLogTbl[tcBkgrndCompTypePktPrc]);
        /* Http Proc -> Bkgrnd Proc */
        pCntx->tHttpPrcThd.pQHttpProcToBkgrnd =
                &(pCntx->tQLogTbl[tcBkgrndCompTypeHttpPrc]);
        pCntx->tBkGnThd.pQEvLogTbl[tcBkgrndCompTypeHttpPrc] =
                &(pCntx->tQLogTbl[tcBkgrndCompTypeHttpPrc]);
        /* Pkt Gen -> Bkgrnd Proc */
        pCntx->tPktGenThd.pQPktGenToBkgrnd =
                &(pCntx->tQLogTbl[tcBkgrndCompTypePktGen]);
        pCntx->tBkGnThd.pQEvLogTbl[tcBkgrndCompTypePktGen] =
                &(pCntx->tQLogTbl[tcBkgrndCompTypePktGen]);
        /* Mib Proc -> Bkgrnd Proc */
        pCntx->tMibThd.pQMibToBkgrnd =
                &(pCntx->tQLogTbl[tcBkgrndCompTypeMib]);
        pCntx->tBkGnThd.pQEvLogTbl[tcBkgrndCompTypeMib] =
                &(pCntx->tQLogTbl[tcBkgrndCompTypeMib]);
        /* Sim -> Bkgrnd logging */
        pCntx->tSimThd.pQSimToBkgrnd =
                &(pCntx->tQLogTbl[tcBkgrndCompTypeSimMgr]);
        pCntx->tBkGnThd.pQEvLogTbl[tcBkgrndCompTypeSimMgr] =
                &(pCntx->tQLogTbl[tcBkgrndCompTypeSimMgr]);
        _ibkgrnd=tcBkgrndCompTypeMax;
        /* sim\simsend-> Bkgrnd logging */
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
        {
            if(_ibkgrnd > pCntx->tBkGnThd.nQEvLogTbl)
            {
                _result = EFAILURE;
                evLogTraceSys(evLogLvlFatal,
                        "unable to create sim/sim-send to Bkgrnd Event Log Queue\n");
                break;
            }
            pCntx->tSimSndThdTbl[_i].pQSimSendWToBkgrnd =
                    &(pCntx->tQLogTbl[_ibkgrnd]);
            pCntx->tBkGnThd.pQEvLogTbl[_ibkgrnd] =
                    &(pCntx->tQLogTbl[_ibkgrnd]);
            _ibkgrnd++;
        }
#if TRANSC_TCSIM
        /* Http Proc -> sim */
        pCntx->tHttpPrcThd.pQHttpProcToSim =
                &(pCntx->tQHttpProcToSim);
        pCntx->tSimThd.pQHttpProcToSim =
                &(pCntx->tQHttpProcToSim);
        /* Sim -> sim\sim-send */
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
        {
            pCntx->tSimThd.pQSimToSimSendWTbl[_i] =
                    &(pCntx->tQSimToSimSendWTbl[_i]);
            pCntx->tSimSndThdTbl[_i].pQSimToSimSendW =
                    &(pCntx->tQSimToSimSendWTbl[_i]);
        }
        /* sim\sim-send-> sim thread */
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
        {
            pCntx->tSimSndThdTbl[_i].pQSimSendWToSim =
                    &(pCntx->tQSimSendWToSimTbl[_i]);
            pCntx->tSimThd.pQSimSendWToSimTbl[_i] =
                    &(pCntx->tQSimSendWToSimTbl[_i]);
        }
#endif /* TRANSC_TCSIM */
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcInitHealthThreadRes
 *
 * description: Init Health thread resources.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitHealthThreadRes(
        tc_health_thread_ctxt_t*             pCntx,
        tc_bkgrnd_evlogtbl_t*                pZLogCatCompTbl,
        tc_ldcfg_conf_t*                     pLdConfigYamlCfg,
        tc_ldsyscfg_conf_t*                  pLdSysYamlCfg,
        int32_t                              healthCheck)
{
    tresult_t   _result;
    CHAR        _strBuff[512];

    CCURASSERT(pCntx);
    CCURASSERT(pZLogCatCompTbl);
    CCURASSERT(pLdConfigYamlCfg);
    CCURASSERT(pLdSysYamlCfg);
    CCURASSERT(pZLogCatCompTbl->nZLogCat == 0 ||
               pZLogCatCompTbl->nZLogCat >= TRANSC_BKGRNDLOG_QUEUE_MAX);
    do
    {
        pCntx->healthCheck = healthCheck;
        /* Init mib component sys logging */
        _result = evLogOpenLogicalLog(
               pZLogCatCompTbl->tZLogCat,
               pZLogCatCompTbl->nZLogCat,
               &(pCntx->tLogDescSys),
               evLogLvlClassCompSys,
               TRANSC_LOGCLASS_COMP_HEALTH);
        if(ESUCCESS != _result)
        {
           evLogTraceSys(evLogLvlError,
              "Error, Mib Shared resource init");
        }
        _result = mSemCondVarSemCreate(
                &(pCntx->tCondVarSem),
                _strBuff,
                sizeof(_strBuff)-1);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlError,
                   "%s",
                   _strBuff);
            break;
        }
        pCntx->bCondVarSet = TRUE;
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcInitMibThreadRes
 *
 * description: Init Mib thread resources.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitMibThreadInitRes(
        tc_tcplanesnmp_thread_ctxt_t*        pCntx,
        tc_bkgrnd_evlogtbl_t*                pZLogCatCompTbl,
        tc_ldcfg_conf_t*                     pLdConfigYamlCfg,
        tc_ldsyscfg_conf_t*                  pLdSysYamlCfg)
{
    tresult_t   _result;
    CHAR        _strBuff[512];

    CCURASSERT(pCntx);
    CCURASSERT(pZLogCatCompTbl);
    CCURASSERT(pLdConfigYamlCfg);
    CCURASSERT(pLdSysYamlCfg);
    CCURASSERT(pZLogCatCompTbl->nZLogCat == 0 ||
               pZLogCatCompTbl->nZLogCat >= TRANSC_BKGRNDLOG_QUEUE_MAX);
    do
    {
        /* Init mib component sys logging */
        _result = evLogOpenLogicalLog(
               pZLogCatCompTbl->tZLogCat,
               pZLogCatCompTbl->nZLogCat,
               &(pCntx->tLogDescSys),
               evLogLvlClassCompSys,
               TRANSC_LOGCLASS_COMP_MIB);
        if(ESUCCESS != _result)
        {
           evLogTraceSys(evLogLvlError,
              "Error, Mib Shared resource init");
        }
        _result = mSemCondVarSemCreate(
                &(pCntx->tCondVarSem),
                _strBuff,
                sizeof(_strBuff)-1);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlError,
                   "%s",
                   _strBuff);
            break;
        }
        pCntx->bCondVarSet = TRUE;
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcInitBkGrndThreadInitRes
 *
 * description: Init background thread resources.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitBkGrndThreadInitRes(
        tc_bkgrnd_thread_ctxt_t*     pCntx,
        tc_bkgrnd_evlogtbl_t*        pZLogCatCompTbl,
        tc_ldcfg_conf_t*             pLdConfigYamlCfg,
        tc_ldsyscfg_conf_t*          pLdSysYamlCfg)
{
    pCntx->tZLogCatComp.nZLogCat  = TRANSC_BKGRNDLOG_QUEUE_MAX;
    pCntx->tZLogCatSvc.nZLogCat   = TRANSC_BKGRNDLOG_QUEUE_MAX;
    pCntx->tid = (U32)pthread_self();
    /* Sets up signal handler,
     * this will get overriden by zeromq but if not,
     * then use this signal handler setup.*/
    tcUtilsSignalHandlerSetup();
    tcShProtectedDMsgSetCompInitRdy(tcTRCompTypeLog,TRUE);
    tcShProtectedDSetCompSts(tcTRCompTypeLog,tcTrStsUp);
    return ESUCCESS;
}

/***************************************************************************
 * function: _tcInitPktPrcThreadInitRes
 *
 * description: Init thread resources. This function is being called only
 * once at startup.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitPktPrcThreadInitRes(
        tc_pktprc_thread_ctxt_t*     pCntx,
        tc_bkgrnd_evlogtbl_t*        pZLogCatCompTbl,
        tc_ldcfg_conf_t*             pLdConfigYamlCfg,
        tc_ldsyscfg_conf_t*          pLdSysYamlCfg)
{
    tresult_t                   _result;
    CHAR                        _strBuff[512];

    CCURASSERT(pCntx);
    CCURASSERT(pLdConfigYamlCfg);
    CCURASSERT(pLdSysYamlCfg);

    do
    {
        /* Init pkt proc component sys logging */
       _result = evLogOpenLogicalLog(
               pZLogCatCompTbl->tZLogCat,
               pZLogCatCompTbl->nZLogCat,
               &(pCntx->tLogDescSys),
               evLogLvlClassCompSys,
               TRANSC_LOGCLASS_COMP_PKTPRC);
       if(ESUCCESS != _result)
       {
           evLogTraceSys(evLogLvlError,
              "Error, Pkt Proc Shared resource init");
           break;
       }
        _result = mSemCondVarSemCreate(
                &(pCntx->tCondVarSem),
                _strBuff,
                sizeof(_strBuff)-1);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlError,
                   "%s\n",
                   _strBuff);
            break;
        }
        pCntx->bCondVarSet = TRUE;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcInitHttpThreadInitRes
 *
 * description: Init thread resources. This function is being called only
 * once at startup.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitHttpThreadInitRes(
        tc_httpprc_thread_ctxt_t*     pCntx,
        tc_bkgrnd_evlogtbl_t*         pZLogCatCompTbl,
        tc_bkgrnd_evlogtbl_t*         pZLogCatSvcTbl,
        tc_ldcfg_conf_t*              pLdConfigYamlCfg,
        tc_ldsyscfg_conf_t*           pLdSysYamlCfg)
{
    tresult_t                   _result;
    CHAR                        _strBuff[512];

    CCURASSERT(pCntx);
    CCURASSERT(pLdConfigYamlCfg);
    CCURASSERT(pLdSysYamlCfg);

    do
    {
        /* Init http proc component sys logging */
        _result = evLogOpenLogicalLog(
                pZLogCatCompTbl->tZLogCat,
                pZLogCatCompTbl->nZLogCat,
               &(pCntx->tLogDescSys),
               evLogLvlClassCompSys,
               TRANSC_LOGCLASS_COMP_HTTPPRC);
        if(ESUCCESS != _result)
        {
           evLogTraceSys(evLogLvlError,
              "Error, Http Proc Shared resource init");
           break;
        }
        /* Init http proc Class services logging */
        _result = evLogOpenLogicalLog(
              pZLogCatSvcTbl->tZLogCat,
              pZLogCatSvcTbl->nZLogCat,
              &(pCntx->tLogDescSvc),
              evLogLvlClassServices,
              TRANSC_LOGCLASS_SVC_HTTPROC);
        if(ESUCCESS != _result)
        {
          evLogTraceSys(evLogLvlError,
             "Error, Pkt Gen Shared resource init");
          break;
        }
        _result = mSemCondVarSemCreate(
                &(pCntx->tCondVarSem),
                _strBuff,
                sizeof(_strBuff)-1);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlError,
                   "%s\n",
                   _strBuff);
            break;
        }
        pCntx->bCondVarSet = TRUE;
    }while(FALSE);

    return _result;

}

/***************************************************************************
 * function: _tcInitPktGenThreadInitRes
 *
 * description: Init thread resources. This function is being called only
 * once at startup.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitPktGenThreadInitRes(
        tc_pktgen_thread_ctxt_t*     pCntx,
        tc_bkgrnd_evlogtbl_t*        pZLogCatCompTbl,
        tc_bkgrnd_evlogtbl_t*        pZLogCatSvcTbl,
        tc_ldcfg_conf_t*             pLdConfigYamlCfg,
        tc_ldsyscfg_conf_t*          pLdSysYamlCfg,
        BOOL                         bBlockTraffic)
{
    tresult_t                   _result;
#if PKTGEN_THD
    CHAR                        _strBuff[512];
#endif

    CCURASSERT(pCntx);
    CCURASSERT(pLdConfigYamlCfg);
    CCURASSERT(pLdSysYamlCfg);

    do
    {
        pCntx->bBlockTraffic = bBlockTraffic;
        /* Init pkt gen component sys logging */
       _result = evLogOpenLogicalLog(
               pZLogCatCompTbl->tZLogCat,
               pZLogCatCompTbl->nZLogCat,
               &(pCntx->tLogDescSys),
               evLogLvlClassCompSys,
               TRANSC_LOGCLASS_COMP_PKTGEN);
       if(ESUCCESS != _result)
       {
           evLogTraceSys(evLogLvlError,
              "Error, Pkt Gen Shared resource init");
           break;
       }
       /* Init Pkt gen Class services logging */
       _result = evLogOpenLogicalLog(
             pZLogCatSvcTbl->tZLogCat,
             pZLogCatSvcTbl->nZLogCat,
             &(pCntx->tLogDescSvc),
             evLogLvlClassServices,
             TRANSC_LOGCLASS_SVC_PKTGEN);
       if(ESUCCESS != _result)
       {
         evLogTraceSys(evLogLvlError,
            "Error, Http Proc Shared resource init");
         break;
       }
#if PKTGEN_THD
        _result = mSemCondVarSemCreate(
                &(pCntx->tCondVarSem),
                _strBuff,
                sizeof(_strBuff)-1);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlError,
                   "%s\n",
                   _strBuff);
            break;
        }
        pCntx->bCondVarSet = TRUE;
#else
        _result = ESUCCESS;
#endif
    }while(FALSE);

    return _result;

}

/***************************************************************************
 * function: _tcInitSimThreadInitRes
 *
 * description: Init sim thread resources. This function is being called
 * from background thread context.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitSimThreadInitRes(
        tc_sim_thread_ctxt_t*        pCntx,
        tc_bkgrnd_evlogtbl_t*        pZLogCatCompTbl,
        tc_bkgrnd_evlogtbl_t*        pZLogCatSvcTbl,
        tc_ldcfg_conf_t*             pLdConfigYamlCfg,
        tc_ldsyscfg_conf_t*          pLdSysYamlCfg)
{
    tresult_t                   _result;
    CHAR                        _strBuff[256];

    CCURASSERT(pCntx);
    CCURASSERT(pLdConfigYamlCfg);
    CCURASSERT(pLdSysYamlCfg);

    do
    {
#if TRANSC_TCSIM
        /* Init sim component sys logging */
        sprintf(_strBuff,"%s%s",
                TRANSC_LOGCLASS_COMP_SIMPRC,"main");
        _result = evLogOpenLogicalLog(
                pZLogCatCompTbl->tZLogCat,
                pZLogCatCompTbl->nZLogCat,
                &(pCntx->tLogDescSys),
                evLogLvlClassCompSys,
                _strBuff);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlError,
                "Error, logging initialization %s",_strBuff);
            break;
        }
        /* Init sim Class services logging */
        sprintf(_strBuff,"%s%s",
                TRANSC_LOGCLASS_SVC_SIMPRC,"main");
        _result = evLogOpenLogicalLog(
                pZLogCatSvcTbl->tZLogCat,
                pZLogCatSvcTbl->nZLogCat,
                &(pCntx->tLogDescSvc),
                evLogLvlClassServices,
                _strBuff);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlError,
               "Error, logging initialization %s",_strBuff);
            break;
        }
#endif
        _result = mSemCondVarSemCreate(
                &(pCntx->tCondVarSem),
                _strBuff,
                sizeof(_strBuff)-1);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlError,
                   "%s\n",
                   _strBuff);
            break;
        }
        pCntx->bCondVarSet = TRUE;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcInitSimSendThreadInitRes
 *
 * description: Init sim thread resources. This function is being called
 * from background thread context.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitSimSendThreadInitRes(
        tc_simsnd_thread_ctxt_t*     pCntx,
        U32                          cfgId,
        tc_bkgrnd_evlogtbl_t*        pZLogCatCompTbl,
        tc_bkgrnd_evlogtbl_t*        pZLogCatSvcTbl,
        tc_ldcfg_conf_t*             pLdConfigYamlCfg,
        tc_ldsyscfg_conf_t*          pLdSysYamlCfg)
{
    tresult_t                   _result;
    CHAR                        _strBuff[256];

    CCURASSERT(pCntx);
    CCURASSERT(pLdConfigYamlCfg);
    CCURASSERT(pLdSysYamlCfg);

    do
    {
#if TRANSC_TCSIM
        _result = EFAILURE;
        if(cfgId >= TRANSC_SIM_THD_MAX)
        {
            evLogTraceSys(evLogLvlError,
               "Error, logging initialization %s",_strBuff);
            break;
        }
        /* Set config id for simulation thread */
        pCntx->cfgId = cfgId;
        sprintf(_strBuff,"%sworker_%lu",
                TRANSC_LOGCLASS_COMP_SIMPRC,pCntx->cfgId);
        /* Init sim component sys logging */
        _result = evLogOpenLogicalLog(
                pZLogCatCompTbl->tZLogCat,
                pZLogCatCompTbl->nZLogCat,
                &(pCntx->tLogDescSys),
                evLogLvlClassCompSys,
                _strBuff);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlError,
               "Error, logging initialization %s",_strBuff);
            break;
        }
        sprintf(_strBuff,"%sworker_%lu",
                TRANSC_LOGCLASS_SVC_SIMPRC,pCntx->cfgId);
        /* Init sim Class services logging */
        _result = evLogOpenLogicalLog(
                pZLogCatSvcTbl->tZLogCat,
                pZLogCatSvcTbl->nZLogCat,
                &(pCntx->tLogDescSvc),
                evLogLvlClassServices,
                _strBuff);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlError,
               "Error, logging initialization %s",_strBuff);
            break;
        }
#endif
        _result = mSemCondVarSemCreate(
                &(pCntx->tCondVarSem),
                _strBuff,
                sizeof(_strBuff)-1);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlError,
                   "%s\n",
                   _strBuff);
            break;
        }
        pCntx->bCondVarSet = TRUE;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcInitHealthThreadCleanupRes
 *
 * description: Cleanup Health thread resources.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitHealthThreadCleanupRes(
        tc_health_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);
    if(pCntx->bCondVarSet)
        mSemCondVarSemDestroy(
            &(pCntx->tCondVarSem));
    return ESUCCESS;
}

/***************************************************************************
 * function: _tcInitMibThreadCleanupRes
 *
 * description: Cleanup Mib thread resources.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitMibThreadCleanupRes(
        tc_tcplanesnmp_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);
    if(pCntx->bCondVarSet)
        mSemCondVarSemDestroy(
            &(pCntx->tCondVarSem));
    return ESUCCESS;
}

/***************************************************************************
 * function: _tcInitBkGrndThreadCleanupRes
 *
 * description: Cleanup background thread resources.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitBkGrndThreadCleanupRes(
        tc_bkgrnd_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);
    return ESUCCESS;
}

/***************************************************************************
 * function: _tcInitPktGenThreadCleanupRes
 *
 * description: Cleanup thread resources. This function is being called only
 * once at cleanup.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcInitPktGenThreadCleanupRes(
        tc_pktgen_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);

#if PKTGEN_THD
    if(pCntx->bCondVarSet)
        mSemCondVarSemDestroy(
            &(pCntx->tCondVarSem));
#endif
}

/***************************************************************************
 * function: _tcInitHttpThreadInitRes
 *
 * description: Cleanup thread resources. This function is being called only
 * once at cleanup.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcInitHttpThreadCleanupRes(
        tc_httpprc_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);

    if(pCntx->bCondVarSet)
        mSemCondVarSemDestroy(
            &(pCntx->tCondVarSem));
}

/***************************************************************************
 * function: _tcInitPktPrcThreadCleanupRes
 *
 * description: Cleanup thread resources. This function is being called only
 * once at cleanup.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcInitPktPrcThreadCleanupRes(
    tc_pktprc_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);

    if(pCntx->bCondVarSet)
        mSemCondVarSemDestroy(
            &(pCntx->tCondVarSem));
}


/***************************************************************************
 * function: _tcInitSimThreadCleanupRes
 *
 * description: Cleanup and destroy resources
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcInitSimThreadCleanupRes(
    tc_sim_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);

    if(pCntx->bCondVarSet)
        mSemCondVarSemDestroy(
            &(pCntx->tCondVarSem));
}

/***************************************************************************
 * function: _tcInitSimSendThreadCleanupRes
 *
 * description: Cleanup and destroy resources
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcInitSimSendThreadCleanupRes(
    tc_simsnd_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);

    if(pCntx->bCondVarSet)
        mSemCondVarSemDestroy(
            &(pCntx->tCondVarSem));
}
/***************************************************************************
 * function: _tcInitExternalLibaryRes
 *
 * description: Init Any external library component.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitExternalLibaryRes(
        tc_gd_thread_ctxt_t*     pCntx)
{
    CCURASSERT(pCntx);
    /* Init glib for each thread for version < 2.32 */
    if( ! g_thread_supported() )
    {
        g_thread_init( NULL );
        evLogTraceSys(evLogLvlDebug,
           "glib thread initialized!");
    }
    return ESUCCESS;
}

/***************************************************************************
 * function: _tcInitExternalLibraryCleanupRes
 *
 * description: Cleanup Any xternal library component.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcInitExternalLibraryCleanupRes(
        tc_gd_thread_ctxt_t*     pCntx)
{}

/***************************************************************************
 * function: _tcInitSharedRes
 *
 * description: Initialize shared resources
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcInitSharedRes(
        tc_gd_thread_ctxt_t*     pCntx)
{
    tresult_t                   _result;
    U32                         _i;
    tc_shared_cfgmsg_t*        _ptConfigYaml;
    tc_shared_syscfgmsg_t*     _ptSysYaml;
    pthread_mutex_t*            _pCommonMutex;

    CCURASSERT(pCntx);

    do
    {
        _result = ESUCCESS;
        _ptConfigYaml   = tcShDGetCfgYamlDesc();
        if(_ptConfigYaml)
        {
            _result = pthread_mutex_init ( &(_ptConfigYaml->tCfgMutex), NULL);
            if(ESUCCESS != _result)
            {
                evLogTraceSys(evLogLvlError,
                   "Error, logging initialization, unable to init config.yaml mutex");
                break;
            }
        }
        _ptSysYaml      = tcShDGetSysYamlDesc();
        if(_ptSysYaml)
        {
            _result = pthread_mutex_init ( &(_ptSysYaml->tCfgMutex), NULL);
            if(ESUCCESS != _result)
            {
                evLogTraceSys(evLogLvlError,
                   "Error, logging initialization, unable to sys.yaml mutex");
                break;
            }
        }
        /* One time init of number of sim threads */
        pCntx->tSimThd.nSimSendWThreadsNum       = pCntx->nSimSendWThreadsNum;
        pCntx->tMibThd.nSimSendWThreadsNum       = pCntx->nSimSendWThreadsNum;
        pCntx->tBkGnThd.nQEvLogTbl                  = pCntx->nSimSendWThreadsNum+tcBkgrndCompTypeMax;
        if(pCntx->tBkGnThd.nQEvLogTbl <= pCntx->nSimSendWThreadsNum ||
           pCntx->tBkGnThd.nQEvLogTbl >= TRANSC_BKGRNDLOG_QUEUE_MAX)
        {
            _result = EFAILURE;
            evLogTraceSys(evLogLvlError,
               "Error, logging initialization, unable to malloc bkgrnd event log table");
            break;
        }
        /* Initialize shared thread common mutex condition. Yep, it's redundant we
         * have one for config but it doesn't hurt. */
        _pCommonMutex = tcShDGetCommonMutex();
        _result = pthread_mutex_init ( _pCommonMutex, NULL);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlError,
               "Error, logging initialization, unable to common mutex");
            break;
        }
        /* init mutex */
        for(_i=0;_i<tcTRCompTypeMax;_i++)
        {
            _result = pthread_mutex_init ( tcShDGetCompMutex(_i), NULL);
            if(ESUCCESS != _result)
            {
                evLogTraceSys(evLogLvlError,
                   "Error, mutex initialization, "
                   "unable to init component mutex");
                break;
            }

        }
        /* Right now Http Proc and Pkt gen are combined
         * Together as one thread.*/
        pCntx->tHttpProcPktGen.pHttpPrcThd =
                &(pCntx->tHttpPrcThd);
        pCntx->tHttpProcPktGen.pPktGenThd  =
                &(pCntx->tPktGenThd);
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcInitCleanupSharedRes
 *
 * description: Cleanup thread shared resources
 ***************************************************************************/
CCUR_PRIVATE(void)
    _tcInitCleanupSharedRes(
    tc_gd_thread_ctxt_t*     pCntx)
{
    U16                         _i;
    tc_shared_cfgmsg_t*        _ptConfigYaml;
    tc_shared_syscfgmsg_t*     _ptSysYaml;
    CCURASSERT(pCntx);

    for(_i=0;_i<pCntx->tBkGnThd.nQEvLogTbl;_i++)
    {
        lkfqDestQueue(&(pCntx->tQLogTbl[_i].tLkfq));
        lkfqDestQueue(&(pCntx->tQMibTbl[_i]));
    }
    lkfqDestQueue(&(pCntx->tQPPktProcToHttpProc));
#if TRANSC_TCSIM
    if(pCntx->tQHttpProcToSim.pMp)
    {
        lkfqDestQueue(&(pCntx->tQHttpProcToSim));
    }
    for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
    {
        if(pCntx->tQSimToSimSendWTbl[_i].pMp)
        {
            lkfqDestQueue(&(pCntx->tQSimToSimSendWTbl[_i]));
        }
    }
    for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
    {
        if(pCntx->tQSimSendWToSimTbl[_i].pMp)
        {
            lkfqDestQueue(&(pCntx->tQSimSendWToSimTbl[_i]));
        }
    }
#endif /* TRANSC_TCSIM */
    for(_i=0;_i<tcTRCompTypeMax;_i++)
        pthread_mutex_destroy(tcShDGetCompMutex(_i));
    pthread_mutex_destroy(tcShDGetCommonMutex());
    _ptConfigYaml   = tcShDGetCfgYamlDesc();
    pthread_mutex_destroy(&(_ptConfigYaml->tCfgMutex));
    _ptSysYaml      = tcShDGetSysYamlDesc();
    pthread_mutex_destroy(&(_ptSysYaml->tCfgMutex));

}

/**************** PROTECTED Functions **********************/
CCUR_PROTECTED(tc_gd_thread_ctxt_t*)
tcInitGetGlobalThdContext(){return(&g_ThdCntx);}
/***************************************************************************
 * function: tcInitUsageMsgPrintBanner
 *
 * description: Transparent routing usage banner
 ***************************************************************************/
CCUR_PROTECTED(void)
tcInitUsageMsgPrintBanner()
{
    CHAR    _strBuff[64];
    tcPrintSysLog(LOG_NOTICE,"\n");
    tcPrintSysLog(LOG_NOTICE,"Transparent Routing v.%s-%s\n",
                  TRANSC_VER,TRANSC_REL);
#ifdef TRANSC_BUILDCFG_LIBPCAP
    tcPrintSysLog(LOG_NOTICE,"Supporting libpcap v.1.4.0");
#else
    tcPrintSysLog(LOG_NOTICE,"%s\n",tcUtilsGetPfringVer
                    (_strBuff,RING_VERSION_NUM));
#endif /* TRANSC_BUILDCFG_LIBPCAP */
    tcPrintSysLog(LOG_NOTICE,"czmq ver.%d\n",CZMQ_VERSION);
    tcPrintSysLog(LOG_NOTICE,"zeromq ver.%d\n",ZMQ_VERSION);
#if defined(__DATE__) && defined(__TIME__) && defined(__VERSION__)
    tcPrintSysLog(LOG_NOTICE,"Built on %s %s with ", __DATE__, __TIME__);
    if (strlen(__VERSION__) > 45) {tcPrintSysLog(LOG_NOTICE,"\n"); }
    tcPrintSysLog(LOG_NOTICE,"GCC %s.\n", __VERSION__ );
#endif
    tcPrintSysLog(LOG_NOTICE,"Copyright(c) 2015 Concurrent Computer Corporation\n");
}

/***************************************************************************
 * function: tcInitReadFromConsole
 *
 * description: Read input from console
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcInitReadFromConsole(
        tc_ldcfg_t* pCfg,
        S32 argc, CHAR*  argv[])
{
    S32                 _i;
    tresult_t           _result;
    BOOL                _gdb = FALSE;

    do
    {
        /* Open /var/log/messages for only this error */
         tcOpenSysLog("messages", LOG_PID, LOG_USER);
        /* setup main thread logging */
        if(1 == argc)
        {
            _result = EFAILURE;
            break;
        }
        else
            _result = ESUCCESS;
        for( _i = 1; _i < argc; ++_i )
        {
            if( !strcmp( argv[_i], "-i" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgMonIntf,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgMonIntf));
            }
            else if( !strcmp( argv[_i], "-o" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgOutIntf,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgOutIntf));
            }
            else if( !strcmp( argv[_i], "-r" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgPcapFilterRules,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgPcapFilterRules));
            }
            else if( !strcmp( argv[_i], "-t" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgRedirTarget,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgRedirTarget));
            }
            else if( !strcmp( argv[_i], "-c" ) )
            {
                ccur_strlcpy(pCfg->strCmdArgConfigYamlLoc,
                        argv[++_i],sizeof(pCfg->strCmdArgConfigYamlLoc));
            }
            else if( !strcmp( argv[_i], "-lc" ) )
            {
                ccur_strlcpy(pCfg->strCmdArgLogConfLoc,
                        argv[++_i],sizeof(pCfg->strCmdArgLogConfLoc));
            }
            else if( !strcmp( argv[_i], "-sc" ) )
            {
                ccur_strlcpy(pCfg->strCmdArgSysYamlLoc,
                        argv[++_i],sizeof(pCfg->strCmdArgSysYamlLoc));
            }
            else if( !strcmp( argv[_i], "-smac" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgOutIntfSrcMacAddr,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgOutIntfSrcMacAddr));
            }
            else if( !strcmp( argv[_i], "-dmac" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgOutIntfDestMacAddr,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgOutIntfDestMacAddr));
            }
            else if( !strcmp( argv[_i], "-m" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgModeOfOperation,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgModeOfOperation));
            }
            else if( !strcmp( argv[_i], "-b" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgIpBlackList,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgIpBlackList));
            }
            else if( !strcmp( argv[_i], "-st" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgSimBwThreadsNum,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgSimBwThreadsNum));
            }
            else if( !strcmp( argv[_i], "-sm" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgSimBwSimMode,
                        "true",sizeof(pCfg->tConfigYamlLdCfg.strCmdArgSimBwSimMode));
            }
            else if( !strcmp( argv[_i], "-so" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgSimBwOutIntf,
                        argv[++_i],sizeof(pCfg->tConfigYamlLdCfg.strCmdArgSimBwOutIntf));
            }
            else if( !strcmp( argv[_i], "-aml" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgOutIntfMplsLabel,
                        "true",sizeof(pCfg->tConfigYamlLdCfg.strCmdArgOutIntfMplsLabel));
            }
            else if( !strcmp( argv[_i], "-icr" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgIgnoreCORSReq,
                        "true",sizeof(pCfg->tConfigYamlLdCfg.strCmdArgIgnoreCORSReq));
            }
            else if( !strcmp( argv[_i], "-bt" ) )
            {
                pCfg->bBlockTraffic = TRUE;
            }
            else if( !strcmp( argv[_i], "-ppr" ) )
            {
                ccur_strlcpy(pCfg->tConfigYamlLdCfg.strCmdArgProcessProxyReq,
                        "true",sizeof(pCfg->tConfigYamlLdCfg.strCmdArgProcessProxyReq));
            }
            else if( !strcmp( argv[_i], "-lq" ) )
            {
                pCfg->bLockQ = TRUE;
            }
            else if( !strcmp( argv[_i], "-hc" ) )
            {
				pCfg->healthCheck = atoi(argv[++_i]);
            }
            else if( !strcmp( argv[_i], "-d" ) )
            {
                pCfg->bDaemonize = TRUE;
            }
            else if( !strcmp( argv[_i], "-gdb" ) )
            {
                _gdb = TRUE;
                while(_gdb)
                {
                    tcPrintSysLog(LOG_ERR,
                            "Error, set gdb=0 to attach debug\n");
                    sleep(5);
                }
            }
            else
            {
                tcPrintSysLog(LOG_ERR,
                        "Error, Unknown argument: %s\n", argv[_i] );
                _result = EFAILURE;
                break;
            }
        }
        if(ESUCCESS != _result)
            break;
        if('\0' == pCfg->strCmdArgConfigYamlLoc[0])
        {
            _result = EINVAL;
            tcPrintSysLog(LOG_ERR,
                    "Error, invalid -c "
                    "main config.yaml must be specified\n");
        }
        if('\0' == pCfg->strCmdArgLogConfLoc[0])
        {
            _result = EINVAL;
            tcPrintSysLog(LOG_ERR,
                    "Error, invalid -lc "
                    "log config must be specified\n");
        }
        /* Skips reading config values that are already set here... */
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgOutIntf[0])
        {
           pCfg->tConfigYamlLdCfg.bstrCmdArgOutIntfSkip      = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgMonIntf[0])
        {
            pCfg->tConfigYamlLdCfg.bStrCmdArgMonIntfSkip         = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgPcapFilterRules[0])
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgPcapFilterRulesSkip         = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgRedirTarget[0])
        {
            pCfg->tConfigYamlLdCfg.bStrCmdArgRedirTargetSkip     = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgModeOfOperation[0])
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgModeOfOperationSkip     = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgIpBlackList[0])
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgIpBlackListSkip     = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgSimBwSimMode[0])
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgSimBwSimModeSkip     = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgSimBwThreadsNum[0])
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgSimBwThreadsNumSkip     = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgSimBwOutIntf[0])
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgSimBwOutIntfSkip     = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgOutIntfSrcMacAddr[0])
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgOutIntfSrcMacAddrSkip     = TRUE;
        }
        if('\0' != pCfg->tConfigYamlLdCfg.strCmdArgOutIntfDestMacAddr[0])
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgOutIntfDestMacAddrSkip     = TRUE;
        }
        if(!strcmp(pCfg->tConfigYamlLdCfg.strCmdArgSimBwSimMode,"true"))
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgSimBwSimModeSkip     = TRUE;
        }
        if(!strcmp(pCfg->tConfigYamlLdCfg.strCmdArgOutIntfMplsLabel,"true"))
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgOutIntfMplsLabelSkip     = TRUE;
        }
        if(!strcmp(pCfg->tConfigYamlLdCfg.strCmdArgProcessProxyReq,"true"))
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgProcessProxyReqSkip     = TRUE;
        }
        if(!strcmp(pCfg->tConfigYamlLdCfg.strCmdArgIgnoreCORSReq,"true"))
        {
            pCfg->tConfigYamlLdCfg.bstrCmdArgIgnoreCORSReqSkip     = TRUE;
        }
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: tcInitLoadConfigFiles
 *
 * description: Initiaze the config files
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcInitLoadConfigFiles(tc_gd_thread_ctxt_t* pCntx)
{
    tresult_t                   _result;
    BOOL                        _bBwSimMode;
    U32                         _nSimSendWThreadsNum = 0;
    CHAR*                       _strS;

    do
    {
        _result = EFAILURE;
        _result = tcLoadUnmarshallSysYaml(
                NULL,NULL,
                pCntx->tMibThd.tConfig.strCmdArgSysYamlLoc,
                &(pCntx->tMibThd.tConfig.tSysYamlLdCfg),
                &(pCntx->tMibThd.tConfig.tSysYamlParser));
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                    evLogLvlError,
                    (char*) "Unable to load Config.yaml");
            break;
        }
        _result = tcLoadUnmarshallConfigYaml(
                NULL,NULL,
                pCntx->tMibThd.tConfig.strCmdArgConfigYamlLoc,
                &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                &(pCntx->tMibThd.tConfig.tConfigYamlParser));
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                    evLogLvlError,
                    (char*) "Unable to load sys.yaml");
            break;
        }
        /* Below logic is to check and
         * extract number of simSend Worker threads */
        if(0 == ccur_strcasecmp(
                pCntx->tMibThd.tConfig.tConfigYamlLdCfg.strCmdArgSimBwSimMode,"true"))
            _bBwSimMode = TRUE;
        else
            _bBwSimMode = FALSE;
        _nSimSendWThreadsNum = TRANSC_SIMSENDW_THD_DFLT_NUM;
        if('\0' != pCntx->tMibThd.tConfig.tConfigYamlLdCfg.strCmdArgSimBwThreadsNum[0])
        {
            /* validate arguments make sure it is a numberical value */
            _strS = pCntx->tMibThd.tConfig.tConfigYamlLdCfg.strCmdArgSimBwThreadsNum;
            while('\0' != *_strS)
            {
                if(!isdigit(*_strS))
                {
                    evLogTraceSys(evLogLvlError,
                        "Error, invalid -st value %s is not a numerical value",
                        pCntx->tMibThd.tConfig.tConfigYamlLdCfg.strCmdArgSimBwThreadsNum);
                    _result = EINVAL;
                    break;
                }
                _strS++;
            }
            if(ESUCCESS != _result)
                break;
            _nSimSendWThreadsNum =
                    strtol(pCntx->tMibThd.tConfig.tConfigYamlLdCfg.strCmdArgSimBwThreadsNum,NULL,10);
        }
        if(_nSimSendWThreadsNum == 0)
        {
            _nSimSendWThreadsNum = 1;
            evLogTraceSys(evLogLvlInfo,"Sim Thread is set to 0, setting it to %lu\n",
                   _nSimSendWThreadsNum);
        }
        if(_nSimSendWThreadsNum >= TRANSC_SIM_THD_MAX)
        {
            _nSimSendWThreadsNum = TRANSC_SIM_THD_MAX-1;
            evLogTraceSys(evLogLvlInfo,"Sim Thread is set > %d, setting it to %lu\n",
                    TRANSC_SIM_THD_MAX,_nSimSendWThreadsNum);
        }
        if(FALSE == _bBwSimMode)
        {
            _nSimSendWThreadsNum = 1;
            evLogTraceSys(evLogLvlInfo,
                    "Sim Thread is disabled, "
                   "running default 1 sim and sim/send thread");
        }
        if(ESUCCESS != _result)
        {
            evLogTraceSys(
                   evLogLvlFatal,
                   "Unable to init sim proc data\n");
            break;
        }
        /* This value is not reloadable */
        pCntx->nSimSendWThreadsNum = _nSimSendWThreadsNum;
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: tcInitDaemonize
 *
 * description: Daemonization by Forking a process and
 * killing the parent process.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcInitDaemonize(BOOL bDaemonize)
{
    tresult_t   _result;

    do
    {
        _result = ESUCCESS;
        if (bDaemonize)
        {
            CHAR  _errStr[128];
            /* Do not allow n-instances of process to run.
             * Check if there is already process running in the
             * system.
             */
            _result = tcUtilHostCkProcessActive(
                    TRANSC_PROCNAME,
                    TRANSC_PIDFILE,
                    _errStr);
            if(ESUCCESS != _result)
            {
                evLogTraceSys(
                      evLogLvlError,
                      _errStr);
                break;
            }
            _result = tcUtilsDaemonize();
            if(ESUCCESS != _result)
                break;
            _result =
                    tcUtilHostSaveActiveProcess(
                    TRANSC_PIDFILE,
                    _errStr);
            if(ESUCCESS != _result)
            {
                evLogTraceSys(
                      evLogLvlError,
                      _errStr);
                break;
            }
        }
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: tcInitIsSwitchDaemonMode
 *
 * description: Switch to daemon mode by killing parent process after forking.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcInitIsSwitchDaemonMode(BOOL bDaemonize)
{
    if (bDaemonize)
    {
        /*
         * Signal the parent process that it is now OK to exit.
         */
        kill(getppid(), SIGTERM);
    }
    else
    {
        /*
         * Display some additional info to the console if not running
         *   in daemon mode.
         */
        tcInitUsageMsgPrintBanner();
        tcPrintSysLog(LOG_INFO,
            "--->%s started. Send SIGTERM, SIGSTOP, or SIGQUIT to "
            "stop process.<----\n", TRANSC_PROCNAME);
    }
}

/***************************************************************************
 * function: tcInitIsInitSuccess
 *
 * description: Check status
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcInitIsInitSuccess(
        tc_gd_thread_ctxt_t*    pCntx,
        tc_tr_comptype_e       eCompType)
{
    U32     _i;
    BOOL    _bInitSuccess;
    CHAR    _strBuff1[256];

    _bInitSuccess = FALSE;
    /* Retry 10 times or 10 secs */
    for(_i=0;_i<10;_i++)
    {
        _bInitSuccess = FALSE;
        if(eCompType < tcTRCompTypeMax)
        {
            if(tcShProtectedDMsgIsCompInitRdy(eCompType))
            {
                _bInitSuccess = TRUE;
                snprintf(_strBuff1,sizeof(_strBuff1),"Component %d initialized",eCompType);
                evLogTraceSys(evLogLvlInfo,_strBuff1);
                break;
            }
            else
            {
                snprintf(_strBuff1,sizeof(_strBuff1),
                        "Component %d unitialized, wait...",eCompType);
                evLogTraceSys(evLogLvlInfo,_strBuff1);
                sleep(1);
            }
        }
        else
        {
            evLogTraceSys(evLogLvlInfo,"Component %d init error",eCompType);
            break;
        }
    }
    if(FALSE == _bInitSuccess)
        tcShProtectedDSetAppExitSts(TRUE);

    return _bInitSuccess;
}

/***************************************************************************
 * function: tcInitRunThreads
 *
 * description: Launch all threads. Keep in mind that zero MQ context
 * created by snmp thread will launch it's own thread so there will be
 * extra thread in the system.
 * Note: Pkt gen and http proc components are currently under one thread.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcInitRunThreads(
        tc_gd_thread_ctxt_t*             pCntx)
{
    BOOL                _bInit;
    tresult_t           _result;
    tresult_t           _sts;
    U16                 _i;

    CCURASSERT(pCntx);

    do
    {
        _result = EFAILURE;
        /* This is where we spawn all threads
         * with context being passed into each thread.*/
        /*** start tcplane_snmp thread ***/
        if(pthread_create(&(pCntx->tMibThd.tcplane_snmp_thread),
                NULL, tcplane_snmp, (void*)(&pCntx->tMibThd)))
        {
            evLogTraceSys(
                   evLogLvlFatal,
                   "unable to create snmp mib thread\n");
            break;
        }

        _sts = mthreadCreate(
                      &(pCntx->tHealthThd.tThd),
                      tcHealthThreadEntry,
                      TRANSC_HEALTH_THD_STACK,
                      mThreadRtPriNormal,
                      SCHED_RR,
                      &(pCntx->tHealthThd));
        if(ESUCCESS != _sts)
        {
            evLogTraceSys(
                   evLogLvlFatal,
                   "unable to create health thread\n");
            break;
        }
        _sts = mthreadCreate(
                      &(pCntx->tPktPrcThd.tThd),
                      tcPktPrcThreadEntry,
                      TRANSC_PKTPRC_THD_STACK,
                      mThreadRtPriHigh,
                      SCHED_RR,
                      &(pCntx->tPktPrcThd));
        if(ESUCCESS != _sts)
        {
            evLogTraceSys(
                   evLogLvlFatal,
                   "unable to create pkt prc thread\n");
            break;
        }
        _sts = mthreadCreate(
                      &(pCntx->tHttpPrcThd.tThd),
                      tcHttpProcThreadEntry,
                      TRANSC_HTTPPRC_THD_STACK,
                      mThreadRtPriHigh,
                      SCHED_RR,
                      &(pCntx->tHttpProcPktGen));
        if(ESUCCESS != _sts)
        {
            evLogTraceSys(
                   evLogLvlFatal,
                   "unable to create http prc thread\n");
            break;
        }
#if PKTGEN_THD
        _sts = mthreadCreate(
                      &(pCntx->tPktGenThd.tThd),
                      tcPktGenThreadEntry,
                      mThreadRtPriHigh,
                      SCHED_RR,
                      &(pCntx->tPktGenThd));
        if(ESUCCESS != _sts)
        {
            evLogTraceSys(
                   evLogLvlFatal,
                   "unable to create Pkt Gen thread\n");
            break;
        }
#endif
#if TRANSC_TCSIM
        _sts = mthreadCreate(
                      &(pCntx->tSimThd.tThd),
                      tcSimProcThreadEntry,
                      TRANSC_SIM_THD_STACK,
                      mThreadRtPriHigh,
                      SCHED_RR,
                      &(pCntx->tSimThd));
        if(ESUCCESS != _sts)
        {
            evLogTraceSys(
                   evLogLvlFatal,
                   "unable to create Sim thread\n");
            break;
        }
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
        {
            _sts = mthreadCreate(
                          &(pCntx->tSimSndThdTbl[_i].tThd),
                          tcSimSendProcThreadEntry,
                          TRANSC_SIMSNDW_THD_STACK,
                          mThreadRtPriHigh,
                          SCHED_RR,
                          &(pCntx->tSimSndThdTbl[_i]));
            if(ESUCCESS != _sts)
            {
                evLogTraceSys(
                       evLogLvlFatal,
                       "unable to create Sim-snd thread %d\n",_i);
                break;
            }
        }
        pCntx->bIsThdStart = TRUE;
#endif /* TRANSC_TCSIM */
        evLogTraceSys(evLogLvlInfo,
                "%d Simulation thread created",pCntx->nSimSendWThreadsNum);
        /* Launch all threads once all initializations completed */
        /* Do not change the ordering how the threads are
         * set to initialize. The order is defined within
         * tc_tr_comptype_e
         */
        for(_i=0;_i<tcTRCompTypeMax;_i++)
        {
            _bInit = tcInitIsInitSuccess(pCntx,_i);
            if(FALSE == _bInit)
            {
                _sts = EFAILURE;
                break;
            }
        }
        if(ESUCCESS != _sts)
            break;
        /* At this point, logging and snmp threads should already
         * run...
         */
        /* Do not change the ordering how the threads are
         * set to run. Here is the order:
         * - health (background) thread
         * - snmp_plane/mib (background) thread
         * - sim-send worker n-threads
         * - sim thread
         * - Pkt gen thread
         * - Http proc thread
         * - pkt proc thread
         * - logging (background) thread
         */
        /* Health thread */
        mSemCondVarSemPost(
                &(pCntx->tHealthThd.tCondVarSem),NULL);
#if TRANSC_TCSIM
        /* Run sim-send worker threads */
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
            mSemCondVarSemPost(
                    &(pCntx->tSimSndThdTbl[_i].tCondVarSem),NULL);
        /* Run sim thread */
        mSemCondVarSemPost(
                &(pCntx->tSimThd.tCondVarSem),NULL);
#endif /* TRANSC_TCSIM */
#if PKTGEN_THD
        /* Launch pkt gen thread */
        mSemCondVarSemPost(
                &(pCntx->tPktGenThd.tCondVarSem),NULL);
#endif
        /* Launch http processing threads */
        mSemCondVarSemPost(
                &(pCntx->tHttpPrcThd.tCondVarSem),NULL);
        /* Snmp Plane/Mib thread */
        mSemCondVarSemPost(
                &(pCntx->tMibThd.tCondVarSem),NULL);
        /* Launch pkt processing threads */
        mSemCondVarSemPost(
                &(pCntx->tPktPrcThd.tCondVarSem),NULL);
        /* Demote this thread as background thread */
        _sts = mthreadSetPriority(NULL,
                SCHED_OTHER,mThreadRtPriBackGrnd);
        if(ESUCCESS != _sts)
            break;
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: tcInitRes
 *
 * description: Initialize all component resources. Please keep in mind that
 * there are two background threads, one for dumping logs from other
 * component and the other one is snmp mib thread.
 * Note: Pkt gen and http proc components are currently under one thread.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcInitRes(
        tc_gd_thread_ctxt_t*             pCntx)
{
    tresult_t       _result;
    U16             _i;

    CCURASSERT(pCntx);

    do
    {
        _result = ESUCCESS;
        /* DO not change the order of 1-3 */
        /* 1. Init External Library components */
        _result = _tcInitExternalLibaryRes(pCntx);
        if(ESUCCESS != _result)
            break;

        /* 2. Shared Resource Init */
        _result = _tcInitSharedRes(pCntx);
        if(ESUCCESS != _result)
            break;

        /* 3. BkGrnd Thread Resource Scope Init */
        _result = _tcInitBkGrndThreadInitRes(
                &(pCntx->tBkGnThd),
                &(pCntx->tBkGnThd.tZLogCatComp),
                &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                &(pCntx->tMibThd.tConfig.tSysYamlLdCfg));
        if(ESUCCESS != _result)
            break;

        /* 4. Health Thread Resource Scope Init */
        _result = _tcInitHealthThreadRes(
                &(pCntx->tHealthThd),
                &(pCntx->tBkGnThd.tZLogCatComp),
                &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                &(pCntx->tMibThd.tConfig.tSysYamlLdCfg),
                pCntx->tMibThd.tConfig.healthCheck);
        if(ESUCCESS != _result)
            break;

        /* 5. Mib Thread Resource Scope Init */
        _result = _tcInitMibThreadInitRes(
                &(pCntx->tMibThd),
                &(pCntx->tBkGnThd.tZLogCatComp),
                &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                &(pCntx->tMibThd.tConfig.tSysYamlLdCfg));
        if(ESUCCESS != _result)
            break;

#if TRANSC_TCSIM
        /* 6. sim/send worker Threads Resource Scope Init */
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
        {
            _result = _tcInitSimSendThreadInitRes(
                    &(pCntx->tSimSndThdTbl[_i]),
                    _i,
                    &(pCntx->tBkGnThd.tZLogCatComp),
                    &(pCntx->tBkGnThd.tZLogCatSvc),
                    &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                    &(pCntx->tMibThd.tConfig.tSysYamlLdCfg));
            if(ESUCCESS != _result)
                break;
        }
        if(ESUCCESS != _result)
            break;

        /* 7. Simulation Thread Resource Scope Init */
        _result = _tcInitSimThreadInitRes(
                &(pCntx->tSimThd),
                &(pCntx->tBkGnThd.tZLogCatComp),
                &(pCntx->tBkGnThd.tZLogCatSvc),
                &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                &(pCntx->tMibThd.tConfig.tSysYamlLdCfg));
        if(ESUCCESS != _result)
            break;

#endif /* TRANSC_TCSIM */
        /* 8. Pkt Proc Thread Resource Scope Init */
        _result = _tcInitPktPrcThreadInitRes(
                &(pCntx->tPktPrcThd),
                &(pCntx->tBkGnThd.tZLogCatComp),
                &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                &(pCntx->tMibThd.tConfig.tSysYamlLdCfg));
        if(ESUCCESS != _result)
            break;

        /* 9. Http Proc Thread Resource Scope Init */
        _result = _tcInitHttpThreadInitRes(
                &(pCntx->tHttpPrcThd),
                &(pCntx->tBkGnThd.tZLogCatComp),
                &(pCntx->tBkGnThd.tZLogCatSvc),
                &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                &(pCntx->tMibThd.tConfig.tSysYamlLdCfg));
        if(ESUCCESS != _result)
            break;

        /* 10. Pkt Gen Thread Resource Scope Init */
        _result = _tcInitPktGenThreadInitRes(
                &(pCntx->tPktGenThd),
                &(pCntx->tBkGnThd.tZLogCatComp),
                &(pCntx->tBkGnThd.tZLogCatSvc),
                &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                &(pCntx->tMibThd.tConfig.tSysYamlLdCfg),
                pCntx->tMibThd.tConfig.bBlockTraffic);
        if(ESUCCESS != _result)
            break;

        _result = _tcInitSharedQ(pCntx);
        if(ESUCCESS != _result)
        {
            evLogTraceSys(evLogLvlError,
               "Error, queue initialization");
            break;
        }
        /* Set threads to load config.yaml */
        tcShProtectedDPushConfigYaml(
                &(pCntx->tMibThd.tConfig.tConfigYamlLdCfg),
                pCntx->nSimSendWThreadsNum);
        /* Set threads to load sys.yaml */
        tcShProtectedDPushSysYaml(
                &(pCntx->tMibThd.tConfig.tSysYamlLdCfg),
                pCntx->nSimSendWThreadsNum);

    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: tcInitEventLog
 *
 * description: Initialize main event logging, which belongs to background
 * thread.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcInitEventLog(
        tc_gd_thread_ctxt_t*    pCntx)
{
    tresult_t                   _result;

    CCURASSERT(pCntx);

    _result = tcBkgrndInitEventLog(
            &(pCntx->tBkGnThd),
            pCntx->tMibThd.tConfig.strCmdArgLogConfLoc);

    if(ESUCCESS !=  _result)
    {
        tcPrintSysLog(LOG_ERR,
                "Error, Load log config from: %s\n",
                pCntx->tMibThd.tConfig.strCmdArgLogConfLoc);
    }

    return _result;
}

/***************************************************************************
 * function: tcInitCleanupRes
 *
 * description: Cleanup all system resources after the user shuts down
 * the program. We will wait until all the threads exit then properly
 * clean up all the threads resources.
 ***************************************************************************/
CCUR_PROTECTED(void)
        tcInitCleanupRes(
        mthread_result_t*        pExitCode,
        tc_gd_thread_ctxt_t*     pCntx)
{
    tresult_t           _PollSts;
    U32                 _i;

    CCURASSERT(pCntx);

    if(pCntx->bIsThdStart)
    {
        pCntx->tMibThd.bExit = TRUE;
        pCntx->tHealthThd.bExit = TRUE;
        pCntx->tSimThd.bExit = TRUE;
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
            pCntx->tSimSndThdTbl[_i].bExit = TRUE;
        pCntx->tPktPrcThd.bExit         = TRUE;
        pCntx->tBkGnThd.bExit           = TRUE;
        pCntx->tHttpPrcThd.bExit        = TRUE;
#if PKTGEN_THD
        pCntx->tPktGenThd.bExit         = TRUE;
#endif

        /* joins and wait till all thread(s) exit here */
        pthread_join(
                pCntx->tMibThd.tcplane_snmp_thread,NULL);
#if TRANSC_TCSIM
        mthreadWaitExit(
                &(pCntx->tSimThd.tThd), pExitCode);
        for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
            mthreadWaitExit(
                &(pCntx->tSimSndThdTbl[_i].tThd), pExitCode);
#endif /* TRANSC_TCSIM */
#if PKTGEN_THD
        if(pCntx->tPktGenThd.bIsThdStart)
            mthreadWaitExit(
                    &(pCntx->tPktGenThd.tThd), pExitCode);
#endif
        mthreadWaitExit(
                &(pCntx->tHttpPrcThd.tThd), pExitCode);
        mthreadWaitExit(
                &(pCntx->tPktPrcThd.tThd), pExitCode);
        mthreadWaitExit(
                &(pCntx->tHealthThd.tThd), pExitCode);
        /* Flush the logs */
        do
        {
            for(_i=0;_i<10;_i++)
                _PollSts = tcBkgrndEvLogReadFromPktProcQ(
                        &(pCntx->tBkGnThd));
        }while(ENODATA !=_PollSts);
    }
    /* Note: pcap_compile memory leak during cleanup.
     * https://github.com/mcr/libpcap/issues/26
     */
#if TRANSC_TCSIM
    for(_i=0;_i<pCntx->nSimSendWThreadsNum;_i++)
        _tcInitSimSendThreadCleanupRes(&(pCntx->tSimSndThdTbl[_i]));
    _tcInitSimThreadCleanupRes(&(pCntx->tSimThd));
#endif /* TRANSC_TCSIM */
    /* Clean up pkt proc resources */
    _tcInitPktPrcThreadCleanupRes(&(pCntx->tPktPrcThd));
    /* Clean up Http proc resources */
    _tcInitHttpThreadCleanupRes(&(pCntx->tHttpPrcThd));
    /* Clean up Pkt gen resources */
    _tcInitPktGenThreadCleanupRes(&(pCntx->tPktGenThd));
    /* Clean up Mib resources */
    _tcInitMibThreadCleanupRes(&(pCntx->tMibThd));
    /* Clean up Health resources */
    _tcInitHealthThreadCleanupRes(&(pCntx->tHealthThd));
    /* Clean up Background resources */
    _tcInitBkGrndThreadCleanupRes(&(pCntx->tBkGnThd));
    /* Clean up Shared resources */
    _tcInitCleanupSharedRes(pCntx);
    /* Clean up Media hawk resources */
    _tcInitExternalLibraryCleanupRes(pCntx);
    /* Close logging resources */
    evLogCloseLogicalLog();
}
