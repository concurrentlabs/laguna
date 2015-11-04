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

#ifdef  TRANSC_BUILDCFG_LIBPCAP
/**************** PRIVATE Functions **********************/

/***************************************************************************
* function: tcPfrPcapInitIsRxIntfLinkUp
*
* description: Check if Rx link is up
****************************************************************************/
CCUR_PROTECTED(BOOL)
tcPlpcapInitIsRxIntfLinkUp(
        tc_pktprc_thread_ctxt_t * pCntx)
{
    return TRUE;
}

/***************************************************************************
* function: _tcPlpcapPktHandler
*
* description: Callback function invoked by libpcap for every incoming packet
****************************************************************************/
CCUR_PRIVATE(void)
_tcPlpcapPktHandler(u_char *param,
                    const struct pcap_pkthdr *header,
                    const u_char *pkt_data)
{
    tresult_t                       _result;
    tc_pktprc_thread_ctxt_t*        _pCntx;
    tc_g_svcdesc_t                  _pSvcType;
    CHAR                            _strDomainName[64];
    tc_g_qmsgpptohp_t*              _tPktDescMsg;
    BOOL                            _bMemFree;

    CCURASSERT(header);
    CCURASSERT(pkt_data);

    do
    {
        _bMemFree = TRUE;
        _pCntx = (tc_pktprc_thread_ctxt_t*)param;
        _pCntx->nPcapPktTotal++;
#if TRANSC_STRMINJTIME
        tPktDescMsg->pktDesc.tRxTime.reserved      = TRANSC_TIMEORIENT_UTC;
        tPktDescMsg->pktDesc.tRxTime.nSeconds      = header->ts.tv_sec;
        tPktDescMsg->pktDesc.tRxTime.nMicroseconds = header->ts.tv_usec;
#endif /* TRANSC_STRMTIME */
        _tPktDescMsg = (tc_g_qmsgpptohp_t*)
                 lkfqMalloc(_pCntx->pQPPktProcToHttpProc);
        if(NULL == _tPktDescMsg)
        {
            _pCntx->nPcapPktIgnored++;
            break;
        }
        _tPktDescMsg->pktDesc.pMsgStrt              = (U8*)pkt_data;
        _tPktDescMsg->pktDesc.nCaplen               = header->caplen;
        _tPktDescMsg->pktDesc.nWireLen              = header->len;
        _result                                     = tcPktParse(&_tPktDescMsg->pktDesc);
        if(EIGNORE == _result)
        {
            _pCntx->nPcapParseIgnored++;
            break;
        }
        else if(ESUCCESS != _result)
        {
            _pCntx->nPcapParseErr++;
            break;
        }
        else if(_tPktDescMsg->pktDesc.tcpHdr.nPyldLen <
                TRANSC_PKTPRC_HTTP_MINPYLD_SZ)
        {
            _pCntx->nPcapPktIgnored++;
            break;
        }
        if(_tPktDescMsg->pktDesc.tcpHdr.nPyldLen)
        {
            /* Write Info to Mib */
            tcPktPrcParseGetDomainName(
                    _pCntx,_strDomainName,
                    sizeof(_strDomainName),
                    (CHAR*)_tPktDescMsg->pktDesc.tcpHdr.pPyld,
                    _tPktDescMsg->pktDesc.tcpHdr.nPyldLen);
            tcPktPrcWriteMibQTraffic(
                    _pCntx,_strDomainName,
                    &(_tPktDescMsg->pktDesc.ipHdr.tSrcIP));
        }
        if(FALSE ==
                tcPktPrcIsRequestServiceAvail(
                        _pCntx,
                        &_pSvcType,
                        _tPktDescMsg->pktDesc.tcpHdr.pPyld,
                        _tPktDescMsg->pktDesc.tcpHdr.nPyldLen))
        {
            _pCntx->nPcapPktIgnored++;
            break;
        }
        if(ESUCCESS !=
                tcPktPrcCkIpBlacklist(_pCntx,&_tPktDescMsg->pktDesc))
        {
            _pCntx->nPcapPktIgnored++;
            break;
        }
        _result =
                tcPktPrcToHttpProcQueueMsg(
                        _pCntx,&_pSvcType,_tPktDescMsg);
        if(ESUCCESS != _result)
            _pCntx->nPcapPktIgnored++;
    }while(FALSE);

    if(_bMemFree)
    {
        if(_tPktDescMsg)
            lkfqFree(_pCntx->pQPPktProcToHttpProc,
                    _tPktDescMsg);
    }

}

