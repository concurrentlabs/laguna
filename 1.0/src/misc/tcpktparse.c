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

#include "tcpktparse.h"
#include "tcutil.h"

/* IP Layer Macro */
#define TRANSC_PRS_L3IPV4               0x0800
#define TRANSC_PRS_L3IPV6               0x86DD

/**************** PRIVATE Functions **********************/

/* Assume eth offset is zero */
CCUR_PRIVATE(tresult_t)
_tcPktParseL2(tc_pktdesc_t * pPktDesc,
        tc_phshdr_sz_t * pPktDescSz)
{
    U16                 _nEthType;
    U8*                 _pPyld;
    U8*                 _pnTmp;
    U16                 _i;
    tresult_t           _result;

    CCURASSERT(pPktDesc);

    do
    {
        _result = ESUCCESS;
        if(pPktDesc->nCaplen < 14)
        {
            _result = EFAILURE;
            break;
        }
        _pPyld =
                pPktDesc->pMsgStrt;
        _pnTmp = pPktDesc->l2Hdr.aDstMACAddress;
        for( _i = 0; _i < 3; ++_i )
        {
            _nEthType = ccur_nptrtohs( _pPyld + (2 * _i) );
            *_pnTmp++ = (_nEthType & 0xFF00) >> 8;
            *_pnTmp++ = _nEthType & 0x00FF;
        }
        _pPyld += 6;
        _pnTmp = pPktDesc->l2Hdr.aSrcMACAddress;
        for( _i = 0; _i < 3; ++_i )
        {
            _nEthType = ccur_nptrtohs( _pPyld + (2 * _i) );
            *_pnTmp++ = (_nEthType & 0xFF00) >> 8;
            *_pnTmp++ = _nEthType & 0x00FF;
        }
        _pPyld += 6;
        /* Skip VLAN (if available)*/
        pPktDescSz->nL3Offset = 12;
        _nEthType = ccur_nptrtohs( _pPyld );
        if(TRANSC_IPV4 == _nEthType ||
           TRANSC_IPV6 == _nEthType)
        {
            pPktDescSz->nL3Offset   +=2;
            pPktDescSz->nVlanOffset  =0;
            pPktDesc->ipHdr.nEthType = _nEthType;
            break;
        }
        else
        {
            pPktDescSz->nL3Offset +=2;
            pPktDescSz->nVlanOffset =
                    pPktDescSz->nL3Offset;
            _result = EFAILURE;
            /* skip VLAN tags */
            if( _nEthType == 0x8100 &&
                _nEthType == 0x00 )
            {
                /* VLAN tag */
                if(pPktDescSz->nL3Offset+4
                        >= pPktDesc->nCaplen)
                    break;
                else
                {
                    _pPyld +=4;
                    _nEthType = ccur_nptrtohs( _pPyld );
                    pPktDescSz->nL3Offset +=4;
                    if( _nEthType == 0x8800 &&
                        _nEthType == 0xa800 )
                    {
                        /* stacked VLAN tag */
                        if(pPktDescSz->nL3Offset+4
                                >= pPktDesc->nCaplen)
                            break;
                        else
                        {
                            _pPyld +=4;
                            _nEthType = ccur_nptrtohs( _pPyld );
                            pPktDescSz->nL3Offset +=4;
                            _result = EIGNORE;
                        }
                    }
                }
            }
            if(TRANSC_IPV4 == _nEthType)
            {
                if(pPktDescSz->nL3Offset+2
                        >= pPktDesc->nCaplen)
                {
                    _result = EFAILURE;
                    break;
                }
                else
                {
                    pPktDesc->ipHdr.nEthType = _nEthType;
                    pPktDescSz->nL3Offset +=2;
                }
            }
            else
                _result = EIGNORE;
            pPktDesc->ipHdr.nEthType = _nEthType;
        }
    }while(FALSE);

    return _result;
}

CCUR_PRIVATE(void)
_tcPktParseIPv4DataPopulate(tc_pktdesc_t * pPktDesc,
        tc_phshdr_sz_t* pPktDescSz)
{
    U8*     _p;

    CCURASSERT(pPktDesc);

    _p = pPktDesc->pMsgStrt +
            pPktDescSz->nL3Offset;
    _p += 12;
    pPktDesc->ipHdr.tSrcIP.eType = tcIpaddrTypeIPv4;
    memcpy((&pPktDesc->ipHdr.tSrcIP.ip.v4),_p, 4);
    _p += 4;
    pPktDesc->ipHdr.tDstIP.eType = tcIpaddrTypeIPv4;
    memcpy(&(pPktDesc->ipHdr.tDstIP.ip.v4),_p, 4);
}

