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
/*
CCUR_PRIVATE(CHAR*)
_tcPktInjGetStrDateTime(
        CHAR* pTbuf,
        U16   nTBuf)
{
    time_t      _tnow;
    struct tm*  _tm;

    _tnow = time(0);
    _tm = gmtime(&_tnow);
    strftime(pTbuf, nTBuf, "%a, %d %b %Y %H:%M:%S %Z", _tm);

    return pTbuf;
}
*/
#if 0
CCUR_PRIVATE(U32)
_tcPktInjGetTime( tc_pktgen_t *pPktGen )
{
    struct timeval uts;
    U32            t;

    CCURASSERT(pPktGen);

    gettimeofday( &uts, NULL );
    t = uts.tv_sec;
    t *= 1000;
    t += uts.tv_usec / 1000;

    return( t );
}
#endif

CCUR_PRIVATE(void)
_tcPktInjLogPktInject(
        tc_pktgen_thread_ctxt_t*            pCntx,
        tc_iphdr_ipddrtype_e                eType,
        tc_pktgen_tcpflgproto_e             protocol,
        U32                                 nInjectCntr,
        tc_pktgen_t*                        pPktGen,
        tc_iphdr_ipaddr_t*                  pLogSrcIP,
        tc_iphdr_ipaddr_t*                  pLogDstIP)
{

    CHAR                        _srcaddr[64];
    CHAR                        _dstaddr[64];

    do
    {
        if (TRANSC_ZLOGCATEGORY_BITMAP_CK(pCntx->tLogDescSvc,evLogLvlDebug))
            return;
        if(tcIpaddrTypeIPv4 == eType)
        {
            inet_ntop(AF_INET,&(pLogSrcIP->ip.v4),_srcaddr,sizeof(_srcaddr));
            inet_ntop(AF_INET,&(pLogDstIP->ip.v4),_dstaddr,sizeof(_dstaddr));
        }
        else
        {
            inet_ntop(AF_INET6,&(pLogSrcIP->ip.v6),_srcaddr,sizeof(_srcaddr));
            inet_ntop(AF_INET6,&(pLogDstIP->ip.v6),_dstaddr,sizeof(_dstaddr));
        }
        if(pPktGen->tPktInfo.TcpPayldLen)
        {
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlDebug,
                    &(pCntx->tLogDescSvc),
                    "Pkt Injections:%lu\n"
                    "Length payload: %lu\n"
                    "src MAC/Ip/Port: %02x:%02x:%02x:%02x:%02x:%02x/%s/%lu\n"
                    "dest MAC/Ip/Port: %02x:%02x:%02x:%02x:%02x:%02x/%s/%lu\n"
                    "Http Payload:\n%s",
                    nInjectCntr,
                    pPktGen->tPktInfo.TcpPayldLen,
                    pPktGen->tPktInfo.tFwd.mac[0],pPktGen->tPktInfo.tFwd.mac[1],
                    pPktGen->tPktInfo.tFwd.mac[2],pPktGen->tPktInfo.tFwd.mac[3],
                    pPktGen->tPktInfo.tFwd.mac[4],pPktGen->tPktInfo.tFwd.mac[5],
                    _srcaddr,
                    pPktGen->tPktInfo.tFwd.nPort,
                    pPktGen->tPktInfo.tRev.mac[0],pPktGen->tPktInfo.tRev.mac[1],
                    pPktGen->tPktInfo.tRev.mac[2],pPktGen->tPktInfo.tRev.mac[3],
                    pPktGen->tPktInfo.tRev.mac[4],pPktGen->tPktInfo.tRev.mac[5],
                    _dstaddr,
                    pPktGen->tPktInfo.tRev.nPort,
                    pPktGen->tPktInfo.TcpPayldBuff);
        }
        else
        {
            switch(protocol)
            {
                case tcTcpFlagProtoFin:
                    evLogTrace(
                            pCntx->pQPktGenToBkgrnd,
                            evLogLvlDebug,
                            &(pCntx->tLogDescSvc),
                            "Pkt Injections:%lu\n"
                            "Length payload: %lu\n"
                            "src MAC/Ip/Port: %02x:%02x:%02x:%02x:%02x:%02x/%s/%lu\n"
                            "dest MAC/Ip/Port: %02x:%02x:%02x:%02x:%02x:%02x/%s/%lu\n"
                            "Http Payload:\n%s",
                            nInjectCntr,
                            pPktGen->tPktInfo.TcpPayldLen,
                            pPktGen->tPktInfo.tFwd.mac[0],pPktGen->tPktInfo.tFwd.mac[1],
                            pPktGen->tPktInfo.tFwd.mac[2],pPktGen->tPktInfo.tFwd.mac[3],
                            pPktGen->tPktInfo.tFwd.mac[4],pPktGen->tPktInfo.tFwd.mac[5],
                            _srcaddr,
                            pPktGen->tPktInfo.tFwd.nPort,
                            pPktGen->tPktInfo.tRev.mac[0],pPktGen->tPktInfo.tRev.mac[1],
                            pPktGen->tPktInfo.tRev.mac[2],pPktGen->tPktInfo.tRev.mac[3],
                            pPktGen->tPktInfo.tRev.mac[4],pPktGen->tPktInfo.tRev.mac[5],
                            _dstaddr,
                            pPktGen->tPktInfo.tRev.nPort,
                            "None(TCP FIN)\n\n");
                    break;
                case tcTcpFlagProtoRst:
                    evLogTrace(
                            pCntx->pQPktGenToBkgrnd,
                            evLogLvlDebug,
                            &(pCntx->tLogDescSvc),
                            "Pkt Injections:%lu\n"
                            "Length payload: %lu\n"
                            "src MAC/Ip/Port: %02x:%02x:%02x:%02x:%02x:%02x/%s/%lu\n"
                            "dest MAC/Ip/Port: %02x:%02x:%02x:%02x:%02x:%02x/%s/%lu\n"
                            "Http Payload:\n%s",
                            nInjectCntr,
                            pPktGen->tPktInfo.TcpPayldLen,
                            pPktGen->tPktInfo.tFwd.mac[0],pPktGen->tPktInfo.tFwd.mac[1],
                            pPktGen->tPktInfo.tFwd.mac[2],pPktGen->tPktInfo.tFwd.mac[3],
                            pPktGen->tPktInfo.tFwd.mac[4],pPktGen->tPktInfo.tFwd.mac[5],
                            _srcaddr,
                            pPktGen->tPktInfo.tFwd.nPort,
                            pPktGen->tPktInfo.tRev.mac[0],pPktGen->tPktInfo.tRev.mac[1],
                            pPktGen->tPktInfo.tRev.mac[2],pPktGen->tPktInfo.tRev.mac[3],
                            pPktGen->tPktInfo.tRev.mac[4],pPktGen->tPktInfo.tRev.mac[5],
                            _dstaddr,
                            pPktGen->tPktInfo.tRev.nPort,
                            "None(TCP RST)\n\n");
                    break;
                default:
                    break;
            }
        }
    }while(FALSE);
}


