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

#include "pktgen.h"
#include "tcmapintf.h"

/**************** PROTECTED Functions **********************/

CCUR_PROTECTED(void)
tcPktGenInitLogOutCfgLoadStatus(
        tc_pktgen_thread_ctxt_t*      pCntx)
{
    U32                     _i;
    tc_outintf_intfd_t*     _pIntfd;
    tc_intf_config_t*       _pIntfdCfg;
    tc_outintf_out_t*       _pOutIntf;
    CHAR                    _strBuff[128];

    _pIntfd       = &(pCntx->tIntfX);
    _pIntfdCfg    = &(pCntx->tIntfCfg);

    for(_i=0;_i<_pIntfd->nOutIntfTotal;_i++)
    {
        _pOutIntf = &(_pIntfd->tOutIntfTbl[_i]);
        if(_pOutIntf)
        {
            evLogTrace(pCntx->pQPktGenToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Egress stats:\n"
                    "intfName/IsMACvalid/TotalNum/OpenNum/Sts/IsUp\n"
                    "%s/%d/%s/%d/%d/%d/%d",
                    _pOutIntf->tIntf.strIntfName,
                    _pOutIntf->tIntf.bIntfRdy,
                    _pOutIntf->tIntf.strIntfVal,
                    _pIntfd->nOutIntfTotal-0,
                    _pIntfd->nOutIntfOpen-0,
                    _pIntfd->bOutIntfOpen,
                    _pIntfd->bOutIntfUp);
            if(_pOutIntf->bIsRouter)
            {
                sprintf(_strBuff,"SRC MAC Addr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x | "
                                 "DST MAC Addr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                                 _pOutIntf->pCmdArgSmacAddr[0],
                                 _pOutIntf->pCmdArgSmacAddr[1],
                                 _pOutIntf->pCmdArgSmacAddr[2],
                                 _pOutIntf->pCmdArgSmacAddr[3],
                                 _pOutIntf->pCmdArgSmacAddr[4],
                                 _pOutIntf->pCmdArgSmacAddr[5],
                                 _pOutIntf->pCmdArgDmacAddr[0],
                                 _pOutIntf->pCmdArgDmacAddr[1],
                                 _pOutIntf->pCmdArgDmacAddr[2],
                                 _pOutIntf->pCmdArgDmacAddr[3],
                                 _pOutIntf->pCmdArgDmacAddr[4],
                                 _pOutIntf->pCmdArgDmacAddr[5]);
                evLogTrace(pCntx->pQPktGenToBkgrnd,
                        evLogLvlWarn,
                        &(pCntx->tLogDescSys),
                       "%s is set to inject to: router (L3) switch | %s:%s",
                       _pOutIntf->tIntf.strIntfName,_pOutIntf->tIntf.strIntfVal,_strBuff);
            }
            else
            {
                sprintf(_strBuff,"SRC MAC Addr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x | "
                                 "DST MAC Addr: N/A (From Request)",
                                 _pOutIntf->pCmdArgSmacAddr[0],
                                 _pOutIntf->pCmdArgSmacAddr[1],
                                 _pOutIntf->pCmdArgSmacAddr[2],
                                 _pOutIntf->pCmdArgSmacAddr[3],
                                 _pOutIntf->pCmdArgSmacAddr[4],
                                 _pOutIntf->pCmdArgSmacAddr[5]);
                evLogTrace(pCntx->pQPktGenToBkgrnd,
                        evLogLvlWarn,
                        &(pCntx->tLogDescSys),
                       "%s is set to inject to: other (L2) switch | %s:%s",
                       _pOutIntf->tIntf.strIntfName,_pOutIntf->tIntf.strIntfVal,_strBuff);
            }
        }
    }
    if(_pIntfdCfg->bActiveMode)
    {
        evLogTrace(pCntx->pQPktGenToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
               "Mode Of Operation Mapped Entries:%lu/%lu",
               _pIntfd->nIntfMapTblTotal,TRANSC_INTERFACE_MAX);
        for(_i=0;_i<_pIntfd->nIntfMapTblTotal;_i++)
        {
            if(_pIntfd->tIntfMapTbl[_i].bIsModeActv &&
               _pIntfd->tIntfMapTbl[_i].pOutIntf)
            {
                evLogTrace(pCntx->pQPktGenToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                       "Egress Mode Of Operation: %s/%s->%s/ACTIVE",
                       _pIntfd->tMonIntfTbl[_i].strRedirAddr,
                       _pIntfd->tMonIntfTbl[_i].tLinkIntf.strIntfName,
                       _pIntfd->tIntfMapTbl[_i].pOutIntf->tIntf.strIntfName);
            }
        }
    }
    evLogTrace(pCntx->pQPktGenToBkgrnd,
            evLogLvlInfo,
           &(pCntx->tLogDescSys),
           "Redirection Rate Max (redirReq/sec):%lu",
           pCntx->nRedirReqRateCap);
    evLogTrace(pCntx->pQPktGenToBkgrnd,
            evLogLvlInfo,
           &(pCntx->tLogDescSys),
           "Redirection sample Rate (req/sec):%lu",
           pCntx->nReqSampleCnt);
    evLogTrace(pCntx->pQPktGenToBkgrnd,
            evLogLvlInfo,
           &(pCntx->tLogDescSys),
           "Outgoing Interface Add MPLS Tag (0-false/1-true):%d ",
           pCntx->bOutIntfInsertMplsTag);
}

