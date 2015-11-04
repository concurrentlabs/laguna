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

#ifdef PFRING_DBG
#include <net/ethernet.h>     /* the L2 protocols */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#endif
#include "pktprc.h"

#ifndef TRANSC_BUILDCFG_LIBPCAP
/**************** PRIVATE Functions **********************/

#ifdef PFRING_DBG
/* grabbed From pfring example for pkt debugging...*/
/*
 * A faster replacement for inet_ntoa().
 */
char* _intoa(unsigned int addr, char* buf, u_short bufLen) {
  char *cp, *retStr;
  u_int byte;
  int n;

  cp = &buf[bufLen];
  *--cp = '\0';

  n = 4;
  do {
    byte = addr & 0xff;
    *--cp = byte % 10 + '0';
    byte /= 10;
    if (byte > 0) {
      *--cp = byte % 10 + '0';
      byte /= 10;
      if (byte > 0)
    *--cp = byte + '0';
    }
    *--cp = '.';
    addr >>= 8;
  } while (--n > 0);

  /* Convert the string to lowercase */
  retStr = (char*)(cp+1);

  return(retStr);
}


static char hex[] = "0123456789ABCDEF";

char* etheraddr_string(const u_char *ep, char *buf) {
  u_int i, j;
  char *cp;

  cp = buf;
  if ((j = *ep >> 4) != 0)
    *cp++ = hex[j];
  else
    *cp++ = '0';

  *cp++ = hex[*ep++ & 0xf];

  for(i = 5; (int)--i >= 0;) {
    *cp++ = ':';
    if ((j = *ep >> 4) != 0)
      *cp++ = hex[j];
    else
      *cp++ = '0';

    *cp++ = hex[*ep++ & 0xf];
  }

  *cp = '\0';
  return (buf);
}

char* intoa(unsigned int addr) {
  static char buf[sizeof "ff:ff:ff:ff:ff:ff:255.255.255.255"];

  return(_intoa(addr, buf, sizeof(buf)));
}

/* ************************************ */