/***************************************************************************
 * _tcPktInjWriteIPv4Buff
 *
 * description: Write packet into a buffer for Ipv4.
 * note: No support for fragmented packet creation.
 ***************************************************************************/
CCUR_PRIVATE(U32)
_tcPktInjWriteIPv4Buff(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_pktgen_pktinj_t*         pPktInj,
        tc_pktgen_pktinfo_t*        pPktInfo)
{
    S32                 _i, _cv;
    S32                 _pktlen;
    S32                 _nIpPktSize;
    U32                 _csum;
    U32                 _nseq, _nack;
    U8                  *_pWr,  *_u, *_c, *_cs;
    U8                  *_pPyld, *_pktend;
    U8                  *_pPyldEnd;
    S16                 _mplstags;
    S32                 _nPktCnt = 0;

    CCURASSERT(pCntx);
    CCURASSERT(pPktInj);
    CCURASSERT(pPktInfo);

    do
    {
        /* No fragmentation, only 1 pkt being sent at a time */
        _nseq           = pPktInfo->tFwd.tcp_seq;
        _nack           = pPktInfo->tFwd.tcp_ack;
        /**********************************/
        /* Calculate Payload length     */
        /**********************************/
        if(pPktInfo->bInsertMplsTag &&
           TRANSC_MPLS == pPktInfo->ethType)
        {
            if(pPktInfo->tPktPcap.nEthOffset < 0 ||
               pPktInfo->tPktPcap.nL3Offset < 0)
            {
                break;
            }
            /* Pkt length =
             * Mac addr + MPLS tags + ipheader + tcp header + tcp payload */
            _mplstags = pPktInfo->tPktPcap.nL3Offset-
                     (pPktInfo->tPktPcap.nEthOffset+TRANSC_PKTGEN_MACADDRHDR_SZ);
            if(_mplstags < 0)
                break;
            _pktlen =
                    TRANSC_PKTGEN_MACADDRHDR_SZ + _mplstags +
                    TRANSC_PKTGEN_IPV4HDR_SZ + TRANSC_PKTGEN_TCPHDR_SZ + pPktInfo->TcpPayldLen;
        }
        else
        {
            /* Pkt length =
             * Mac addr + ipheader + tcp header + tcp payload */
            _pktlen =
                    TRANSC_PKTGEN_MACADDRHDR_SZ + TRANSC_PKTGEN_IPV4HDR_SZ +
                    TRANSC_PKTGEN_TCPHDR_SZ + pPktInfo->TcpPayldLen;
        }
        /* Check if packet needs to be fragmented, currently unsupported */
        if((_pktlen + 4) > pPktInfo->nEthFramesize)
            break;
        if((_pktlen + 4) > (S32)sizeof(pPktInj->pkt))
            break;
        pPktInj->pktlen = _pktlen;
        _pWr            = pPktInj->pkt;
        _pktend         = (_pWr+_pktlen);

        /*
         *  Write all the values
         */
        /******************************/
        /* L2 - Destination MAC      */
        /******************************/
        /* Destination MAC */
        for( _i=0; _i<6; _i++ )
            *_pWr++ = pPktInfo->tRev.mac[_i];

        /* Source MAC */
        for( _i=0; _i<6; _i++ )
            *_pWr++ = pPktInfo->tFwd.mac[_i];

        if(pPktInfo->bInsertMplsTag &&
           TRANSC_MPLS == pPktInfo->ethType)
        {
            /* Ethertype mpls */
            *_pWr++ = 0x88;
            *_pWr++ = 0x47;
            /* Copy mpls tags */
            _pPyld          = pPktInfo->tPktPcap.pMsgStrt+
                              pPktInfo->tPktPcap.nEthOffset+TRANSC_PKTGEN_MACADDRHDR_SZ;
            _pPyldEnd       = pPktInfo->tPktPcap.pMsgStrt+
                              pPktInfo->tPktPcap.nL3Offset;
            do
            {
                *_pWr++ = *_pPyld++;
            } while( (_pPyld < _pPyldEnd) && (_pWr < _pktend) );
        }
        else
        {
            /* Ethertype ipv4 */
            *_pWr++ = 0x08;
            *_pWr++ = 0x00;
        }
        /******************************/
        /* L3 - IPv4 header           */
        /******************************/
        _c = _pWr;
        *_pWr++ = (0x05 | 0x40); /* IPv4 with header length of 5 words */
        *_pWr++ = 0x00; /* dscp + ecn */
        _nIpPktSize = _pktlen - TRANSC_PKTGEN_MACADDRHDR_SZ;
        *_pWr++ = (U8) (_nIpPktSize >> 8);
        *_pWr++ = (U8) _nIpPktSize; /* payload size */
        *_pWr++ = 0x00; /* identification 1 */
        *_pWr++ = 0x00; /* identification 2 */
        /* last packet or fragmented */
        *_pWr++ = 0x40;
        /**_pWr++ = 0x20;*/ /* fragmented flag set */
        *_pWr++ = 0x00;
        *_pWr++ = 0x80; /* TTL */
        *_pWr++ = (U8) TRANSC_PKTGEN_TCPPROTO_BYTE;
        _cs = _pWr;
        *_pWr++ = 0x00; /* header checksum 1 */
        *_pWr++ = 0x00; /* header checksum 2 */
        _u = _pWr;
        *_pWr++ = (U8)(pPktInfo->tFwd.ipv4 >> 24);
        *_pWr++ = (U8)(pPktInfo->tFwd.ipv4 >> 16);
        *_pWr++ = (U8)(pPktInfo->tFwd.ipv4 >> 8);
        *_pWr++ = (U8)pPktInfo->tFwd.ipv4;
        *_pWr++ = (U8)(pPktInfo->tRev.ipv4 >> 24);
        *_pWr++ = (U8)(pPktInfo->tRev.ipv4 >> 16);
        *_pWr++ = (U8)(pPktInfo->tRev.ipv4 >> 8);
        *_pWr++ = (U8)pPktInfo->tRev.ipv4;

        /* calculate IP header checksum */
        _csum = 0;
        while( _c < _pWr )
        {
            _cv = (U32) *_c++;
            _cv = (_cv << 8) + (U32) (*_c++ & 0xFF);
            _csum += _cv;
        }
        _csum = (_csum & 0xFFFF) + (_csum >> 16);
        _csum = _csum ^ 0xFFFF;
        *_cs++ = (U8) (_csum >> 8 );
        *_cs = (U8) _csum;

        /* calculate part of transport checksum */
        _csum = (U32) TRANSC_PKTGEN_TCPPROTO_BYTE;
        for( _i=0; _i<4; _i++ )
        {
            _cv = (U32) *_u++;
            _cv = (_cv << 8) + (U32) (*_u++ & 0xFF);
            _csum += _cv;
        }
        /* All below are generic and can be made into
         * a function. */
        /*************************************/
        /* L4 - Transport Layer (TCP) header */
        /*************************************/
        _u = _pWr;
        *_pWr++ = (U8) (pPktInfo->tFwd.nPort >> 8);
        *_pWr++ = (U8) pPktInfo->tFwd.nPort;
        *_pWr++ = (U8) (pPktInfo->tRev.nPort >> 8);
        *_pWr++ = (U8) pPktInfo->tRev.nPort;
        /* Seq Num field */
        *_pWr++ = (U8) (_nseq >> 24);
        *_pWr++ = (U8) (_nseq >> 16);
        *_pWr++ = (U8) (_nseq >> 8);
        *_pWr++ = (U8) _nseq;

        *_pWr++ = (U8) (_nack >> 24); /* ack field */
        *_pWr++ = (U8) (_nack >> 16);
        *_pWr++ = (U8) (_nack >> 8);
        *_pWr++ = (U8) _nack;

        *_pWr++ = (U8) 0x50; /*data offset */
        *_pWr = (U8) 0x00; /* flags */
        switch(pPktInfo->eFlagProto)
        {
            case tcTcpFlagProtoFin:
                *_pWr = (U8) (TCTCPFLG_FIN); /* flags */
                break;
            case tcTcpFlagProtoFinAck:
                *_pWr = (U8) (TCTCPFLG_FIN | TCTCPFLG_ACK); /* flags */
                break;
            case tcTcpFlagProtoRst:
                *_pWr = (U8) (TCTCPFLG_RST); /* flags */
                break;
            case tcTcpFlagProtoRstAck:
                *_pWr = (U8) (TCTCPFLG_RST | TCTCPFLG_ACK); /* flags */
                break;
            case tcTcpFlagProto:
                *_pWr = (U8) (TCTCPFLG_PSH | TCTCPFLG_ACK); /* flags */
                break;
            default:
                *_pWr = (U8) (TCTCPFLG_ACK); /* flags */
                break;
        }
        _pWr++;

        *_pWr++ = (U8) 0x2a; /* window size */
        *_pWr++ = (U8) 0x00;
        _cs = _pWr;
        *_pWr++ = (U8) 0x00; /*_csum*/
        *_pWr++ = (U8) 0x00;

        *_pWr++ = (U8) 0x00; /*urgent ptr*/
        *_pWr++ = (U8) 0x00;
        /**************************************/
        /* L4 - Transport Layer (TCP) Payload */
        /**************************************/
        _pPyld          = pPktInfo->TcpPayldBuff;
        _pPyldEnd       = pPktInfo->TcpPayldBuff + pPktInfo->TcpPayldLen;
        if( (pPktInfo->eFlagProto == tcTcpFlagProto) && (_pPyld < _pPyldEnd))
        {
            do
            {
                *_pWr++ = *_pPyld++;
            } while( (_pPyld < _pPyldEnd) && (_pWr < _pktend) );
        }
        pPktInfo->tFwd.tcp_seq = _nseq;
        _csum += (U32)(_pWr-_u); /* add length */
        while( _u < _pWr )
        {
            _cv = (U32) *_u++;
            if( _u >= _pWr )
            {
                _cv = (_cv << 8);
            }
            else
            {
                _cv = (_cv << 8) + (U32) (*_u++ & 0xFF);
            }
            _csum += _cv;
        }
        _csum = (_csum & 0xFFFF) + (_csum >> 16);
        _csum = _csum ^ 0xFFFF;
        *_cs++ = (U8) (_csum >> 8 );
        *_cs = (U8) _csum;
        _nPktCnt++;
    } while( FALSE );

    return(_nPktCnt);
}