//*********************************************************************************
// function: _tcPfrPcapGetNextPkt
//
// description: recieve next packet, polling mode.
//      returns capture length, -1 error.
//*********************************************************************************
CCUR_PRIVATE(I32)
_tcPlpcapGetNextPkt(tc_pktprc_thread_ctxt_t * pCntx)
{
    I32         _rc;
    tresult_t   _result;
    u_char*     _user = (u_char*)pCntx;
    CHAR        _strErrbuf[PCAP_ERRBUF_SIZE];

    if (NULL == pCntx->g_pPcap)
    {
        _result = ESUCCESS;
        /*
         * Check to see if the capture source needs to be opened. This
         *   may occur as a result of an interface being unavailable for
         *   some period of time.
         */
        pCntx->g_pPcap = pcap_open_live(
                pCntx->strPktPrcArgMonIntf,
                TRANSC_PKTPRC_SNAPLEN,TRUE,
                TRANSC_PCAP_READTIMEOUT_MS,
                _strErrbuf);
        if(NULL == pCntx->g_pPcap)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,Unable to open pcap: %s",
                    _strErrbuf);
            sleep(1);
        }
        if (0 > pcap_setnonblock(
                pCntx->g_pPcap, TRUE,
                _strErrbuf))
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,Unable to set to nonblocking: %s",
                    _strErrbuf);
            sleep(1);
        }
    }
    else
        _result = ESUCCESS;

    if(ESUCCESS == _result)
    {
        _rc = pcap_dispatch(
                    pCntx->g_pPcap,
                    TRANSC_PKTPRC_BUFFWTRMRK,
                    _tcPlpcapPktHandler,
                    _user
                    );
        if (_rc <= 0)
        {
            lkfqFlushFreeList(&(pCntx->pQPktProcToBkgrnd->tLkfq));
            lkfqFlushFreeList(pCntx->pQPktProcToMib);
            poll(&(pCntx->g_pfd),
                    TRANSC_PKTPRC_FDNUM,
                    TRANSC_PKTPRC_POLLDURATION_MS);
            tcPktPrcFlushMibTable(pCntx);
        }
    }
    else
        _rc = 0;

    return _rc;
}

/**************** PROTECTED Functions **********************/

/***************************************************************************
* function: tcPlpcapInitLibPcapRx
*
* description: Init libpcap packet capture
****************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPlpcapInitLibPcapRx(
        tc_pktprc_thread_ctxt_t*     pCntx)
{
    tresult_t           _result;
    const CHAR*         _strTsDesc;
    CHAR                _strErrbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program  _fp;      /* The compiled filter */
    bpf_u_int32         _tNet;       /* The IP of our sniffing device */

    CCURASSERT(pCntx);

    do
    {
        _result = ESUCCESS;
        /* Find the properties for the device */
        _tNet = 0;
        pCntx->g_pPcap = pcap_open_live(
                pCntx->strPktPrcArgMonIntf,
                TRANSC_PKTPRC_SNAPLEN,TRUE,
                TRANSC_PCAP_READTIMEOUT_MS,
                _strErrbuf);
        if(NULL == pCntx->g_pPcap)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,Unable to open pcap: %s",
                    _strErrbuf);
            break;
        }
        /*
         * Time stamp types.
         * Not all systems and interfaces will necessarily support all of these.
         *
         * A system that supports PCAP_TSTAMP_HOST is offering time stamps
         * provided by the host machine, rather than by the capture device,
         * but not committing to any characteristics of the time stamp;
         * it will not offer any of the PCAP_TSTAMP_HOST_ subtypes.
         *
         * PCAP_TSTAMP_HOST_LOWPREC is a time stamp, provided by the host machine,
         * that's low-precision but relatively cheap to fetch; it's normally done
         * using the system clock, so it's normally synchronized with times you'd
         * fetch from system calls.
         *
         * PCAP_TSTAMP_HOST_HIPREC is a time stamp, provided by the host machine,
         * that's high-precision; it might be more expensive to fetch.  It might
         * or might not be synchronized with the system clock, and might have
         * problems with time stamps for packets received on different CPUs,
         * depending on the platform.
         *
         * PCAP_TSTAMP_ADAPTER is a high-precision time stamp supplied by the
         * capture device; it's synchronized with the system clock.
         *
         * PCAP_TSTAMP_ADAPTER_UNSYNCED is a high-precision time stamp supplied by
         * the capture device; it's not synchronized with the system clock.
         *
         * Note that time stamps synchronized with the system clock can go
         * backwards, as the system clock can go backwards.  If a clock is
         * not in sync with the system clock, that could be because the
         * system clock isn't keeping accurate time, because the other
         * clock isn't keeping accurate time, or both.
         *
         * Note that host-provided time stamps generally correspond to the
         * time when the time-stamping code sees the packet; this could
         * be some unknown amount of time after the first or last bit of
         * the packet is received by the network adapter, due to batching
         * of interrupts for packet arrival, queueing delays, etc..
         */
        if (0 > pcap_set_tstamp_type(
                pCntx->g_pPcap,PCAP_TSTAMP_ADAPTER))
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Rx hw timestamp is off");
        }
        if (0 > pcap_list_tstamp_types(
                pCntx->g_pPcap,&(pCntx->g_pnLibPcapTstampTypes)))
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,unable to "
                    "get list of libpcap timestamps",
                    _strErrbuf);
            break;
        }
        _strTsDesc = pcap_tstamp_type_val_to_description(
                *(pCntx->g_pnLibPcapTstampTypes));
        if(_strTsDesc)
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Libpcap TimeStamp Description:%s\n",
                    _strTsDesc);
        /*
         * Set the packet read
         * operations into a non-blocking mode.
         */
        if (0 > pcap_setnonblock(
                pCntx->g_pPcap, 1,
                _strErrbuf))
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,Unable to set to nonblocking: %s",
                    _strErrbuf);
            break;
        }
        /* Compile and apply the filter */
        _result = pcap_compile(pCntx->g_pPcap,
                &_fp,
                pCntx->strPktPrcArgRuleset,
                FALSE,
                _tNet);
        if (ESUCCESS != _result)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,Unable to compile libpcap filter");
            break;
        }
        _result = pcap_setfilter(pCntx->g_pPcap, &_fp);
        if (ESUCCESS != _result)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,Unable to set libpcap filter");
            break;
        }
        pCntx->g_pfd.fd = pcap_get_selectable_fd(pCntx->g_pPcap);
        pCntx->g_pfd.events  = POLLIN;
        pCntx->g_pfd.revents = 0;
    }while(FALSE);

    if(ESUCCESS != _result)
    {
        if(pCntx->g_pPcap)
        {
            pcap_close(pCntx->g_pPcap);
            pCntx->g_pPcap = NULL;
        }
    }

    return _result;
}

