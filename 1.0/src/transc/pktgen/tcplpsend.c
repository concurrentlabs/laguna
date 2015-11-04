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

#ifdef  TRANSC_BUILDCFG_LIBPCAP

CCUR_PROTECTED(BOOL)
tcPlPsendInitIsTxIntfLinkUp(
        tc_pktgen_thread_ctxt_t * pCntx)
{
    BOOL    _bIntfUp = TRUE;

    CCURASSERT(pCntx);

    return _bIntfUp;
}

CCUR_PRIVATE(S32)
_tcPlPsendOpenRawSocket(
        tc_pktgen_thread_ctxt_t*  pCntx,
        S32                       interface)
{
    struct sockaddr_ll  _skt;
    S32                 _size;
    S32                 _fd;

    CCURASSERT(pCntx);

    do
    {
        _fd = EFAILURE;
        pCntx->g_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (pCntx->g_fd == -1)
            break;
        ccur_memclear(&_skt, sizeof(struct sockaddr_ll));
        _skt.sll_family = AF_PACKET;
        _skt.sll_protocol = htons(ETH_P_ALL);
        _skt.sll_ifindex = interface;
        if (bind(pCntx->g_fd, (struct sockaddr *)&_skt, sizeof(struct sockaddr_ll)))
            break;
        _size = TRANSC_PKTGEN_NETSOCKOPTSIZE;
        if (setsockopt(pCntx->g_fd, SOL_SOCKET, SO_SNDBUF, &_size,
                       sizeof(_size)) == -1)
            break;
        _fd = pCntx->g_fd;
    }while(FALSE);

    return _fd;
}

CCUR_PROTECTED(tresult_t)
tcPlPsendInitLibPcapTx(
        tc_pktgen_thread_ctxt_t * pCntx)
{
    tresult_t   _result;
    S32         _ifIdx;

    _ifIdx = if_nametoindex(pCntx->strPktPrcArgOutIntf);
    if (0 == _ifIdx)
        _result = EFAILURE;
    else
    {
        pCntx->g_fd = _tcPlPsendOpenRawSocket(pCntx, _ifIdx);
        _result = ESUCCESS;
    }

    return(_result);
}

CCUR_PROTECTED(tresult_t)
tcPlPsendShutdownLibPcapTx(
        tc_pktgen_thread_ctxt_t * pCntx)
{
    tresult_t   _result;

    CCURASSERT(pCntx);

    do
    {
        _result = EFAILURE;
        if(EFAILURE == pCntx->g_fd)
            break;
        _result = close(pCntx->g_fd);
    }while(FALSE);

    return(_result);
}

CCUR_PROTECTED(I32)
tcPlPsendLibPcapTx(
        tc_pktgen_thread_ctxt_t*    pCntx,
        tc_pktgen_t*                pPktGen,
        tc_pktgen_pktinj_t*         pkt)
{
    I32         _nPktsSent;

    CCURASSERT(pPktGen);
    CCURASSERT(pkt);

    pCntx->g_iovec.iov_base              = pkt->pkt;
    pCntx->g_iovec.iov_len               = pkt->pktlen;
    pCntx->g_datagrams.msg_iov           = &(pCntx->g_iovec);
    pCntx->g_datagrams.msg_iovlen        = 1;
    _nPktsSent                  =
            (I32)(sendmsg(pCntx->g_fd, &(pCntx->g_datagrams), 0));

    return(_nPktsSent);
}


CCUR_PROTECTED(CHAR*)
tcPlPsendGetStatLibPcapTx(
        tc_pktgen_thread_ctxt_t*     pCntx,
        CHAR*                        strBuff,
        U32                          nstrBuff)
{
    ccur_strlcpy(
            strBuff,
            "       Tx Dropped:None",
            nstrBuff);
    return strBuff;
}
#endif
