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

#include "pktprc.h"

/**************** PRIVATE Functions **********************/

/**************** PROTECTED Functions **********************/

/* Driver interface to libpcap send (raw interface) or
 * Pfring send ring buffer */

/***************************************************************************
* function: tcPktIORxMonIntfIsLinkUp
*
* description: Check if Monitoring interface link is up
****************************************************************************/
CCUR_PROTECTED(BOOL)
tcPktIORxMonIntfIsLinkUp(
        tc_pktprc_thread_ctxt_t * pCntx)
{
    BOOL _bIsLinkUp;
#if TRANSC_BUILDCFG_LIBPCAP
    _bIsLinkUp = tcPlpcapInitIsRxIntfLinkUp(pCntx);
#else
    _bIsLinkUp = tcPfrPcapInitIsRxIntfLinkUp(pCntx);
#endif
    return _bIsLinkUp;
}

/***************************************************************************
 * function: tcPktIORxMonIntfOpen
 *
 * description: Generic call to open/initialize Packet
 * processing Monitoring interface.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktIORxMonIntfOpen(
        tc_pktprc_thread_ctxt_t*     pCntx)
{
    tresult_t                   _result;
    tc_monintf_mon_t*           _pMonIntf;
    tc_monintf_fltr_t*          _pMonIntffltr;
    U16                         _i;
    U16                         _j;

    if(0 == pCntx->tIntfX.nMonIntfTotal ||
       pCntx->tIntfX.nMonIntfTotal >= TRANSC_INTERFACE_MAX)
        _result = EFAILURE;
    else
    {
        _pMonIntf       = NULL;
        _pMonIntffltr   = NULL;
        _result         = ESUCCESS;
        evLogTrace(
                pCntx->pQPktProcToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "Monitoring Interface total:%d",
                pCntx->tIntfX.nMonIntfTotal-0);
        pCntx->tIntfX.nMonIntfOpen = 0;
        for(_i=0;_i<pCntx->tIntfX.nMonIntfTotal;_i++)
        {
            _pMonIntf = &(pCntx->tIntfX.tMonIntfTbl[_i]);
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Monitoring Interface \"%s:%s\" filters total:%d",
                    _pMonIntf->tIntf.strIntfName,
                    _pMonIntf->tIntf.strIntfVal,
                    _pMonIntf->nFilterTotal);
            if(0 == _pMonIntf->nFilterTotal ||
               _pMonIntf->nFilterTotal >= TRANSC_INTERFACE_FLTR_MAX)
            {
                _result = EFAILURE;
                break;
            }
            for(_j=0;_j<_pMonIntf->nFilterTotal;_j++)
            {
                _pMonIntffltr = &(_pMonIntf->tFilterTbl[_j]);
                /* Skip this one */
                if(_pMonIntffltr->pRingMonIntf)
                    continue;
#ifdef TRANSC_BUILDCFG_LIBPCAP
                _result = tcPlpcapInitLibPcapRx(pCntx,_i,_j);
#else
                _result = tcPfrPcapInitLibPfringRx(pCntx,_i,_j);