inline char* in6toa(struct in6_addr addr6) {
  static char buf[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"];

  snprintf(buf, sizeof(buf),
       "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
       addr6.s6_addr[0], addr6.s6_addr[1], addr6.s6_addr[2],
       addr6.s6_addr[3], addr6.s6_addr[4], addr6.s6_addr[5], addr6.s6_addr[6],
       addr6.s6_addr[7], addr6.s6_addr[8], addr6.s6_addr[9], addr6.s6_addr[10],
       addr6.s6_addr[11], addr6.s6_addr[12], addr6.s6_addr[13], addr6.s6_addr[14],
       addr6.s6_addr[15]);

  return(buf);
}

char* proto2str(u_short proto) {
  static char protoName[8];

  switch(proto) {
  case IPPROTO_TCP:  return("TCP");
  case IPPROTO_UDP:  return("UDP");
  case IPPROTO_ICMP: return("ICMP");
  default:
    snprintf(protoName, sizeof(protoName), "%d", proto);
    return(protoName);
  }
}

static int32_t thiszone;

void verboseProcessing(const struct pfring_pkthdr *h, const u_char *p) {

    struct ether_header ehdr;
    u_short eth_type, vlan_id;
    char buf1[32], buf2[32];
    struct ip ip;
    int s;
    uint usec;
    uint nsec=0;

    /*if(h->ts.tv_sec == 0) {
      memset((void*)&h->extended_hdr.parsed_pkt, 0, sizeof(struct pkt_parsing_info));
      pfring_parse_pkt((u_char*)p, (struct pfring_pkthdr*)h, 4, 1, 1);
    }*/

    s = (h->ts.tv_sec + thiszone) % 86400;

    if(h->extended_hdr.timestamp_ns) {
      /* be careful with drifts mixing sys time and hw timestamp */
      usec = (h->extended_hdr.timestamp_ns / 1000) % 1000000;
      nsec = h->extended_hdr.timestamp_ns % 1000;
    } else {
      usec = h->ts.tv_usec;
    }

    printf("%02d:%02d:%02d.%06u%03u ",
       s / 3600, (s % 3600) / 60, s % 60,
       usec, nsec);

    if(h->extended_hdr.parsed_header_len > 0) {
      printf("[eth_type=0x%04X]",
         h->extended_hdr.parsed_pkt.eth_type);
      printf("[l3_proto=%u]", (unsigned int)h->extended_hdr.parsed_pkt.l3_proto);

      printf("[%s:%d -> ", (h->extended_hdr.parsed_pkt.eth_type == 0x86DD) ?
         in6toa(h->extended_hdr.parsed_pkt.ipv6_src) : intoa(h->extended_hdr.parsed_pkt.ipv4_src),
         h->extended_hdr.parsed_pkt.l4_src_port);
      printf("%s:%d] ", (h->extended_hdr.parsed_pkt.eth_type == 0x86DD) ?
         in6toa(h->extended_hdr.parsed_pkt.ipv6_dst) : intoa(h->extended_hdr.parsed_pkt.ipv4_dst),
         h->extended_hdr.parsed_pkt.l4_dst_port);

      printf("[%s -> %s] ",
         etheraddr_string(h->extended_hdr.parsed_pkt.smac, buf1),
         etheraddr_string(h->extended_hdr.parsed_pkt.dmac, buf2));
    }

    memcpy(&ehdr, p+h->extended_hdr.parsed_header_len, sizeof(struct ether_header));
    eth_type = ntohs(ehdr.ether_type);

    printf("[%s][if_index=%d][%s -> %s][eth_type=0x%04X] ",
       h->extended_hdr.rx_direction ? "RX" : "TX",
       h->extended_hdr.if_index,
       etheraddr_string(ehdr.ether_shost, buf1),
       etheraddr_string(ehdr.ether_dhost, buf2), eth_type);


    if(eth_type == 0x8100) {
      vlan_id = (p[14] & 15)*256 + p[15];
      eth_type = (p[16])*256 + p[17];
      printf("[vlan %u] ", vlan_id);
      p+=4;
    }

    if(eth_type == 0x0800) {
      memcpy(&ip, p+h->extended_hdr.parsed_header_len+sizeof(ehdr), sizeof(struct ip));
      printf("[%s]", proto2str(ip.ip_p));
      printf("[%s:%d ", intoa(ntohl(ip.ip_src.s_addr)), h->extended_hdr.parsed_pkt.l4_src_port);
      printf("-> %s:%d] ", intoa(ntohl(ip.ip_dst.s_addr)), h->extended_hdr.parsed_pkt.l4_dst_port);

      printf("[hash=%u][tos=%d][tcp_seq_num=%u][caplen=%d][len=%d][parsed_header_len=%d]"
         "[eth_offset=%d][l3_offset=%d][l4_offset=%d][payload_offset=%d]\n",
         h->extended_hdr.pkt_hash,
         h->extended_hdr.parsed_pkt.ipv4_tos, h->extended_hdr.parsed_pkt.tcp.seq_num,
         h->caplen, h->len, h->extended_hdr.parsed_header_len,
         h->extended_hdr.parsed_pkt.offset.eth_offset,
         h->extended_hdr.parsed_pkt.offset.l3_offset,
         h->extended_hdr.parsed_pkt.offset.l4_offset,
         h->extended_hdr.parsed_pkt.offset.payload_offset);

    } else {
      if(eth_type == 0x0806)
    printf("[ARP]");
      else
    printf("[eth_type=0x%04X]", eth_type);

      printf("[caplen=%d][len=%d][parsed_header_len=%d]"
         "[eth_offset=%d][l3_offset=%d][l4_offset=%d][payload_offset=%d]\n",
         h->caplen, h->len, h->extended_hdr.parsed_header_len,
         h->extended_hdr.parsed_pkt.offset.eth_offset,
         h->extended_hdr.parsed_pkt.offset.l3_offset,
         h->extended_hdr.parsed_pkt.offset.l4_offset,
         h->extended_hdr.parsed_pkt.offset.payload_offset);
    }

}
#endif


//*********************************************************************************
// function: tcPfrPcapSetRingHttpFilter
//
// description: setup TCS filter
//*********************************************************************************
CCUR_PRIVATE(tresult_t)
_tcPfrPcapSetRingHttpFilter(
        tc_pktprc_thread_ctxt_t *   pCntx,
        tc_monintf_fltr_t*          pMonIntffltr)
{
    tresult_t       _result;

    CCURASSERT(pCntx);
    CCURASSERT(pMonIntffltr);

    do
    {
        _result = EFAILURE;
        if(NULL == pMonIntffltr)
            break;
        if((pMonIntffltr->pRingMonIntf) &&
           ('\0' != pMonIntffltr->strRuleset[0]))
        {
            /* if no match on packets then drop */
            /*_result = pfring_toggle_filtering_policy
                    ( pCntx->tMonIntf[nIntfIdx].pRingMonIntf, 0);*/
            _result = pfring_set_bpf_filter(pMonIntffltr->pRingMonIntf,
                                            pMonIntffltr->strRuleset);
        }
    }while(FALSE);

    return _result;
}

/**************** PROTECTED Functions **********************/
/***************************************************************************
* function: tcPfrPcapInitIsRxIntfLinkUp
*
* description: Check if Monitoring interface link is up
****************************************************************************/
CCUR_PROTECTED(BOOL)
tcPfrPcapInitIsRxIntfLinkUp(
        tc_pktprc_thread_ctxt_t * pCntx)
{
    BOOL                        _bIntfUp;
    tc_monintf_mon_t*           _pMonIntf;
    tc_monintf_fltr_t*          _pMonIntffltr;
    U16                         _i;
    U16                         _j;

    CCURASSERT(pCntx);

    _bIntfUp        = FALSE;
    _pMonIntf       = NULL;
    _pMonIntffltr   = NULL;
    for(_j=0;_j<pCntx->tIntfX.nMonIntfOpen;_j++)
    {
        _pMonIntf = &(pCntx->tIntfX.tMonIntfTbl[_j]);
        if('\0' != _pMonIntf->tIntf.strIntfName[0])
        {
            for(_i=0;_i<_pMonIntf->nFilterActv;_i++)
            {
                _pMonIntffltr = &(_pMonIntf->tFilterTbl[_i]);
                if(0 ==
                        pfring_get_link_status(
                                _pMonIntffltr->pRingMonIntf))
                {
                    _pMonIntf->tIntf.bIntfRdy = FALSE;
                }
                else
                {
                    evLogTrace(
                            pCntx->pQPktProcToBkgrnd,
                            evLogLvlInfo,
                            &(pCntx->tLogDescSys),
                            "Checking, Monitoring link \"%s:%s/%s\" Up!",
                            _pMonIntf->tIntf.strIntfName,
                            _pMonIntf->tIntf.strIntfVal,
                            _pMonIntffltr->strRuleset);
                    _pMonIntf->tIntf.bIntfRdy = TRUE;
                }
            }
        }
    }
    /* Transc won't be up if one interface is down
     * during data structure init */
    if(pCntx->tIntfX.nMonIntfOpen > 0)
        _bIntfUp        = TRUE;
    else
        _bIntfUp        = FALSE;
    for(_i=0;_i<pCntx->tIntfX.nMonIntfOpen;_i++)
    {
        if(FALSE ==
                pCntx->tIntfX.tMonIntfTbl[_i].tIntf.bIntfRdy)
        {
            _bIntfUp = FALSE;
            break;
        }
    }
    if(FALSE == _bIntfUp)
    {
        if(_pMonIntf)
        {
                evLogTrace(
                        pCntx->pQPktProcToBkgrnd,
                        evLogLvlError,
                        &(pCntx->tLogDescSys),
                        "Checking, Monitoring link \"%s:%s\" is down!, error - link is not yet ready",
                        _pMonIntf->tIntf.strIntfName,
                        _pMonIntf->tIntf.strIntfVal);
        }
        else
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Checking, Monitoring link \"null\" is down!, error - link is not yet ready");
        }
    }

    return _bIntfUp;
}