/***************************************************************************
 * function: tcPktGenInitIntf
 *
 * description: Open Tx interface for injection.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktGenInitIntf(
        tc_pktgen_thread_ctxt_t*     pCntx,
        BOOL                         bTxLoad)
{
    tresult_t   _result;

    CCURASSERT(pCntx);

    _result = ESUCCESS;

    if(bTxLoad)
    {
        if(pCntx->tIntfX.bOutIntfOpen)
            tcPktIOTxOutIntfClose(pCntx);
        if(FALSE == pCntx->tIntfX.bOutIntfOpen)
            _result = tcPktIOTxOutIntfOpen(pCntx);
    }

    return _result;
}

/***************************************************************************
 * function: tcPktGenLogDownStatusAndRetry
 *
 * description: Dumps the reason why TCS is down and retry in the case
 * network interface is not up yet.
 * TCS can be down due to network interface is not up yet or
 * configuration failure.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktGenLogDownStatusAndRetry(
        tc_pktgen_thread_ctxt_t*     pCntx)
{
    BOOL _bWait = TRUE;
    /* Wait until both monitoring and
     * output interfaces are up and running
     * then continue.
     */
    if(FALSE == pCntx->bNonIntfCfgLoadSuccess)
    {
        evLogTrace(
              pCntx->pQPktGenToBkgrnd,
              evLogLvlError,
              &(pCntx->tLogDescSys),
              "TCS is down, signature service "
              "configuration loading failure, "
              "please check config file.");
    }
    else if(FALSE == pCntx->bIntfCfgLoadSuccess)
    {
        evLogTrace(
              pCntx->pQPktGenToBkgrnd,
              evLogLvlError,
              &(pCntx->tLogDescSys),
              "TCS is down, interface "
              "configuration loading failure, "
              "please check config file.");
    }
    else if(FALSE == pCntx->tIntfX.bOutIntfUp)
    {
        /* if Rings are not initialized then try to
         * initialize it, this case happens when
         * pfring trying to open interface not
         * up yet in case of fresh bootup with
         * multiple interfaces. */
        if(FALSE == pCntx->tIntfX.bOutIntfOpen)
            tcPktGenInitIntf(pCntx,TRUE);
        /* Check to make sure both links are
         * up and running */

        if(pCntx->tIntfX.bOutIntfOpen &&
           tcPktIOTxOutIntfIsLinkUp(pCntx))
            pCntx->tIntfX.bOutIntfUp = TRUE;
        else
            pCntx->tIntfX.bOutIntfUp = FALSE;
        /* Definition of transc is up :
           transc all system is ready to process.
           - monitoring up,
           - injection up,
           - all threads up,
           - configuration successfully loaded. */
        /* TODO: Add all threads up check */
        if(pCntx->bIntfCfgLoadSuccess &&
           pCntx->bNonIntfCfgLoadSuccess &&
           pCntx->tIntfX.bOutIntfUp)
        {
            pCntx->bTrSts = tcTrStsUp;
            _bWait = FALSE;
        }
    }
    else
    {
        evLogTrace(
              pCntx->pQPktGenToBkgrnd,
              evLogLvlError,
              &(pCntx->tLogDescSys),
              "TCS is down");
    }
    if(_bWait)
        sleep(5);
}