/***************************************************************************
 * _tcPktInjWriteIPv6Buff
 *
 * description: Write packet into a buffer for Ipv6.
 * note: No support for fragmented packet creation.
 ***************************************************************************/
CCUR_PRIVATE(U32)
_tcPktInjWriteIPv6Buff(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_pktgen_pktinj_t*         pPktInj,
        tc_pktgen_pktinfo_t*        pPktInfo)
{
    S32                 _i, _cv;
    S32                 _pktlen;
    S32                 _nIpPktSize;
    U32                 _csum;
    U32                 _nseq, _nack;
    U8                  *_pWr,  *_u, *_cs;
    U8                  *_pPyld, *_pktend;
    U8                  *_pPyldEnd;
    S16                 _mplstags;
    S32                 _nPktCnt = 0;

    CCURASSERT(pCntx);
    CCURASSERT(pPktInj);
    CCURASSERT(pPktInfo);

    do
    {
        /* No fragmentation, only 1 pkt being sent at a time */
        _nseq           = pPktInfo->tFwd.tcp_seq;
        _nack           = pPktInfo->tFwd.tcp_ack;
        /**********************************/
        /* Calculate Payload length     */
        /**********************************/
        if(pPktInfo->bInsertMplsTag &&
           TRANSC_MPLS == pPktInfo->ethType)
        {
            if(pPktInfo->tPktPcap.nEthOffset < 0 ||
               pPktInfo->tPktPcap.nL3Offset < 0)
            {
                break;
            }
            /* Pkt length =
             * Mac addr + MPLS tags +  ipheader + tcp header + tcp payload */
            _mplstags = pPktInfo->tPktPcap.nL3Offset-
                     (pPktInfo->tPktPcap.nEthOffset+TRANSC_PKTGEN_MACADDRHDR_SZ);
            if(_mplstags < 0)
                break;
            _pktlen =
                    TRANSC_PKTGEN_MACADDRHDR_SZ + _mplstags +
                    TRANSC_PKTGEN_IPV6HDR_SZ + TRANSC_PKTGEN_TCPHDR_SZ + pPktInfo->TcpPayldLen;
        }
        else
        {
            /* Pkt length =
             * Mac addr + ipheader + tcp header + tcp payload */
            _pktlen =
                    TRANSC_PKTGEN_MACADDRHDR_SZ + TRANSC_PKTGEN_IPV6HDR_SZ +
                    TRANSC_PKTGEN_TCPHDR_SZ + pPktInfo->TcpPayldLen;
        }
        /* Check if packet needs to be fragmented, currently unsupported */
        if((_pktlen + 4) > pPktInfo->nEthFramesize)
            break;
        if((_pktlen + 4) > (S32)sizeof(pPktInj->pkt))
            break;
        pPktInj->pktlen = _pktlen;
        _pWr            = pPktInj->pkt;
        _pktend         = (_pWr+_pktlen);

        /*
         *  Write all the values
         */
        /******************************/
        /* L2 - Destination MAC      */
        /******************************/
        for( _i=0; _i<6; _i++ )
            *_pWr++ = pPktInfo->tRev.mac[_i];
        /* Source MAC */
        for( _i=0; _i<6; _i++ )
            *_pWr++ = pPktInfo->tFwd.mac[_i];

        if(pPktInfo->bInsertMplsTag &&
           TRANSC_MPLS == pPktInfo->ethType)
        {
            /* Ethertype mpls */
            *_pWr++ = 0x88;
            *_pWr++ = 0x47;
            /* Copy mpls tags */
            _pPyld          = pPktInfo->tPktPcap.pMsgStrt+
                              pPktInfo->tPktPcap.nEthOffset+TRANSC_PKTGEN_MACADDRHDR_SZ;
            _pPyldEnd       = pPktInfo->tPktPcap.pMsgStrt+
                              pPktInfo->tPktPcap.nL3Offset;
            do
            {
                *_pWr++ = *_pPyld++;
            } while( (_pPyld < _pPyldEnd) && (_pWr < _pktend) );
        }
        else
        {
            /* Ethertype ipv6 */
            *_pWr++ = 0x86;
            *_pWr++ = 0xDD;
        }
        /******************************/
        /* L3 - IPv6 header           */
        /******************************/
        *_pWr++ = 0x60; /* V6 and 4 0's for DSCP */
        *_pWr++ = 0x04; /* 4 bits of DSCP and 4 bits of flow identifier */
        *_pWr++ = 0x00; /* flow identifier */
        *_pWr++ = 0x00; /* flow identifier */
        _nIpPktSize = TRANSC_PKTGEN_TCPHDR_SZ + pPktInfo->TcpPayldLen;
        *_pWr++ = (U8) (_nIpPktSize >> 8);
        *_pWr++ = (U8) _nIpPktSize; /* payload size */
        /* if fragmented then 44 else protocol */
        *_pWr++ = (U8) TRANSC_PKTGEN_TCPPROTO_BYTE;
        *_pWr++ = 0x50; /* hop limit/ ttl */
        _u = _pWr;
        for( _i=0; _i<8; _i++ )
        {
            *_pWr++ = (U8)(pPktInfo->tFwd.ipv6[_i] >> 8);
            *_pWr++ = (U8)pPktInfo->tFwd.ipv6[_i];
        }
        for( _i=0; _i<8; _i++ )
        {
            *_pWr++ = (U8)(pPktInfo->tRev.ipv6[_i] >> 8);
            *_pWr++ = (U8)pPktInfo->tRev.ipv6[_i];
        }
#if 0
        _i = 16;
        _csum = (U32) PKTGEN_TCPPROTO_BYTE;
        do
        {
            _cv = (U32) *_u++;
            _cv = (_cv << 8) + (U32) (*_u++ & 0xFF);
            _csum += _cv;
            _i = _i-2;
        } while( _i > 0 );
#endif
        /* All below are generic and can be made into
         * a function. */
        /*************************************/
        /* L4 - Transport Layer (TCP) header */
        /*************************************/
        /* calculate part of transport checksum */
        _csum = (U32) TRANSC_PKTGEN_TCPPROTO_BYTE;
        for( _i=0; _i<4; _i++ )
        {
            _cv = (U32) *_u++;
            _cv = (_cv << 8) + (U32) (*_u++ & 0xFF);
            _csum += _cv;
        }
        /* create tcp header */
        _u = _pWr;
        *_pWr++ = (U8) (pPktInfo->tFwd.nPort >> 8);
        *_pWr++ = (U8) pPktInfo->tFwd.nPort;
        *_pWr++ = (U8) (pPktInfo->tRev.nPort >> 8);
        *_pWr++ = (U8) pPktInfo->tRev.nPort;
        /* Seq Num field */
        *_pWr++ = (U8) (_nseq >> 24);
        *_pWr++ = (U8) (_nseq >> 16);
        *_pWr++ = (U8) (_nseq >> 8);
        *_pWr++ = (U8) _nseq;

        *_pWr++ = (U8) (_nack >> 24); /* ack field */
        *_pWr++ = (U8) (_nack >> 16);
        *_pWr++ = (U8) (_nack >> 8);
        *_pWr++ = (U8) _nack;

        *_pWr++ = (U8) 0x50; /*data offset */
        *_pWr = (U8) 0x00; /* flags */
        switch(pPktInfo->eFlagProto)
        {
            case tcTcpFlagProtoFin:
                *_pWr = (U8) (TCTCPFLG_FIN); /* flags */
                break;
            case tcTcpFlagProtoFinAck:
                *_pWr = (U8) (TCTCPFLG_FIN | TCTCPFLG_ACK); /* flags */
                break;
            case tcTcpFlagProtoRst:
                *_pWr = (U8) (TCTCPFLG_RST); /* flags */
                break;
            case tcTcpFlagProtoRstAck:
                *_pWr = (U8) (TCTCPFLG_RST | TCTCPFLG_ACK); /* flags */
                break;
            case tcTcpFlagProto:
                *_pWr = (U8) (TCTCPFLG_PSH | TCTCPFLG_ACK); /* flags */
                break;
            default:
                *_pWr = (U8) (TCTCPFLG_ACK); /* flags */
                break;
        }
        _pWr++;

        *_pWr++ = (U8) 0x2a; /* window size */
        *_pWr++ = (U8) 0x00;
        _cs = _pWr;
        *_pWr++ = (U8) 0x00; /*_csum*/
        *_pWr++ = (U8) 0x00;

        *_pWr++ = (U8) 0x00; /*urgent ptr*/
        *_pWr++ = (U8) 0x00;
        /**************************************/
        /* L4 - Transport Layer (TCP) Payload */
        /**************************************/
        _pPyld          = pPktInfo->TcpPayldBuff;
        _pPyldEnd       = pPktInfo->TcpPayldBuff + pPktInfo->TcpPayldLen;
        if( (pPktInfo->eFlagProto == tcTcpFlagProto) && (_pPyld < _pPyldEnd))
        {
            do
            {
                *_pWr++ = *_pPyld++;
            } while( (_pPyld < _pPyldEnd) && (_pWr < _pktend) );
        }
        pPktInfo->tFwd.tcp_seq = _nseq;
        _csum += (U32)(_pWr-_u); /* add length */
        while( _u < _pWr )
        {
            _cv = (U32) *_u++;
            if( _u >= _pWr )
            {
                _cv = (_cv << 8);
            }
            else
            {
                _cv = (_cv << 8) + (U32) (*_u++ & 0xFF);
            }
            _csum += _cv;
        }
        _csum = (_csum & 0xFFFF) + (_csum >> 16);
        _csum = _csum ^ 0xFFFF;
        *_cs++ = (U8) (_csum >> 8 );
        *_cs = (U8) _csum;
        _nPktCnt++;
    } while( FALSE );

    return(_nPktCnt);
}