/***************************************************************************
* function: tcPfrPcapInitLibPfringRx
*
* description: Init pfring Monitoring interface
****************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPfrPcapInitLibPfringRx(
        tc_pktprc_thread_ctxt_t * pCntx,
        U16                       nIntfIdx,
        U16                       nFilterIdx)
{
    int                         _rc;
    struct timespec             _ltime;
    tresult_t                   _result;
    unsigned int                _version;
    CHAR                        _strBuff[128];
    packet_direction            _pktDir;
    tc_monintf_mon_t*           _pMonIntf;
    tc_monintf_fltr_t*          _pMonIntffltr;

    CCURASSERT(pCntx);

    do
    {
        _pMonIntf       = NULL;
        _pMonIntffltr   = NULL;
        _result         = EINVAL;
        if(nIntfIdx >= TRANSC_INTERFACE_MAX)
            break;
        _pMonIntf = &(pCntx->tIntfX.tMonIntfTbl[nIntfIdx]);
        if(NULL == _pMonIntf)
            break;
        if(nFilterIdx >= TRANSC_INTERFACE_FLTR_MAX)
            break;
        _pMonIntffltr = &(_pMonIntf->tFilterTbl[nFilterIdx]);
        if(NULL == _pMonIntffltr)
            break;
        if('\0' == _pMonIntf->tIntf.strIntfName[0])
            break;
        if('\0' == _pMonIntf->tIntf.strIntfVal[0])
            break;
        if('\0' == _pMonIntffltr->strRuleset[0])
            break;
        /* Open Pfring */
        _pMonIntffltr->pRingMonIntf = pfring_open(
                        _pMonIntf->tIntf.strIntfName,
                        TRANSC_PKTPRC_SNAPLEN,
                        PF_RING_PROMISC | PF_RING_LONG_HEADER);
        if(NULL == _pMonIntffltr->pRingMonIntf)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "pfring_open on %s error - errno:[%s]",
                    _pMonIntf->tIntf.strIntfName, strerror(errno));
            break;
        }
        _rc = pfring_version(_pMonIntffltr->pRingMonIntf,&_version);
        if(0 != _rc )
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, unable to get pfring version number");
            break;
        }
        if(_version < TRANSC_COMPAT_PFRING_VER)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlFatal,
                    &(pCntx->tLogDescSys),
                    "Invalid pfring library version! "
                    "minimal compat needed %s, currently used %s",
                    tcUtilsGetPfringVer(_strBuff,TRANSC_COMPAT_PFRING_VER),
                    tcUtilsGetPfringVer(_strBuff,RING_VERSION_NUM));
            break;
        }
        evLogTrace(
                pCntx->pQPktProcToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                "PF_RING Kernel Module:%s",
                tcUtilsGetPfringVer(_strBuff,_version));
        /* Set polling */
        _pMonIntf->tMonIntfPollFdTbl[nFilterIdx].fd = pfring_get_selectable_fd(_pMonIntffltr->pRingMonIntf);
        _pMonIntf->tMonIntfPollFdTbl[nFilterIdx].events  = POLLIN;
        _pMonIntf->tMonIntfPollFdTbl[nFilterIdx].revents = 0;
        sprintf(_strBuff,"transc_rx_%d_%d",nIntfIdx,nFilterIdx);
        _rc = pfring_set_application_name(_pMonIntffltr->pRingMonIntf,_strBuff);
        if(0 != _rc )
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,Rx Unable to set application name");
            break;
        }
        /* int pfring_set_device_clock(pfring *ring, struct timespec *ts)
          Sets the time in the device hardware clock, when the adapter supports
          hardware timestamping. */
        if((PF_RING_HW_TIMESTAMP) &&
           (clock_gettime(CLOCK_REALTIME, &_ltime) != 0 ||
            pfring_set_device_clock(_pMonIntffltr->pRingMonIntf,&_ltime) < 0))
        {
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Rx hw timestamp is off");
        }
        if(!ccur_strcasecmp(_pMonIntf->tIntf.strIntfVal,"rx"))
            _pktDir = rx_only_direction;
        else if(!ccur_strcasecmp(_pMonIntf->tIntf.strIntfVal,"tx"))
            _pktDir = tx_only_direction;
        else if(!ccur_strcasecmp(_pMonIntf->tIntf.strIntfVal,"rxtx"))
            _pktDir = rx_and_tx_direction;
        else
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, unknown monitor direction value:%s",
                    _pMonIntf->tIntf.strIntfVal);
            break;
        }
       /* int pfring_set_direction(pfring *ring, packet_direction direction)
          Tell PF_RING to consider only those packets matching the specified
          direction. If the application does not call this function, all the
          packets (regardless of the direction, either RX or TX) are returned.
          typedef enum {
              rx_and_tx_direction = 0,
              rx_only_direction,
              tx_only_direction
            } packet_direction;*/
        _rc = pfring_set_direction(_pMonIntffltr->pRingMonIntf,_pktDir);
        if(0 != _rc)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, Rx Unable to set direction");
            break;
        }
        /*Set the poll timeout when passive wait is used. */
        _rc = pfring_set_poll_duration(_pMonIntffltr->pRingMonIntf,TRANSC_PKTPRC_POLLDURATION_MS);
        if(0 != _rc )
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, Rx Unable to set poll direction");
            break;
        }
        /* Whenever a user-space application has to wait until incoming packets arrive,
         it can instruct PF_RING not to return from poll() call unless at least â€œwatermarkâ€� packets have been returned.
         A low watermark value such as 1, reduces the latency of poll() but likely increases the number of poll() calls.
         A high watermark (it cannot exceed 50% of the ring size, otherwise the PF_RING kernel module will top its value)
         instead reduces the number of poll() calls but slightly increases the packet latency.
         The default value for the watermark (i.e. if user-space applications do not manipulate is value via this call) is 128*/
        _rc = pfring_set_poll_watermark(_pMonIntffltr->pRingMonIntf,TRANSC_PKTPRC_BUFFWTRMRK);
        if(0 != _rc )
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, Rx Unable to set poll watermark");
            break;
        }
        /* int pfring_set_socket_mode(pfring *ring, socket_mode mode)
           Tell PF_RING if the application needs to send and/or receive packets
           to/from the socket.
           typedef enum {
              send_and_recv_mode = 0,
              send_only_mode,
              recv_only_mode
            } socket*/
        _rc = pfring_set_socket_mode(_pMonIntffltr->pRingMonIntf,recv_only_mode);
        if(_rc != 0)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, Rx Unable to set socket mode");
            break;
        }
        /* Set filter for TCP port 80 */
        _rc = _tcPfrPcapSetRingHttpFilter(pCntx,_pMonIntffltr);
        if(ESUCCESS != _rc )
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,Rx unable to set http filter");
            break;
        }
        _pMonIntffltr->bBpfFilterSet = TRUE;
        if(pfring_enable_ring(_pMonIntffltr->pRingMonIntf))
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, Rx Unable to enable ring");
            break;
        }
        _result = ESUCCESS;
    }while(FALSE);

    if(ESUCCESS != _result && _pMonIntffltr)
        tcPfrPcapShutdownLibPfringRx(pCntx,_pMonIntffltr);

    return _result;
}

