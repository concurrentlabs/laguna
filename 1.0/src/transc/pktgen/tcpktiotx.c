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

/**************** PRIVATE Functions **********************/

/**************** PROTECTED Functions **********************/

/* Driver interface to libpcap send (raw interface) or
 * Pfring send ring buffer */


/***************************************************************************
 * function: tcPktIOTxOutIntfIsLinkUp
 *
 * Generic call to close/destroy Packet processing Transmit interface.
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcPktIOTxOutIntfIsLinkUp(
        tc_pktgen_thread_ctxt_t* pCntx )
{
    BOOL _bIsLinkUp;
#if TRANSC_BUILDCFG_LIBPCAP
    _bIsLinkUp = tcPlPsendInitIsTxIntfLinkUp(pCntx);
#else
    _bIsLinkUp = tcPfrPsendInitIsTxIntfLinkUp(pCntx);
#endif
    return _bIsLinkUp;
}

/***************************************************************************
 * function: tcPktIORxMonIntfOpen
 *
 * description: Generic call to open/initialize Packet
 *  processing Transmit interface.
 **************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktIOTxOutIntfOpen(
        tc_pktgen_thread_ctxt_t* pCntx )
{
    tresult_t                   _result;
    tc_outintf_out_t*        _pOutIntf;
    U16                         _i;

    if(0 == pCntx->tIntfX.nOutIntfTotal ||
       pCntx->tIntfX.nOutIntfTotal >= TRANSC_INTERFACE_MAX)
        _result = EFAILURE;
    else
    {
        _pOutIntf       = NULL;
        _result         = ESUCCESS;
        evLogTrace(
                pCntx->pQPktGenToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "Outgoing Interface total:%d",
                pCntx->tIntfX.nOutIntfTotal-0);
        pCntx->tIntfX.nOutIntfOpen = 0;
        for(_i=0;_i<pCntx->tIntfX.nOutIntfTotal;_i++)
        {
            _pOutIntf = &(pCntx->tIntfX.tOutIntfTbl[_i]);
            if(NULL == _pOutIntf)
            {
                _result = EFAILURE;
                evLogTrace(
                        pCntx->pQPktGenToBkgrnd,
                        evLogLvlError,
                        &(pCntx->tLogDescSys),
                        "Initializing Outgoing Interface \"null:null\" failure!");
                break;
            }
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Outgoing Interface \"%s:%s\"",
                    _pOutIntf->tIntf.strIntfName,
                    _pOutIntf->tIntf.strIntfVal);
#ifdef TRANSC_BUILDCFG_LIBPCAP
            _result = tcPlPsendInitLibPfringTx(pCntx,_i);
#else
            _result = tcPfrPsendInitLibPfringTx(pCntx,_i);
#endif /* TRANSC_BUILDCFG_LIBPCAP */
            if(ESUCCESS != _result)
                break;
            pCntx->tIntfX.nOutIntfOpen++;
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Initializing Outgoing Interface \"%s:%s\"",
                    _pOutIntf->tIntf.strIntfName,
                    _pOutIntf->tIntf.strIntfVal);
            if(ESUCCESS != _result)
            {
                evLogTrace(
                        pCntx->pQPktGenToBkgrnd,
                        evLogLvlError,
                        &(pCntx->tLogDescSys),
                        "Initializing Outgoing Interface \"%s:%s\" failure!",
                        _pOutIntf->tIntf.strIntfName,
                        _pOutIntf->tIntf.strIntfVal);
                break;
            }
        }
    }
    if(pCntx->tIntfX.nOutIntfOpen > 0 &&
       pCntx->tIntfX.nOutIntfTotal == pCntx->tIntfX.nOutIntfOpen)
    {
        pCntx->tIntfX.bOutIntfOpen = TRUE;
        evLogTrace(
                pCntx->pQPktGenToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "Initializing Outgoing Interface %s complete!",
                pCntx->tIntfCfg.strPktPrcArgOutIntf);
        pCntx->tIntfX.bOutIntfUp =
                tcPktIOTxOutIntfIsLinkUp(pCntx);
        _result         = ESUCCESS;
    }
    else
    {
        pCntx->tIntfX.bOutIntfOpen = FALSE;
        if('\0' == pCntx->tIntfCfg.strPktPrcArgOutIntf[0])
        {
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Initializing Outgoing Interface \"null\" failure!",
                    pCntx->tIntfCfg.strPktPrcArgOutIntf);
        }
        else
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Initializing Outgoing Interface %s failure!",
                    pCntx->tIntfCfg.strPktPrcArgOutIntf);
        _result         = EFAILURE;
    }
    evLogTrace(
            pCntx->pQPktGenToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "Outgoing interface Active/Total:%d/%d",
            pCntx->tIntfX.nOutIntfOpen-0,
            pCntx->tIntfX.nOutIntfTotal-0);
    return _result;
}