CCUR_PRIVATE(tresult_t)
_tcPktInjWrite(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_outintf_out_t*          pOutIntf,
        tc_pktgen_t*                pPktGen,
        tc_iphdr_ipddrtype_e        eIpType,
        U32                         nTime)
{
    tresult_t   _result;
    I32         _nPktsSent;

    CCURASSERT(pCntx);
    CCURASSERT(pPktGen);

    do
    {
        _result = ESUCCESS;
        if(tcIpaddrTypeIPv4 == eIpType)
        {
            pPktGen->npackets =
                    _tcPktInjWriteIPv4Buff(
                        pCntx,
                        &(pPktGen->tPktInj),
                        &(pPktGen->tPktInfo));
        }
        else
        {
            pPktGen->npackets =
                    _tcPktInjWriteIPv6Buff(
                        pCntx,
                        &(pPktGen->tPktInj),
                        &(pPktGen->tPktInfo));
        }
        /* only 1 pkt being injected at a time because
         * we currently not supporting fragmented packets */
        if(1 != pPktGen->npackets)
        {
            _result = EFAILURE;
            break;
        }
        _nPktsSent = tcPktIOTxOutIntfToWire(
                pCntx,pOutIntf,&(pPktGen->tPktInj) );
        if (_nPktsSent < 0)
            _result = EFAILURE;
        else
        {
            _result = ESUCCESS;
            pPktGen->nRawPacketsSent    += _nPktsSent;
            pPktGen->nRawBytesSent      += pPktGen->tPktInj.pktlen;
        }
    }while(FALSE);

    return( _result );
}