/***************************************************************************
* function: tcPfrPcapShutdownLibPfringMonIntf
*
* description: destroy Monitoring interface ring.
****************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPfrPcapShutdownLibPfringRx(
        tc_pktprc_thread_ctxt_t*    pCntx,
        tc_monintf_fltr_t*          pMonIntffltr)
{
    CCURASSERT(pCntx);
    CCURASSERT(pMonIntffltr);

    do
    {
        if(NULL == pMonIntffltr)
            break;
        if(pMonIntffltr->pRingMonIntf)
        {
            if(pMonIntffltr->bBpfFilterSet)
            {
                pfring_remove_bpf_filter(pMonIntffltr->pRingMonIntf);
                pMonIntffltr->bBpfFilterSet = FALSE;
            }
            pfring_close(pMonIntffltr->pRingMonIntf);
            pMonIntffltr->pRingMonIntf = NULL;
            /* Other Tx Cleanup here */
        }
    }while(FALSE);

    return ESUCCESS;
}

/***************************************************************************
 * function: tcPfrPcapGetStatPfringRx
 *
 * description: Get statistics from Pfring Monitoring interface Ring
 ***************************************************************************/
CCUR_PROTECTED(CHAR*)
tcPfrPcapGetStatPfringRx(
        tc_pktprc_thread_ctxt_t*     pCntx,
        CHAR*                        strBuff,
        U32                          nstrBuff)
{
    pfring_stat                 _stats;
    tresult_t                   _result;
    CHAR                        _strbuff[256];
    U16                         _nDroppedPkts;
    U16                         _nRecvedPkts;
    tc_monintf_mon_t*           _pMonIntf;
    tc_monintf_fltr_t*          _pMonIntffltr;
    U16                         _i;
    U16                         _j;

    strBuff[0] = '\0';
    _result    = ESUCCESS;
    for(_i=0;_i<pCntx->tIntfX.nMonIntfOpen;_i++)
    {
        _pMonIntf = &(pCntx->tIntfX.tMonIntfTbl[_i]);
        if(NULL == _pMonIntf)
        {
            _result = EFAILURE;
            break;
        }
        _nDroppedPkts = 0;
        _nRecvedPkts  = 0;
        for(_j=0;_j<_pMonIntf->nFilterActv;_j++)
        {
            _pMonIntffltr = &(pCntx->tIntfX.tMonIntfTbl[_i].tFilterTbl[_j]);
            if(NULL == _pMonIntffltr)
            {
                _result = EFAILURE;
                break;
            }
            if(_pMonIntffltr->pRingMonIntf)
            {
                pfring_stats(_pMonIntffltr->pRingMonIntf,&_stats);
                _nDroppedPkts += _stats.drop;
                _nRecvedPkts  += _stats.recv;
            }
            if(ESUCCESS != _result)
                break;
        }
        snprintf(_strbuff,
                sizeof(_strbuff),
                "       Ingress:%d\n"
                "       %s Rx Dropped:%lu\n"
                "       %s Rx Recv:%lu\n"
                "       ********************\n",
                _i,
                _pMonIntf->tIntf.strIntfName,_stats.drop,
                _pMonIntf->tIntf.strIntfName,_stats.recv);
        _strbuff[sizeof(_strbuff)-1] = '\0';
        ccur_strlcat(strBuff,_strbuff,nstrBuff);
    }

    return strBuff;
}