CCUR_PRIVATE(tresult_t)
_tcPktParseIPv4(tc_pktdesc_t * pPktDesc,
        tc_phshdr_sz_t * pPktDescSz)
{
    U8          _nHdrLenBytes;
    U16         _nTotLenBytes;
    U8*         _pIpHdr;
    U32         _nCapturePktLen;
    tresult_t   _result;

    CCURASSERT(pPktDesc);

    do
    {
        _result = EFAILURE;
        _nCapturePktLen =
                pPktDesc->nCaplen-
                pPktDescSz->nL3Offset;
        _pIpHdr =
                pPktDesc->pMsgStrt +
                pPktDescSz->nL3Offset;
        /*
         * Ensure that enough bytes were captured to parse the header
         */
        if (_nCapturePktLen < 20)
            break;
        /*
        * Verify that the header length is sane.  Abort if it is too short.
        */
       _nHdrLenBytes = (*_pIpHdr & 0x0F) * 4;
       if ((_nHdrLenBytes < 20) ||
               (_nCapturePktLen < _nHdrLenBytes))
           break;
       /*
        * Verify that the total length is sane
        */
       _nTotLenBytes =
               ((*(_pIpHdr + 2)) << 8) | *(_pIpHdr + 3);
       if ((_nTotLenBytes < _nHdrLenBytes) )
           break;
       if(pPktDesc->nWireLen < _nTotLenBytes)
           break;
       pPktDescSz->nL4Offset    = _nHdrLenBytes +
                                            pPktDescSz->nL3Offset;
       //pPktDesc->ipHdr.protocol = *(_pIpHdr+9);
       _tcPktParseIPv4DataPopulate( pPktDesc,pPktDescSz );
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

#if 0
CCUR_PRIVATE(void)
_tcPktParseIPv6DataPopulate(tc_pktdesc_t * pPktDesc)
{
    U8*     _p;
    _p = pl3sz->pPyld + 8;
    pl3sz->tSrcIP.eType = tcIpaddrTypeIPv6;
    memcpy( &(pl3sz->tSrcIP.ip.v6), _p, 16 );
    _p = pl3sz->pPyld + 24;
    pl3sz->tDstIP.eType = tcIpaddrTypeIPv6;
    memcpy( &(pl3sz->tDstIP.ip.v6), _p, 16 );
}
#endif

/* UNTESTED!!!! and
 * support for ipv6 extension header */
CCUR_PRIVATE(tresult_t)
_tcPktParseIPv6(
        tc_pktdesc_t*               pPktDesc,
        tc_phshdr_sz_t*             pPktDescSz)
{
#if 1
    tresult_t        _result;
    _result = EFAILURE;
#else
    U8*              _pIpHdr;
    /* Default length of v6 header. Can we
        really assume this is true? */
    U8               _nHdrLenBytes;
    U32              _nCapturePktLen;
    U16              _nTotLenBytes;
    tresult_t        _result;

    /* TODO: Parse ipv6 extension header.
     * below are temporary solution... */
    /* ... */


    do
    {
        _result = EFAILURE;
        _nCapturePktLen =
                pPktDesc->nCaplen-
                pPktDesc->l2Hdr.nLen;
        _pIpHdr = pl3sz->pPyld;
        _nHdrLenBytes = (*_pIpHdr & 0x0F) * 4;
        if ((_nHdrLenBytes < 20) ||
                (_nCapturePktLen < _nHdrLenBytes))
        {
            break;
        }
        /*
         * Verify that the total length is sane
         */
        _nTotLenBytes =
                ((*(_pIpHdr + 2)) << 8) | *(_pIpHdr + 3);
        if ((_nTotLenBytes < _nHdrLenBytes) )
        {
            break;
        }
        if(_nCapturePktLen < _nTotLenBytes)
        {
            break;
        }
        else if(_nCapturePktLen > _nTotLenBytes)
        {
            pPktDesc->nCaplen = _nCapturePktLen;
        }
        pl3sz->nLen        = _nHdrLenBytes;
        pl3sz->protocol    = *(_pIpHdr+9);
        pl3sz->nTotLen     = _nTotLenBytes;
        _tcPktParseIPv6DataPopulate( pPktDesc );
        _result = ESUCCESS;
    }while(FALSE);
#endif
    return _result;
}

CCUR_PRIVATE(tresult_t)
_tcPktParseL3(tc_pktdesc_t * pPktDesc,
        tc_phshdr_sz_t * pPktDescSz)
{
    tresult_t           _result;
    U16                 _L3Protocol;

    CCURASSERT(pPktDesc);

    /* Not IP protocol, we don't care */
    _L3Protocol =
            (*(pPktDesc->pMsgStrt +
                    pPktDescSz->nL3Offset) >> 4);
    /*
     * Diverge based on IPv4/IPv6.
     */
    switch(_L3Protocol)
    {
        case tcIpaddrTypeIPv4:
            _result = _tcPktParseIPv4(pPktDesc,pPktDescSz);
            break;
        case tcIpaddrTypeIPv6:
            _result = _tcPktParseIPv6(pPktDesc,pPktDescSz);
            break;
        default:
            _result  = EIGNORE;
            break;
    }

    return _result;
}

CCUR_PRIVATE(tresult_t)
_tcPktParseL4(tc_pktdesc_t * pPktDesc, tc_phshdr_sz_t * pPktDescSz)
{
    tresult_t       _result;
    U8              _L4Protocol;
    U8              _nTcpHdrLen;

    CCURASSERT(pPktDesc);

    _L4Protocol =
            (*(pPktDesc->pMsgStrt +
            pPktDescSz->nL3Offset+9));
    if( TRANSC_PROTO_TCP ==
            _L4Protocol)
    {
        _result = tcPktParseTCP(
                &_nTcpHdrLen,
                pPktDesc,
                pPktDescSz);
    }
    else
        _result = EIGNORE;

    return _result;
}

CCUR_PRIVATE(void)
_tcPktGetSymHash(tc_pktdesc_t * pPktDesc)
{
    pPktDesc->pktHash    = 0;
}

/**************** PROTECTED Functions **********************/
CCUR_PROTECTED(tresult_t)
tcPktParseTCP(U8 * pTcpHdrLen,
        tc_pktdesc_t * pPktDesc, tc_phshdr_sz_t * pPktDescSz)
{
    U8*         _pTcpHdr;
    U8          _nHdrLenBytes;
    U32         _nCapturedPktLen;
    tresult_t   _result;

    CCURASSERT(pPktDesc);

    do
    {
        _result = EFAILURE;
        _nCapturedPktLen =
                pPktDesc->nCaplen-
                pPktDescSz->nL4Offset;
        /* TCP start Header pointer  */
        _pTcpHdr =
                pPktDesc->pMsgStrt+
                pPktDescSz->nL4Offset;
        memcpy( &_nHdrLenBytes,
                _pTcpHdr + 12, sizeof( U8 ) );
       _nHdrLenBytes =
                (_nHdrLenBytes & 0xF0) >> 2;
        if(_nCapturedPktLen < _nHdrLenBytes)
            break;
        *pTcpHdrLen = _nHdrLenBytes;
        /* TCP start payload pointer  */
        pPktDesc->tcpHdr.pPyld =
                _pTcpHdr + _nHdrLenBytes;
        pPktDescSz->nPayloadOffset =
                pPktDescSz->nL4Offset+
                _nHdrLenBytes;
        /* TCP Payload length */
        pPktDesc->tcpHdr.nPyldLen =
                pPktDesc->nCaplen -
                pPktDescSz->nPayloadOffset;
        if(pPktDesc->tcpHdr.nPyldLen < 0 )
            break;
        /* Populate packet descriptor with header data.  */
        pPktDesc->tcpHdr.nSrcPort = ccur_nptrtohs( _pTcpHdr );
        pPktDesc->tcpHdr.nDstPort = ccur_nptrtohs( _pTcpHdr + 2 );
        pPktDesc->tcpHdr.nTcpFlags = *(_pTcpHdr + 13);
        /*pPktDesc->tcpHdr.nTcpWindow = ccur_nptrtohs( _pTcpHdr + 14 );*/
        pPktDesc->tcpHdr.nTcpSeq = ccur_nptrtohl( _pTcpHdr + 4 );
        if( pPktDesc->tcpHdr.nTcpFlags & TCTCPFLG_ACK )
            pPktDesc->tcpHdr.nTcpAck = ccur_nptrtohl( _pTcpHdr + 8 );
        /*if( pPktDesc->tcpHdr.nTcpFlags & TCTCPFLG_URG )
            pPktDesc->tcpHdr.nTcpUrgPtr = ccur_nptrtohl( _pTcpHdr + 18 );*/
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}


CCUR_PROTECTED(tresult_t)
tcPktParse(tc_pktdesc_t * pPktDesc)
{
    tresult_t           _result;
    tc_phshdr_sz_t      _tPktDescSz;

    CCURASSERT(pPktDesc);
    /* Parse L2, L3 and L4 */
    do
    {
        _tPktDescSz.nEthOffset  = 0;
        _result = _tcPktParseL2(
                pPktDesc,&_tPktDescSz);
        if(ESUCCESS != _result)
        {
            break;
        }
        _result = _tcPktParseL3(
                pPktDesc,&_tPktDescSz);
        if(ESUCCESS != _result)
        {
            break;
        }
        _result = _tcPktParseL4(
                pPktDesc,&_tPktDescSz);
        if(ESUCCESS != _result)
        {
            break;
        }
        /* Calculate stream symmetric hash */
        _tcPktGetSymHash(pPktDesc);
    }while(FALSE);

    return _result;
}