/***************************************************************************
 * tcPktInjInjectReqFinPkt
 *
 * description: construct and inject FIN/RST packet into the network
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktInjInjectReqFinPkt(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_pktdesc_t*               pPktDesc,
        hlpr_httpsite_hndl          eSite,
        tc_pktgen_tcpflgproto_e     eFlagProto,
        tc_outintf_out_t*          pOutIntf)
{
    tresult_t           _result;
    /*U32                 _nTime;*/
    S32                 _i;
    tc_pktgen_ept_t*    _pSrcEp;
    tc_pktgen_ept_t*    _pDstEp;
    tc_pktgen_t         _tPktGen;
    tc_iphdr_ipaddr_t*  _tLogSrcIP;
    tc_iphdr_ipaddr_t*  _tLogDstIP;
    tc_pktgen_tcpflgproto_e _ePrntProto;
    tc_iphdr_ipddrtype_e    _eIpType;

    CCURASSERT(pCntx);
    CCURASSERT(pPktDesc);

    do
    {
        _result = ESUCCESS;
        if(eSite < 0 && eSite >= TRANSC_LDCFG_SITE_MAXSITES_LST)
        {
            _result = EINVAL;
            break;
        }
        _ePrntProto = 0;
        /*_nTime = _tcPktInjGetTime( &_tPktGen );*/
        _pSrcEp = &(_tPktGen.tPktInfo.tFwd);
        _pDstEp = &(_tPktGen.tPktInfo.tRev);
        _tPktGen.nRawBytesSent   = 0;
        _tPktGen.nRawPacketsSent = 0;
        _tPktGen.tPktInfo.TcpPayldBuff[0] = '\0';
        /*
         *
         * TCP FIN
         */
        /* Send to Server */
        for( _i=0; _i<6; _i++ )
            _pSrcEp->mac[_i] = (U8)pOutIntf->pCmdArgSmacAddr[_i];
        if(pOutIntf->bIsRouter)
        {
            for( _i=0; _i<6; _i++ )
                _pDstEp->mac[_i] = (U8)pOutIntf->pCmdArgDmacAddr[_i];
        }
        else
        {
            for( _i=0; _i<6; _i++ )
                _pDstEp->mac[_i] = pPktDesc->l2Hdr.aDstMACAddress[_i];
        }
        /* TODO: need to just pass the pktdesc octet without copying.
         * This component was written separately in the past.
         */
        /* Change to network byte order and copy to pktgen*/
        /* Dst */
        if(tcIpaddrTypeIPv4 ==
                pPktDesc->ipHdr.tDstIP.eType)
        {
            _eIpType = tcIpaddrTypeIPv4;
            _pDstEp->ipv4 =
                    (pPktDesc->ipHdr.tDstIP.ip.v4.octet[0] << 24) |
                    (pPktDesc->ipHdr.tDstIP.ip.v4.octet[1] << 16) |
                    (pPktDesc->ipHdr.tDstIP.ip.v4.octet[2] << 8) |
                    (pPktDesc->ipHdr.tDstIP.ip.v4.octet[3]);
        }
        else
        {
            _eIpType = tcIpaddrTypeIPv6;
            for(_i=0;_i<8;_i++)
            {
                _pDstEp->ipv6[_i] =
                        (pPktDesc->ipHdr.tDstIP.ip.v6.octet[(_i<<1)] << 8) |
                        (pPktDesc->ipHdr.tDstIP.ip.v6.octet[(_i<<1)+1]);
            }
        }
        /* Src */
        if(tcIpaddrTypeIPv4 ==
                pPktDesc->ipHdr.tSrcIP.eType)
        {
            _eIpType = tcIpaddrTypeIPv4;
            _pSrcEp->ipv4 =
                    (pPktDesc->ipHdr.tSrcIP.ip.v4.octet[0] << 24) |
                    (pPktDesc->ipHdr.tSrcIP.ip.v4.octet[1] << 16) |
                    (pPktDesc->ipHdr.tSrcIP.ip.v4.octet[2] << 8) |
                    (pPktDesc->ipHdr.tSrcIP.ip.v4.octet[3]);
        }
        else
        {
            _eIpType = tcIpaddrTypeIPv6;
            for(_i=0;_i<8;_i++)
            {
                _pSrcEp->ipv6[_i] =
                        (pPktDesc->ipHdr.tSrcIP.ip.v6.octet[(_i<<1)] << 8) |
                        (pPktDesc->ipHdr.tSrcIP.ip.v6.octet[(_i<<1)+1]);
            }
        }
        _pSrcEp->nPort = pPktDesc->tcpHdr.nSrcPort;
        _pDstEp->nPort = pPktDesc->tcpHdr.nDstPort;
        if((tcTcpFlagProtoRstAck == eFlagProto) ||
           (tcTcpFlagProtoFinAck == eFlagProto))
        {
            _pSrcEp->tcp_seq = pPktDesc->tcpHdr.nTcpSeq+
                               pPktDesc->tcpHdr.nPyldLen;
            _pSrcEp->tcp_ack = pPktDesc->tcpHdr.nTcpAck;
            if(tcTcpFlagProtoRstAck == eFlagProto)
                _ePrntProto = tcTcpFlagProtoRst;
            else
                _ePrntProto = tcTcpFlagProtoFin;
        }
        else
        {
            _pSrcEp->tcp_seq = pPktDesc->tcpHdr.nTcpSeq+
                               pPktDesc->tcpHdr.nPyldLen;
            _pSrcEp->tcp_ack = 0;
        }
        _pDstEp->tcp_seq = 0;
        _pDstEp->tcp_ack = 0;
        _tPktGen.tPktInfo.TcpPayldLen               = 0;
        _tPktGen.tPktInfo.bInsertMplsTag            = pCntx->bOutIntfInsertMplsTag;
        _tPktGen.tPktInfo.nEthFramesize             = PKTGEN_OUTPUT_FRAMESIZE;
        _tPktGen.tPktInfo.eFlagProto                = eFlagProto;
        _tPktGen.tPktInfo.ethType                   = pPktDesc->ethType;
        /* copy all pcap offsets and pkt orig */
        _tPktGen.tPktInfo.tPktPcap.pMsgStrt         = pPktDesc->pMsgStrt;
        _tPktGen.tPktInfo.tPktPcap.nCaplen          = pPktDesc->nCaplen;
        _tPktGen.tPktInfo.tPktPcap.nEthOffset       = pPktDesc->tOffsets.nEthOffset;
        _tPktGen.tPktInfo.tPktPcap.nL3Offset        = pPktDesc->tOffsets.nL3Offset;
        _tPktGen.tPktInfo.tPktPcap.nL4Offset        = pPktDesc->tOffsets.nL4Offset;
        _tPktGen.tPktInfo.tPktPcap.nPayloadOffset   = pPktDesc->tOffsets.nPayloadOffset;
        _tPktGen.tPktInfo.tPktPcap.nVlanOffset      = pPktDesc->tOffsets.nVlanOffset;
        _result = _tcPktInjWrite(pCntx,pOutIntf,&_tPktGen,_eIpType,0);
        if(ESUCCESS == _result)
        {
            _tLogSrcIP    = &(pPktDesc->ipHdr.tSrcIP);
            _tLogDstIP    = &(pPktDesc->ipHdr.tDstIP);
            (pCntx->nInjectCntr)++;
            _tcPktInjLogPktInject(
                    pCntx,
                    pPktDesc->ipHdr.tDstIP.eType,
                    _ePrntProto,
                    (pCntx->nInjectCntr),
                    &(_tPktGen),
                    _tLogSrcIP,
                    _tLogDstIP);
        }
    }while(FALSE);

    return (_result);
}