#endif /* TRANSC_BUILDCFG_LIBPCAP */
                if(ESUCCESS != _result)
                    break;
                _pMonIntf->nFilterActv++;
                evLogTrace(
                        pCntx->pQPktProcToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "Initializing Monitoring Interface \"%s:%s\" filter:%s",
                        _pMonIntf->tIntf.strIntfName,
                        _pMonIntf->tIntf.strIntfVal,
                        _pMonIntffltr->strRuleset);
            }
            if(ESUCCESS != _result)
            {
                if(_pMonIntf)
                {
                    evLogTrace(
                            pCntx->pQPktProcToBkgrnd,
                            evLogLvlError,
                            &(pCntx->tLogDescSys),
                            "Initializing Monitoring Interface \"%s:%s\" failure!",
                            _pMonIntf->tIntf.strIntfName,
                            _pMonIntf->tIntf.strIntfVal);
                }
                else
                {
                    evLogTrace(
                            pCntx->pQPktProcToBkgrnd,
                            evLogLvlError,
                            &(pCntx->tLogDescSys),
                            "Initializing Monitoring Interface \"null:null\" failure!");
                }
                if(_pMonIntffltr)
                {
                    evLogTrace(
                            pCntx->pQPktProcToBkgrnd,
                            evLogLvlError,
                            &(pCntx->tLogDescSys),
                            "Initializing Monitoring Interface \"%s:%s\" filter:%s failure!",
                            _pMonIntf->tIntf.strIntfName,
                            _pMonIntf->tIntf.strIntfVal,
                            _pMonIntffltr->strRuleset);
                }
                else
                {
                    if(_pMonIntf)
                    {
                        evLogTrace(
                              pCntx->pQPktProcToBkgrnd,
                              evLogLvlError,
                              &(pCntx->tLogDescSys),
                              "Initializing Monitoring Interface \"%s:%s\" filter:null failure!",
                              _pMonIntf->tIntf.strIntfName,
                              _pMonIntf->tIntf.strIntfVal);
                    }
                    else
                        evLogTrace(
                                pCntx->pQPktProcToBkgrnd,
                                evLogLvlError,
                                &(pCntx->tLogDescSys),
                                "Initializing Monitoring Interface \"null:null\" filter:null failure!");
                }
                break;
            }
            if(_pMonIntf->nFilterActv &&
               _pMonIntf->nFilterTotal == _pMonIntf->nFilterActv)
                pCntx->tIntfX.nMonIntfOpen++;
        }
    }
    if(pCntx->tIntfX.nMonIntfOpen > 0 &&
       pCntx->tIntfX.nMonIntfTotal == pCntx->tIntfX.nMonIntfOpen)
    {
        pCntx->tIntfX.bMonIntfOpen = TRUE;
        evLogTrace(
                pCntx->pQPktProcToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "Initializing Monitoring Interface %s complete!",
                pCntx->tIntfCfg.strPktPrcArgMonIntf);
        pCntx->tIntfX.bMonIntfUp =
                tcPfrPcapInitIsRxIntfLinkUp(pCntx);
        _result         = ESUCCESS;
    }
    else
    {
        pCntx->tIntfX.bMonIntfOpen = FALSE;
        if('\0' == pCntx->tIntfCfg.strPktPrcArgMonIntf[0])
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Initializing Monitoring Interface \"null\" failure!",
                    pCntx->tIntfCfg.strPktPrcArgMonIntf);
        }
        else
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Initializing Monitoring Interface %s failure!",
                    pCntx->tIntfCfg.strPktPrcArgMonIntf);
        _result         = EFAILURE;
    }
    evLogTrace(
            pCntx->pQPktProcToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "Monitoring interface Active/Total:%d/%d",
            pCntx->tIntfX.nMonIntfOpen-0,
            pCntx->tIntfX.nMonIntfTotal-0);
    return _result;
}