/***************************************************************************
* function: tcPlpcapShutdownLibPcapRx
*
* description: shutdown libpcap packet capture
****************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPlpcapShutdownLibPcapRx(
        tc_pktprc_thread_ctxt_t* pCntx )
{
    CCURASSERT(pCntx);
    if(pCntx->g_pPcap)
    {
        pcap_breakloop(pCntx->g_pPcap);
        pcap_close(pCntx->g_pPcap);
        if(pCntx->g_pnLibPcapTstampTypes)
            pcap_free_tstamp_types(pCntx->g_pnLibPcapTstampTypes);
        pCntx->g_pPcap = NULL;
    }

    return ESUCCESS;
}

/***************************************************************************
* function: tcPlpcapGetStatLibPcapRx
*
* description: Get libpcap statistics
****************************************************************************/
CCUR_PROTECTED(CHAR*)
tcPlpcapGetStatLibPcapRx(
        tc_pktprc_thread_ctxt_t*    pCntx,
        CHAR*                       strBuff,
        U32                         nstrBuff)
{
    struct pcap_stat _stats;

    pcap_stats(pCntx->g_pPcap,&_stats);

    snprintf(strBuff,
            nstrBuff,
            "       Rx Dropped:%d\n"
            "       Rx Recv:%d",
            _stats.ps_drop,
            _stats.ps_recv);

    return strBuff;
}

/***************************************************************************
* function: tcPlpcapProcessLibPcap
*
 * description: Get packet from libpcap and populate control plane data struct.
 * We only need MAC addr, IP, Port, seq num, ack num, and
 * pointer to HTTP payload.
****************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPlpcapProcessLibPcap(
        tc_pktprc_thread_ctxt_t* pCntx )
{
    U32         _nPcapPktDump = 0;
    U16         _i=0;

    while(!pCntx->bExit)
    {
        /* Read config to see if there are any changes */
        tcPktPrcReadCfg();
        /* Wait until both monitoring and
         * output interfaces are up and running
         * then continue.
         */
        if(FALSE == pCntx->bStrPktPrcArgMonIntfUp)
        {
            tcShDMsgSetPktPrcSts(tcTrStsDown);
            if(_i >= 1000 )
            {
                /* if Rings are not initialized then try to
                 * initialize it */
                if(FALSE == pCntx->bStrPktPrcArgMonIntfSet)
                    tcPktPrcInitIntf(pCntx,TRUE);
                /* Check to make sure both links are
                 * up and running */
                if(pCntx->bStrPktPrcArgMonIntfSet &&
                   tcPktIORxMonIntfIsLinkUp(pCntx))
                    pCntx->bStrPktPrcArgMonIntfUp = TRUE;
                else
                    pCntx->bStrPktPrcArgMonIntfUp = FALSE;
                _i = 0;
            }
            _i++;
            usleep(1000);
        }
        else
        {
            _i = 0;
            tcShDMsgSetPktPrcSts(tcTrStsUp);
            _tcPlpcapGetNextPkt(pCntx);
        }
        _nPcapPktDump++;
        if(TRANSC_PKTPRC_DUMPSTATS_CNTR ==
                _nPcapPktDump)
        {
            tcPktPrcLogStats(pCntx);
            _nPcapPktDump = 0;
        }
    }

    return ESUCCESS;
}

#endif /*  TRANSC_BUILDCFG_LIBPCAP */