/***************************************************************************
 * function: tcPktIORxMonIntfClose
 *
 * Generic call to close/destroy Packet processing Transmit interface.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktIOTxOutIntfClose(
        tc_pktgen_thread_ctxt_t* pCntx )
{
    tc_outintf_out_t*        _pOutIntf;
    U16                         _i;

    if(pCntx->tIntfX.nOutIntfTotal > 0 &&
       pCntx->tIntfX.nOutIntfTotal <= TRANSC_INTERFACE_MAX)
    {
        _pOutIntf       = NULL;
        for(_i=0;_i<TRANSC_INTERFACE_MAX;_i++)
        {
            _pOutIntf = &(pCntx->tIntfX.tOutIntfTbl[_i]);
            if(NULL == _pOutIntf->pRingOutIntf)
                continue;
#if TRANSC_BUILDCFG_LIBPCAP
            tcPlPsendShutdownLibPfringTx(_pOutIntf);
#else
            tcPfrPsendShutdownLibPfringTx(_pOutIntf);
#endif
            if(pCntx->tIntfX.nOutIntfOpen)
                pCntx->tIntfX.nOutIntfOpen--;
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                   evLogLvlInfo,
                   &(pCntx->tLogDescSys),
                   "Shutting Outgoing Interface \"%s:%s\" success!",
                   _pOutIntf->tIntf.strIntfName,
                   _pOutIntf->tIntf.strIntfVal);
            if(0 == pCntx->tIntfX.nOutIntfOpen)
            {
                evLogTrace(
                        pCntx->pQPktGenToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "Shutting Outgoing Interface \"%s:%s\" complete!",
                        _pOutIntf->tIntf.strIntfName,
                        _pOutIntf->tIntf.strIntfVal);
                /* Clear outgoing interface dstruct */
                ccur_memclear(_pOutIntf,
                        sizeof(tc_outintf_out_t));
            }
        }
        evLogTrace(
                pCntx->pQPktGenToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "Outgoing Interface %s Active/Total:%d/%d",
                pCntx->tIntfCfg.strPktPrcArgOutIntf,
                pCntx->tIntfX.nOutIntfOpen-0,
                pCntx->tIntfX.nOutIntfTotal-0);
    }
    if(0 == pCntx->tIntfX.nOutIntfOpen)
    {
        pCntx->tIntfX.nOutIntfTotal = 0;
        pCntx->tIntfX.bOutIntfOpen = FALSE;
        pCntx->tIntfX.bOutIntfUp   = FALSE;
    }
    else
    {
        pCntx->tIntfX.bOutIntfOpen = TRUE;
        if('\0' == pCntx->tIntfCfg.strPktPrcArgOutIntf[0])
        {
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlFatal,
                    &(pCntx->tLogDescSys),
                    "Shutting down Outgoing Interface \"null\" failure!");
        }
        else
        {
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlFatal,
                    &(pCntx->tLogDescSys),
                    "Shutting down Outgoing Interface %s failure!",
                    pCntx->tIntfCfg.strPktPrcArgOutIntf);
        }
    }
    evLogTrace(
            pCntx->pQPktGenToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "Outgoing interface %s Total avail:%d",
            pCntx->tIntfCfg.strPktPrcArgOutIntf,
            pCntx->tIntfX.nOutIntfTotal-0);

}

/***************************************************************************
 * tcPktIOTxOutIntfGetStat
 *
 * description: Generic call to get statistics for Transmit interface
 ***************************************************************************/
CCUR_PROTECTED(CHAR*)
tcPktIOTxOutIntfGetStat(
        tc_pktgen_thread_ctxt_t*     pCntx,
        CHAR*                        strBuff,
        U32                          nstrBuff)
{
#ifdef TRANSC_BUILDCFG_LIBPCAP
    return(tcPlPsendGetStatLibPcapTx(pCntx,strBuff,nstrBuff));
#else
    return(tcPfrPsendGetStatPfringTx(pCntx,strBuff,nstrBuff));
#endif
}

/***************************************************************************
 * tcPktIOTxOutIntfToWire
 *
 * description: Generic call write to wire
 ***************************************************************************/
CCUR_PROTECTED(I32)
tcPktIOTxOutIntfToWire(
        tc_pktgen_thread_ctxt_t* pCntx,
        tc_outintf_out_t*       pOutIntf,
        tc_pktgen_pktinj_t*      pkt)
{
    I32         _nPktsSent;

    CCURASSERT(pkt);

#if TRANSC_BUILDCFG_LIBPCAP
    _nPktsSent =
            tcPlPsendLibPcapTx(
                    pCntx,pOutIntf,pkt);
#else
    _nPktsSent =
            tcPfrPsendLibPfringTx(
                    pCntx,pOutIntf,pkt);
#endif

    return(_nPktsSent);
}