/***************************************************************************
 * function: tcPktIORxMonIntfClose
 *
 * description: Generic call to close/destroy Packet
 * processing Monitoring interface.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcPktIORxMonIntfClose(
        tc_pktprc_thread_ctxt_t*     pCntx)
{
    tc_monintf_mon_t*           _pMonIntf;
    tc_monintf_fltr_t*          _pMonIntffltr;
    U16                         _i;
    U16                         _j;

    if(pCntx->tIntfX.nMonIntfTotal > 0 &&
       pCntx->tIntfX.nMonIntfTotal <= TRANSC_INTERFACE_MAX)
    {
        _pMonIntf       = NULL;
        _pMonIntffltr   = NULL;
        for(_i=0;_i<TRANSC_INTERFACE_MAX;_i++)
        {
            _pMonIntf = &(pCntx->tIntfX.tMonIntfTbl[_i]);
            if((NULL == _pMonIntf) ||
               (0 == _pMonIntf->nFilterActv ||
               _pMonIntf->nFilterActv >= TRANSC_INTERFACE_FLTR_MAX))
                continue;
            for(_j=0;_j<TRANSC_INTERFACE_FLTR_MAX;_j++)
            {
                _pMonIntffltr = &(_pMonIntf->tFilterTbl[_j]);
                if(NULL == _pMonIntffltr ||
                   NULL == _pMonIntffltr->pRingMonIntf)
                    continue;
#ifdef TRANSC_BUILDCFG_LIBPCAP
                tcPlpcapShutdownLibPcapMonIntf(
                        pCntx,_pMonIntffltr);
#else
                tcPfrPcapShutdownLibPfringRx(
                        pCntx,_pMonIntffltr);
#endif /* TRANSC_BUILDCFG_LIBPCAP */
                if(_pMonIntf->nFilterActv)
                    _pMonIntf->nFilterActv--;
                evLogTrace(
                        pCntx->pQPktProcToBkgrnd,
                       evLogLvlInfo,
                       &(pCntx->tLogDescSys),
                       "Shutting Monitoring Interface \"%s:%s:%s\" filter success!"
                       " Active/Total:%d/%d",
                       _pMonIntf->tIntf.strIntfName,
                       _pMonIntf->tIntf.strIntfVal,
                       _pMonIntffltr->strRuleset,
                       _pMonIntf->nFilterActv,
                       _pMonIntf->nFilterTotal);
                /* Clear monitor interface filter dstruct */
                ccur_memclear(_pMonIntffltr,
                        sizeof(tc_monintf_fltr_t));
            }
            if(0 == _pMonIntf->nFilterActv)
            {
                if(pCntx->tIntfX.nMonIntfOpen)
                    pCntx->tIntfX.nMonIntfOpen--;
                evLogTrace(
                        pCntx->pQPktProcToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "Shutting Monitoring Interface \"%s:%s\" filters complete!",
                        _pMonIntf->tIntf.strIntfName,
                        _pMonIntf->tIntf.strIntfVal);
                /* Clear monitor interface dstruct */
                ccur_memclear(_pMonIntf,
                        sizeof(tc_monintf_mon_t));

            }
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Monitoring Interface %s Active/Total:%d/%d",
                    pCntx->tIntfCfg.strPktPrcArgMonIntf,
                    pCntx->tIntfX.nMonIntfOpen-0,
                    pCntx->tIntfX.nMonIntfTotal-0);
        }
    }
    if(0 == pCntx->tIntfX.nMonIntfOpen)
    {
        pCntx->tIntfX.nMonIntfTotal = 0;
        pCntx->tIntfX.bMonIntfOpen = FALSE;
        pCntx->tIntfX.bMonIntfUp   = FALSE;
    }
    else
    {
        pCntx->tIntfX.bMonIntfOpen = TRUE;
        if('\0' == pCntx->tIntfCfg.strPktPrcArgMonIntf[0])
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlFatal,
                    &(pCntx->tLogDescSys),
                    "Shutting down Monitoring Interface \"null\" failure!");
        }
        else
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlFatal,
                    &(pCntx->tLogDescSys),
                    "Shutting down Monitoring Interface %s failure!",
                    pCntx->tIntfCfg.strPktPrcArgMonIntf);
        }
    }
    evLogTrace(
            pCntx->pQPktProcToBkgrnd,
            evLogLvlInfo,
            &(pCntx->tLogDescSys),
            "Monitoring interface %s Total avail:%d",
            pCntx->tIntfCfg.strPktPrcArgMonIntf,
            pCntx->tIntfX.nMonIntfTotal-0);
}

/***************************************************************************
 * function: tcPktIORxMonIntfGetStat
 *
 * description: Generic call to get statistics for Monitoring interface
 ***************************************************************************/
CCUR_PROTECTED(CHAR*)
tcPktIORxMonIntfGetStat(
        tc_pktprc_thread_ctxt_t*     pCntx,
        CHAR*                        strBuff,
        U32                          nstrBuff)
{
#ifdef TRANSC_BUILDCFG_LIBPCAP
    return(tcPlpcapGetStatLibPcapRx(pCntx,strBuff,nstrBuff));
#else
    return(tcPfrPcapGetStatPfringRx(pCntx,strBuff,nstrBuff));
#endif
}

/***************************************************************************
 * function: tcPktIORxMonIntfFromWire
 *
 * description: Generic call read from wire
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktIORxMonIntfFromWire(
        tc_pktprc_thread_ctxt_t* pCntx)
{
    tresult_t           _result;

    CCURASSERT(pCntx);

#if TRANSC_BUILDCFG_LIBPCAP
    _result = tcPlpcapProcessLibPcap(pCntx);
#else
    _result = tcPfrPcapProcessLibPfring(pCntx);
#endif /* TRANSC_BUILDCFG_LIBPCAP */

    return _result;
}