/***************************************************************************
 * function: _tcPfrPcapUpdateTime
 *
 * description: Update time intervals
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcPfrPcapUpdateTime(
        tc_pktprc_thread_ctxt_t*   pCntx,
        tc_gd_time_t*              pUpdTime,
        tc_gd_time_t*              pNowTime,
        tc_gd_time_t*              pOldTime)
{
    tUtilUTCTimeGet(pNowTime);
    if( pNowTime->nSeconds <= pOldTime->nSeconds)
        pUpdTime->nSeconds = 0;
    else
    {
        tUtilUTCTimeDiff(
                pUpdTime,
                pNowTime,
                pOldTime
                );
    }
    if(pUpdTime->nSeconds)
        *pOldTime = *pNowTime;
}

/***************************************************************************
 * function: _tcSimStatsDump
 *
 * description: dump statistics based on time specified.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcPfrPcapStatsDump(
        tc_pktprc_thread_ctxt_t*   pCntx,
        tc_gd_time_t*              pUpdDiff)
{
    pCntx->tStatsDumpCkTime.nSeconds += pUpdDiff->nSeconds;
    if( pCntx->tStatsDumpCkTime.nSeconds >=
            TRANSC_PKTPRC_DUMPSTATS_TIME_SEC )
    {
        tcPktPrcLogStats(pCntx);
        pCntx->tStatsDumpCkTime.nSeconds = 0;
    }
}

/***************************************************************************
 * function: _tcPfrPcapFlushQueues
 *
 * description: Flush freelist and values to mib thread
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcPfrPcapFlushQueueValues(
        tc_pktprc_thread_ctxt_t*   pCntx,
        tc_gd_time_t*              pUpdDiff)
{
    pCntx->tStatsFlushQueues.nSeconds += pUpdDiff->nSeconds;
    if( pCntx->tStatsFlushQueues.nSeconds >=
            TRANSC_PKTPRC_QUEUEFLUSH_TIME_SEC )
    {
        lkfqFlushFreeList(&(pCntx->pQPktProcToBkgrnd->tLkfq));
        lkfqFlushFreeList(pCntx->pQPktProcToMib);
        tcPktPrcFlushMibTable(pCntx);
        pCntx->tStatsFlushQueues.nSeconds = 0;
    }
}

/***************************************************************************
 * function: tcPfrPcapProcessPacket
 *
 * description: Process packets and distribute them to http processing
 * thread(s).
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPfrPcapProcessPacket(
        tc_pktprc_thread_ctxt_t*    pCntx,
        tc_monintf_map_t*           pMapIntf,
        U8*                         buffer,
        pfring_hdr_t*               hdr)
{
    CHAR                    _strDomainName[64];
    tc_g_qmsgpptohp_t*      _tPktDescMsg;
    U32                     _ipaddr;
    tc_g_svcdesc_t          _pSvcType;
    tresult_t               _result;

    _result = EIGNORE;
    _tPktDescMsg = NULL;
#if PFRING_DBG
     verboseProcessing(&hdr,buffer);
#endif
    /* This portion is split into two at IP parsing due to the fact that
     * pfring doesn not returns the correct header values for TCP IPv6
     * This might be because the test being done through replaying
     * traffic. In any case, TCP header is being parsed manually
     * for IPv6.
     */
    switch(hdr->extended_hdr.parsed_pkt.eth_type)
    {
        case TRANSC_IPV4:
            do
            {
                U8*             _pPyld;
                S32             _nPyldLen;
                _nPyldLen =
                    hdr->caplen-hdr->extended_hdr.parsed_pkt.offset.payload_offset;
                if(TRANSC_PROTO_TCP !=
                              hdr->extended_hdr.parsed_pkt.l3_proto)
                {
                    break;
                }
                /* Check for Minimum HTTP header payload size */
                if(_nPyldLen < (S32)TRANSC_PKTPRC_MINHTTPPYLD_SZ &&
                   _nPyldLen >= TRANSC_PKTPRC_MAXHTTPPYLD_SZ)
                {
                    break;
                }
                _tPktDescMsg = (tc_g_qmsgpptohp_t*)
                         lkfqMalloc(pCntx->pQPPktProcToHttpProc);
                if(NULL == _tPktDescMsg)
                {
                    _result = ENOMEM;
                    break;
                }
                _pPyld =
                    buffer+hdr->extended_hdr.parsed_pkt.offset.payload_offset;
                /* Get Source IP address for Mib */
                _ipaddr = ntohl(hdr->extended_hdr.parsed_pkt.ip_src.v4);
                memcpy((&_tPktDescMsg->pktDesc.ipHdr.tSrcIP.ip.v4),
                      &(_ipaddr), 4);
                _tPktDescMsg->pktDesc.ipHdr.tSrcIP.eType = tcIpaddrTypeIPv4;
                /* Write Info to Mib */
                tcPktPrcParseGetDomainName(
                        pCntx,_strDomainName,
                        sizeof(_strDomainName),
                        (CHAR*)_pPyld,
                        _nPyldLen);
                tcPktPrcWriteMibQTraffic(
                        pCntx,_strDomainName,
                        &(_tPktDescMsg->pktDesc.ipHdr.tSrcIP));
                if(FALSE == tcPktPrcIsRequestServiceAvail(
                                pCntx,
                                &_pSvcType,
                                (CHAR*)_pPyld,
                                _nPyldLen))
                {
                    break;
                }
                _tPktDescMsg->pktDesc.tcpHdr.pPyld      = _pPyld;
                _tPktDescMsg->pktDesc.tcpHdr.nPyldLen   = _nPyldLen;
                _tPktDescMsg->pktDesc.pMsgStrt          = buffer;
                _tPktDescMsg->pktDesc.nCaplen           = hdr->caplen;
                _tPktDescMsg->pktDesc.nWireLen          = hdr->len;
                /*if(hdr->extended_hdr.timestamp_ns)
                    _tPktDescMsg->pktDesc.ipHdr.tstampnsecs = hdr->extended_hdr.timestamp_ns;*/
                /* L2 */
                memcpy(_tPktDescMsg->pktDesc.l2Hdr.aDstMACAddress,
                       hdr->extended_hdr.parsed_pkt.dmac,
                       sizeof(_tPktDescMsg->pktDesc.l2Hdr.aDstMACAddress));
                memcpy(&(_tPktDescMsg->pktDesc.l2Hdr.aSrcMACAddress),
                       &(hdr->extended_hdr.parsed_pkt.smac),
                       sizeof(_tPktDescMsg->pktDesc.l2Hdr.aSrcMACAddress));
                /* L3 */
                _ipaddr = ntohl(hdr->extended_hdr.parsed_pkt.ip_dst.v4);
                memcpy((&_tPktDescMsg->pktDesc.ipHdr.tDstIP.ip.v4),
                      &(_ipaddr), 4);
                _tPktDescMsg->pktDesc.ipHdr.tDstIP.eType = tcIpaddrTypeIPv4;
                /* L4 */
                _tPktDescMsg->pktDesc.tcpHdr.nSrcPort =
                      hdr->extended_hdr.parsed_pkt.l4_src_port;
                _tPktDescMsg->pktDesc.tcpHdr.nDstPort =
                      hdr->extended_hdr.parsed_pkt.l4_dst_port;
                _tPktDescMsg->pktDesc.tcpHdr.nTcpAck =
                hdr->extended_hdr.parsed_pkt.tcp.ack_num;
                _tPktDescMsg->pktDesc.tcpHdr.nTcpSeq =
                hdr->extended_hdr.parsed_pkt.tcp.seq_num;
                /* Copy all the offsets, careful! according to pfring
                 * these offsets can be negative number especially
                 * eth_offset. */
                _tPktDescMsg->pktDesc.tOffsets.nEthOffset =
                        hdr->extended_hdr.parsed_pkt.offset.eth_offset;
                _tPktDescMsg->pktDesc.tOffsets.nL3Offset =
                        hdr->extended_hdr.parsed_pkt.offset.l3_offset;
                _tPktDescMsg->pktDesc.tOffsets.nL4Offset =
                        hdr->extended_hdr.parsed_pkt.offset.l4_offset;
                _tPktDescMsg->pktDesc.tOffsets.nPayloadOffset =
                        hdr->extended_hdr.parsed_pkt.offset.payload_offset;
                _tPktDescMsg->pktDesc.tOffsets.nVlanOffset =
                        hdr->extended_hdr.parsed_pkt.offset.vlan_offset;
                if(TRANSC_MPLS == TRANSC_PKTPRC_GET_ETHTYPE(buffer))
                    _tPktDescMsg->pktDesc.ethType = TRANSC_MPLS;
                else
                    _tPktDescMsg->pktDesc.ethType = TRANSC_IPV4;
#if TRANSC_STRMINJTIME
                /* grab time */
                _tPktDescMsg->pktDesc.tRxTime.reserved      = TRANSC_TIMEORIENT_UTC;
                _tPktDescMsg->pktDesc.tRxTime.nSeconds      = hdr->ts.tv_sec;
                _tPktDescMsg->pktDesc.tRxTime.nMicroseconds = hdr->ts.tv_usec;
#endif /* TRANSC_STRMTIME */
                if(ESUCCESS !=
                        tcPktPrcCkIpBlacklist(pCntx,&(_tPktDescMsg->pktDesc)))
                    break;
                _result =
                        tcPktPrcToHttpProcQueueMsg(
                                pCntx,pMapIntf,&_pSvcType,_tPktDescMsg);
            }while(FALSE);
            break;
        case TRANSC_IPV6:
            do
            {
                tc_phshdr_sz_t  _tPktDescSz;
                U8              _nNextHdrType;
                U16             _nIpPayldLen;
                U8              _nTcpHdrLen;
                _result = EIGNORE;
                /* pfring returns incorrect L4 values, must parse L4 manually */
                _nNextHdrType =
                        *(buffer+hdr->extended_hdr.parsed_pkt.offset.l3_offset + 6);
                if(TRANSC_PROTO_TCP != _nNextHdrType)
                {
                    break;
                }
                _tPktDescMsg = (tc_g_qmsgpptohp_t*)
                        lkfqMalloc(pCntx->pQPPktProcToHttpProc);
                if(NULL == _tPktDescMsg)
                {
                    _result = ENOMEM;
                    break;
                }
                _tPktDescMsg->pktDesc.pMsgStrt       = buffer;
                _tPktDescMsg->pktDesc.nCaplen        = hdr->caplen;
                _tPktDescMsg->pktDesc.nWireLen       = hdr->len;
                _tPktDescSz.nEthOffset   = 0;
                _tPktDescSz.nVlanOffset  = 0;
                _tPktDescSz.nL3Offset    = hdr->extended_hdr.parsed_pkt.offset.l3_offset;
                _tPktDescSz.nL4Offset    = hdr->extended_hdr.parsed_pkt.offset.l4_offset;
                _nIpPayldLen = ccur_nptrtohs( buffer+_tPktDescSz.nL3Offset + 4 );
                /* L4 */
                _result = tcPktParseTCP(&_nTcpHdrLen,
                              &(_tPktDescMsg->pktDesc),
                              &_tPktDescSz);
                /* Check for Minimum HTTP header payload size */
                if((ESUCCESS != _result) &&
                   (_tPktDescMsg->pktDesc.tOffsets.nL3Offset >= 0) &&
                   (_tPktDescMsg->pktDesc.tOffsets.nL4Offset >=0) &&
                    _tPktDescMsg->pktDesc.tcpHdr.nPyldLen < (S32)TRANSC_PKTPRC_MINHTTPPYLD_SZ &&
                   _tPktDescMsg->pktDesc.tcpHdr.nPyldLen >= TRANSC_PKTPRC_MAXHTTPPYLD_SZ)
                {
                    break;
                }
                /* Sanity test of TCP payload length,
                 * make sure payload length is the same as caplen calculation
                 * offset for tcp.
                 */
                /* Get Source IP address for Mib */
                memcpy((&_tPktDescMsg->pktDesc.ipHdr.tSrcIP.ip.v6),
                      &(hdr->extended_hdr.parsed_pkt.ip_src.v6), 16);
                _tPktDescMsg->pktDesc.ipHdr.tSrcIP.eType = tcIpaddrTypeIPv6;
                /* Write Info to Mib */
                tcPktPrcParseGetDomainName(
                        pCntx,_strDomainName,
                        sizeof(_strDomainName),
                        (CHAR*)_tPktDescMsg->pktDesc.tcpHdr.pPyld,
                        _tPktDescMsg->pktDesc.tcpHdr.nPyldLen);
                tcPktPrcWriteMibQTraffic(
                        pCntx,_strDomainName,
                        &(_tPktDescMsg->pktDesc.ipHdr.tSrcIP));
                if((_nIpPayldLen-_nTcpHdrLen !=
                        _tPktDescMsg->pktDesc.tcpHdr.nPyldLen) ||
                    FALSE == tcPktPrcIsRequestServiceAvail(
                                  pCntx,
                                  &_pSvcType,
                                  (CHAR*)_tPktDescMsg->pktDesc.tcpHdr.pPyld,
                                  _tPktDescMsg->pktDesc.tcpHdr.nPyldLen))
                {
                    break;
                }
                /*if(hdr->extended_hdr.timestamp_ns)
                  _tPktDescMsg->pktDesc.ipHdr.tstampnsecs = hdr->extended_hdr.timestamp_ns;*/
                /* L2 */
                memcpy(_tPktDescMsg->pktDesc.l2Hdr.aDstMACAddress,
                     hdr->extended_hdr.parsed_pkt.dmac,
                     sizeof(_tPktDescMsg->pktDesc.l2Hdr.aDstMACAddress));
                memcpy(&(_tPktDescMsg->pktDesc.l2Hdr.aSrcMACAddress),
                     &(hdr->extended_hdr.parsed_pkt.smac),
                     sizeof(_tPktDescMsg->pktDesc.l2Hdr.aSrcMACAddress));
                /* L3 */
                memcpy((&_tPktDescMsg->pktDesc.ipHdr.tDstIP.ip.v6),
                      &(hdr->extended_hdr.parsed_pkt.ip_dst.v6), 16);
                _tPktDescMsg->pktDesc.ipHdr.tDstIP.eType = tcIpaddrTypeIPv6;
                if(TRANSC_MPLS == TRANSC_PKTPRC_GET_ETHTYPE(buffer))
                    _tPktDescMsg->pktDesc.ethType = TRANSC_MPLS;
                else
                    _tPktDescMsg->pktDesc.ethType = TRANSC_IPV6;
#if TRANSC_STRMINJTIME
                /* grab time */
                _tPktDescMsg->pktDesc.tRxTime.reserved      = TRANSC_TIMEORIENT_UTC;
                _tPktDescMsg->pktDesc.tRxTime.nSeconds      = hdr.ts.tv_sec;
                _tPktDescMsg->pktDesc.tRxTime.nMicroseconds = hdr.ts.tv_usec;
#endif /* TRANSC_STRMTIME */
                if(ESUCCESS !=
                        tcPktPrcCkIpBlacklist(pCntx,&(_tPktDescMsg->pktDesc)))
                    break;
                _result =
                        tcPktPrcToHttpProcQueueMsg(
                                pCntx,pMapIntf,&_pSvcType,_tPktDescMsg);
            }while(FALSE);
            break;
        default:
            break;
    }
    if(ESUCCESS != _result)
    {
        if(_tPktDescMsg)
            lkfqFree(pCntx->pQPPktProcToHttpProc,
                    _tPktDescMsg);
    }

    return _result;
}