/***************************************************************************
 * tcPktInjInjectReq302Pkt
 *
 * description: construct and inject HTTP 302 packet into the network
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPktInjInjectReq302Pkt(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_g_qmsghptopg_t*          pHttpMsg,
        tc_pktdesc_t*               pPktDesc,
        hlpr_httpsite_hndl          eSite,
        tc_outintf_out_t*           pOutIntf,
        tc_outintf_mon_t*           pMonIntf)
{
    tresult_t           _result;
    /*U32                 _nTime;*/
    S32                 _i;
    tc_pktgen_ept_t*    _pSrcEp;
    tc_pktgen_ept_t*    _pDstEp;
    tc_pktgen_t         _tPktGen;
    tc_iphdr_ipaddr_t*  _tLogSrcIP;
    tc_iphdr_ipaddr_t*  _tLogDstIP;
    tc_iphdr_ipddrtype_e _eIpType;
    CHAR*                _pPUrl;
    S32                  _nPUrlLen;
    U16                  _nSkipLen;
    CHAR                 _strBuff[64];
    /*CHAR                _TBuff[128];*/

    CCURASSERT(pCntx);
    CCURASSERT(pPktDesc);

    do
    {
        _result = ESUCCESS;
        /*_tcPktInjGetStrDateTime(_TBuff,sizeof(_TBuff));*/
        /*_nTime = _tcPktInjGetTime( &_tPktGen );*/
        _pSrcEp = &(_tPktGen.tPktInfo.tFwd);
        _pDstEp = &(_tPktGen.tPktInfo.tRev);
        _tPktGen.nRawBytesSent   = 0;
        _tPktGen.nRawPacketsSent = 0;
        _tPktGen.tPktInfo.TcpPayldBuff[0] = '\0';

        /* Send to client */
        for( _i=0; _i<6; _i++ )
            _pSrcEp->mac[_i] = (U8)pOutIntf->pCmdArgSmacAddr[_i];
        if(pOutIntf->bIsRouter)
        {
            for( _i=0; _i<6; _i++ )
                _pDstEp->mac[_i] = (U8)pOutIntf->pCmdArgDmacAddr[_i];
        }
        else
        {
            for( _i=0; _i<6; _i++ )
                _pDstEp->mac[_i] = pPktDesc->l2Hdr.aSrcMACAddress[_i];
        }

        /* TODO: need to just pass the pktdesc octet without copying.
         * This component was written separately in the past.
         */
        /* Change to network byte order and copy to pktgen */
        /* Src */
        if(tcIpaddrTypeIPv4 ==
                pPktDesc->ipHdr.tDstIP.eType)
        {
            _eIpType = tcIpaddrTypeIPv4;
            _pSrcEp->ipv4 =
                    (pPktDesc->ipHdr.tDstIP.ip.v4.octet[0] << 24) |
                    (pPktDesc->ipHdr.tDstIP.ip.v4.octet[1] << 16) |
                    (pPktDesc->ipHdr.tDstIP.ip.v4.octet[2] << 8) |
                    (pPktDesc->ipHdr.tDstIP.ip.v4.octet[3]);
        }
        else
        {
            _eIpType = tcIpaddrTypeIPv6;
            for(_i=0;_i<8;_i++)
            {
                _pSrcEp->ipv6[_i] =
                        (pPktDesc->ipHdr.tDstIP.ip.v6.octet[(_i<<1)] << 8) |
                        (pPktDesc->ipHdr.tDstIP.ip.v6.octet[(_i<<1)+1]);
            }
        }
        /* Dst */
        if(tcIpaddrTypeIPv4 ==
                pPktDesc->ipHdr.tSrcIP.eType)
        {
            _eIpType = tcIpaddrTypeIPv4;
            _pDstEp->ipv4 =
                    (pPktDesc->ipHdr.tSrcIP.ip.v4.octet[0] << 24) |
                    (pPktDesc->ipHdr.tSrcIP.ip.v4.octet[1] << 16) |
                    (pPktDesc->ipHdr.tSrcIP.ip.v4.octet[2] << 8) |
                    (pPktDesc->ipHdr.tSrcIP.ip.v4.octet[3]);
        }
        else
        {
            _eIpType = tcIpaddrTypeIPv6;
            for(_i=0;_i<8;_i++)
            {
                _pDstEp->ipv6[_i] =
                        (pPktDesc->ipHdr.tSrcIP.ip.v6.octet[(_i<<1)] << 8) |
                        (pPktDesc->ipHdr.tSrcIP.ip.v6.octet[(_i<<1)+1]);
            }
        }
        _pSrcEp->nPort = pPktDesc->tcpHdr.nDstPort;
        _pDstEp->nPort = pPktDesc->tcpHdr.nSrcPort;
        _pSrcEp->tcp_seq = pPktDesc->tcpHdr.nTcpAck;
        _pSrcEp->tcp_ack = pPktDesc->tcpHdr.nTcpSeq+
                           pPktDesc->tcpHdr.nPyldLen;
        _pDstEp->tcp_seq = 0;
        _pDstEp->tcp_ack = 0;
        if('\0' == pHttpMsg->strCRange[0])
        {
            pHttpMsg->strCRange[0] = '0';
            pHttpMsg->strCRange[1] = '\0';
        }
        if('\0' == pHttpMsg->strCMisc[0])
        {
            pHttpMsg->strCMisc[0] = '0';
            pHttpMsg->strCMisc[1] = '\0';
        }
        /* Adds "http:// if not specified */
        if(strncmp(pMonIntf->strRedirAddr,"http://",sizeof("http://")-1))
            snprintf(_strBuff,sizeof(_strBuff),"http://%s",pMonIntf->strRedirAddr);
        else
            ccur_strlcpy(_strBuff,pMonIntf->strRedirAddr,sizeof(_strBuff));
        /* construct "Location:" message.*/
        _tPktGen.tPktInfo.TcpPayldLen = snprintf(
                                (CHAR*)_tPktGen.tPktInfo.TcpPayldBuff,
                                sizeof(_tPktGen.tPktInfo.TcpPayldBuff),
                                "HTTP/1.1 302 Found\r\n"
                                "Location: %s"TRANSC_CCUR_REDIR_PREPEND"/%s/tcshost/%s/tcskey/%s/%s_%s/tcsopt/%s/tcsosig/%s/",
                                _strBuff,
                                pHttpMsg->strSvcName,
                                pHttpMsg->strHostName,
                                pHttpMsg->strCId,
                                pHttpMsg->strCRange,
                                pHttpMsg->strCMisc,
                                pHttpMsg->strOptions,
                                pHttpMsg->strSvcName);
        _pPUrl    = NULL;
        _nPUrlLen = 0;
        if(TCUTIL_IS_URLABSPATH(pHttpMsg->pUrl))
        {
            _pPUrl      = pHttpMsg->pUrl;
            _nPUrlLen   = pHttpMsg->nUrlLen;
        }
        else
        {
            /* Skips http://.../ if exists in case of web proxy */
            if(TCUTIL_IS_URLHTTPSTRING(pHttpMsg->pUrl))
            {
                _nSkipLen = tcUtilSkipGetHttpStringLen(
                            pHttpMsg->pUrl,pHttpMsg->nUrlLen);
                if(_nSkipLen)
                {
                    _pPUrl     = pHttpMsg->pUrl+_nSkipLen;
                    _nPUrlLen  = pHttpMsg->nUrlLen-_nSkipLen;

                }
            }
        }
        /* Check to see if enough space for url length */
        if((NULL == _pPUrl) ||
           _tPktGen.tPktInfo.TcpPayldLen+_nPUrlLen >=
           sizeof(_tPktGen.tPktInfo.TcpPayldBuff))
        {
            _result = EINVAL;
            break;
        }
        strncpy((CHAR*)_tPktGen.tPktInfo.TcpPayldBuff+
                _tPktGen.tPktInfo.TcpPayldLen,_pPUrl,_nPUrlLen);
        _tPktGen.tPktInfo.TcpPayldLen += _nPUrlLen;
        /* Construct other parameter fields */
        if('\0' != pHttpMsg->strCOrigin[0])
        {
            _tPktGen.tPktInfo.TcpPayldLen += snprintf(
                                    (CHAR*)_tPktGen.tPktInfo.TcpPayldBuff+_tPktGen.tPktInfo.TcpPayldLen,
                                    (sizeof(_tPktGen.tPktInfo.TcpPayldBuff)-(_tPktGen.tPktInfo.TcpPayldLen)),
                                    "\r\n"
                                    "Accept-Ranges: bytes\r\n"
                                    "Content-Type: application/octet-stream\r\n"
                                    "Access-Control-Allow-Credentials: true\r\n"
                                    "Access-Control-Allow-Origin: %s\r\n"
                                    "Access-Control-Allow-Methods: GET\r\n"
                                    "Content-Length: 0\r\n",
                                    pHttpMsg->strCOrigin);
        }
        else
        {
            _tPktGen.tPktInfo.TcpPayldLen += snprintf(
                                    (CHAR*)_tPktGen.tPktInfo.TcpPayldBuff+_tPktGen.tPktInfo.TcpPayldLen,
                                    (sizeof(_tPktGen.tPktInfo.TcpPayldBuff)-(_tPktGen.tPktInfo.TcpPayldLen)),
                                    "\r\n"
                                    "Accept-Ranges: bytes\r\n"
                                    "Content-Type: text/html; charset=UTF-8\r\n"
                                    "Content-Length: 0\r\n");
        }
        /* Add last terminating characters */
        if(_tPktGen.tPktInfo.TcpPayldLen+sizeof("\r\n\0") >=
                sizeof(_tPktGen.tPktInfo.TcpPayldBuff))
        {
            _result = EINVAL;
            break;
        }
        strncpy((CHAR*)_tPktGen.tPktInfo.TcpPayldBuff+
                _tPktGen.tPktInfo.TcpPayldLen,"\r\n",sizeof("\r\n"));
        _tPktGen.tPktInfo.TcpPayldLen += sizeof("\r\n")-1;
        _tPktGen.tPktInfo.TcpPayldBuff[_tPktGen.tPktInfo.TcpPayldLen] = '\0';
        _tPktGen.tPktInfo.TcpPayldBuff[sizeof(_tPktGen.tPktInfo.TcpPayldBuff)-1] = '\0';
        _tPktGen.tPktInfo.bInsertMplsTag            = pCntx->bOutIntfInsertMplsTag;
        _tPktGen.tPktInfo.nEthFramesize             = PKTGEN_OUTPUT_FRAMESIZE;
        _tPktGen.tPktInfo.eFlagProto                = tcTcpFlagProto;
        _tPktGen.tPktInfo.ethType                   = pPktDesc->ethType;
        /* copy all pcap offsets and pkt orig */
        _tPktGen.tPktInfo.tPktPcap.pMsgStrt         = pPktDesc->pMsgStrt;
        _tPktGen.tPktInfo.tPktPcap.nCaplen          = pPktDesc->nCaplen;
        _tPktGen.tPktInfo.tPktPcap.nEthOffset       = pPktDesc->tOffsets.nEthOffset;
        _tPktGen.tPktInfo.tPktPcap.nL3Offset        = pPktDesc->tOffsets.nL3Offset;
        _tPktGen.tPktInfo.tPktPcap.nL4Offset        = pPktDesc->tOffsets.nL4Offset;
        _tPktGen.tPktInfo.tPktPcap.nPayloadOffset   = pPktDesc->tOffsets.nPayloadOffset;
        _tPktGen.tPktInfo.tPktPcap.nVlanOffset      = pPktDesc->tOffsets.nVlanOffset;
        /* construct entire packet */
        _result = _tcPktInjWrite(pCntx,pOutIntf,&_tPktGen,_eIpType,0);
        if(ESUCCESS == _result)
        {
            _tLogSrcIP    = &(pPktDesc->ipHdr.tDstIP);
            _tLogDstIP    = &(pPktDesc->ipHdr.tSrcIP);
            (pCntx->nInjectCntr)++;
            _tcPktInjLogPktInject(
                    pCntx,
                    pPktDesc->ipHdr.tDstIP.eType,
                    tcTcpFlagProto,
                    (pCntx->nInjectCntr),
                    &(_tPktGen),
                    _tLogSrcIP,
                    _tLogDstIP);
        }
    }while(FALSE);

    return (_result);
}