/***************************************************************************
 * function: tcPktProcConfigInitLoadableRes
 *
 * description: reload pkt proc data structure from config.yaml configuration
 * file.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktGenConfigInitLoadableRes(
        tc_pktgen_thread_ctxt_t*     pCntx,
        tc_ldcfg_conf_t*             pLdCfg)
{
    tresult_t                   _sts;
    BOOL                        _bRxLoad;
    BOOL                        _bTxLoad;
    BOOL                        _bMapLoad;
    BOOL                        _bRedirLoad;

    CCURASSERT(pCntx);
    CCURASSERT(pLdCfg);


    evLogTrace(
          pCntx->pQPktGenToBkgrnd,
          evLogLvlWarn,
          &(pCntx->tLogDescSys),
          "Warning, pktgen is down, reloading config!");

    do
    {
        pCntx->bTrSts = tcTrStsDown;
        pCntx->bNonIntfCfgLoadSuccess = FALSE;
        pCntx->bIntfCfgLoadSuccess = FALSE;
        if(!strcasecmp("true",
                pLdCfg->strCmdArgIgnoreCORSReq))
            pCntx->bRedirectCORSReq = FALSE;
        else
            pCntx->bRedirectCORSReq = TRUE;
        if('\0' == pLdCfg->strCmdArgOutRedirReqRateMax[0])
            pCntx->nRedirReqRateCap = TRANSC_REDIRRATE_MAX;
        else
            sscanf(pLdCfg->strCmdArgOutRedirReqRateMax,"%lu",
                   &(pCntx->nRedirReqRateCap));
        pCntx->nReqSampleCnt = pCntx->nRedirReqRateCap;
        if(0 == pCntx->nReqSampleCnt)
            pCntx->nReqSampleCnt = TRANSC_REDIRRATE_MAX;
        if(!strcasecmp("true",pLdCfg->strCmdArgOutIntfMplsLabel))
            pCntx->bOutIntfInsertMplsTag = TRUE;
        else
            pCntx->bOutIntfInsertMplsTag = FALSE;
        /* At this point all non-interface has been successfully loaded */
        pCntx->bNonIntfCfgLoadSuccess = TRUE;
        /* initialize all interfaces */
        /* First Check to see if we need to reload RX or TX before
         * rewritting the context data structure. */
        tcIntfCkIntf(&(pCntx->tIntfCfg),pLdCfg,
                &_bRxLoad,&_bTxLoad,&_bMapLoad,&_bRedirLoad);
        if(_bTxLoad)
        {
            /* Reloading, set the TX interface to down */
            pCntx->tIntfX.bOutIntfUp = FALSE;
            if(pCntx->tIntfX.bOutIntfOpen)
                tcPktIOTxOutIntfClose(pCntx);
        }
        _sts =
                tcOutIntfConfigYamlInitAllInterfaceTbls(
                        &(pCntx->tIntfX),&(pCntx->tIntfCfg),pLdCfg,
                            _bRxLoad,_bTxLoad,_bMapLoad,_bRedirLoad);
        if(ESUCCESS != _sts)
        {
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                  evLogLvlError,
                  &(pCntx->tLogDescSys),
                  "failure, config.yaml loading, intf init");
            break;
        }
         if(_bTxLoad)
         {
             if(FALSE == pCntx->tIntfX.bOutIntfOpen)
             {
                 _sts = tcPktIOTxOutIntfOpen(pCntx);
                 if(ESUCCESS != _sts)
                 {
                     evLogTrace(
                           pCntx->pQPktGenToBkgrnd,
                           evLogLvlError,
                           &(pCntx->tLogDescSys),
                           "failure to open TX /Interface");
                     break;
                 }
             }
         }
         pCntx->bIntfCfgLoadSuccess = TRUE;
        /* Definition of transc is up :
           transc all system is ready to process.
           - monitoring up,
           - injection up,
           - all threads up,
           - configuration successfully loaded. */
        /* TODO: Add all threads up check */
        if(pCntx->bIntfCfgLoadSuccess &&
           pCntx->bNonIntfCfgLoadSuccess &&
           pCntx->tIntfX.bOutIntfUp)
        {
            pCntx->bTrSts = tcTrStsUp;
        }
    }while(FALSE);

    /* Cleanup operations */
    if(tcTrStsUp == pCntx->bTrSts)
    {
        _sts = tcOutIntfInitMapCheck(
                &(pCntx->tIntfX),&(pCntx->tIntfCfg));
        if(ESUCCESS != _sts)
        {
            pCntx->bTrSts = tcTrStsDown;
            pCntx->bIntfCfgLoadSuccess = FALSE;
            evLogTrace(
                  pCntx->pQPktGenToBkgrnd,
                  evLogLvlError,
                  &(pCntx->tLogDescSys),
                  "failure, map interface link config, please "
                  "check configuration.");
        }
        else
        {
            evLogTrace(
                  pCntx->pQPktGenToBkgrnd,
                  evLogLvlWarn,
                  &(pCntx->tLogDescSys),
                  "Warning, pktgen is UP, Finished reloading config!");
        }
    }

    /* Set Component status */
    tcShProtectedDSetCompSts(tcTRCompTypePktGen,pCntx->bTrSts);

    return (pCntx->bTrSts? ESUCCESS : EFAILURE);
}