/***************************************************************************
 * function: tcPfrPcapProcessLibPfring
 *
 * description: Entry point of get packet from pfring.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPfrPcapProcessLibPfring(
        tc_pktprc_thread_ctxt_t* pCntx )
{
    pfring_hdr_t            _hdr;
    U8*                     _buffer;
    U16                     _nMapIntfIdx;
    U16                     _nIntffltrIdx;
    tc_gd_time_t            _tOldTime;
    tc_gd_time_t            _tNowTime;
    tc_gd_time_t            _tUpdDiff;
    tc_monintf_map_t*   _pMapIntf;
    U32                     _nCkTime=0;
    U32                     _nHunger = 0;
    tc_monintf_mon_t*      _pMonIntf;

    tUtilUTCTimeGet(&_tOldTime);
    _tNowTime = _tOldTime;
    ccur_memclear(&_tUpdDiff,sizeof(_tUpdDiff));
    ccur_memclear((void*)&(_hdr.extended_hdr.parsed_pkt),
            sizeof(struct pkt_parsing_info));
    while(!pCntx->bExit)
    {
        /* Read config to see if there are any changes.
         * This checking needs to be moved to periodic checking and
         * protected. */
        tcPktPrcReadCfg(pCntx,FALSE);
        /* TCS can be down due to network interface is not up yet
         * or configuration failure.
         */
        if(tcTrStsDown == pCntx->bTrSts)
            tcPktProcInitLogDownStatusAndRetry(pCntx);
        else
        {
            for(_nMapIntfIdx=0;_nMapIntfIdx<pCntx->tIntfX.nIntfMapTblTotal;_nMapIntfIdx++)
            {
                _pMonIntf = pCntx->tIntfX.tIntfMapTbl[_nMapIntfIdx].pMonIntf;
                for(_nIntffltrIdx=0;_nIntffltrIdx<
                        _pMonIntf->nFilterActv;_nIntffltrIdx++)
                {
                    if(pfring_is_pkt_available(
                            _pMonIntf->tFilterTbl[_nIntffltrIdx].pRingMonIntf))
                    {
                        _nHunger = 0;
                        _buffer = pCntx->PktBuffer;
                        _hdr.caplen = 0;
                        pCntx->nPcapPktTotal++;
                        if(pfring_recv(
                                _pMonIntf->tFilterTbl[_nIntffltrIdx].pRingMonIntf,
                                (u_char**)&_buffer,TRANSC_PKTPRC_BUFFCAPLEN,&_hdr,FALSE) > 0)
                        {
                            if(_hdr.caplen >= TRANSC_PKTPRC_HTTP_MINPYLD_SZ)
                            {
                                _pMapIntf = &(pCntx->tIntfX.tIntfMapTbl[_nMapIntfIdx]);
                                if(ESUCCESS !=
                                        tcPfrPcapProcessPacket(pCntx,_pMapIntf,_buffer,&_hdr))
                                {
                                    pCntx->nPcapPktIgnored++;
                                }
                            }
                            else
                                pCntx->nPcapParseErr++;
                        }
                        else
                            pCntx->nPcapParseErr++;
                    }
                    else
                        _nHunger++;
                    _nCkTime++;
                    if(_nCkTime >=
                            TRANSC_PKTPRC_TCPSTRM_TTL_CNT)
                    {
                        _tcPfrPcapUpdateTime(pCntx,&_tUpdDiff,&_tNowTime,&_tOldTime);
                        _tcPfrPcapStatsDump(pCntx,&_tUpdDiff);
                        _tcPfrPcapFlushQueueValues(pCntx,&_tUpdDiff);
                        _nCkTime = 0;
                    }
                    _nCkTime++;
                }
            }
            /*bundle polling of all monitoring interfaces. TCS can use
             * pfring bundle socket in the future. */
            if(_nHunger > TRANSC_PKTPRC_HUNGER_CNT)
            {
                for(_nMapIntfIdx=0;_nMapIntfIdx<pCntx->tIntfX.nIntfMapTblTotal;_nMapIntfIdx++)
                {
                    _pMonIntf = pCntx->tIntfX.tIntfMapTbl[_nMapIntfIdx].pMonIntf;
                    for(_nIntffltrIdx=0;_nIntffltrIdx<_pMonIntf->nFilterActv;_nIntffltrIdx++)
                    {
                        pfring_sync_indexes_with_kernel(_pMonIntf->tFilterTbl[_nIntffltrIdx].pRingMonIntf);
                        _pMonIntf->tMonIntfPollFdTbl[_nIntffltrIdx].events  = POLLIN;
                        _pMonIntf->tMonIntfPollFdTbl[_nIntffltrIdx].revents = 0;
                    }
                    poll(_pMonIntf->tMonIntfPollFdTbl,
                         _pMonIntf->nFilterActv,
                         TRANSC_PKTPRC_POLLDURATION_MS);
                }
                _nHunger = 0;
            }
        }
    }

    return ESUCCESS;
}
#endif

