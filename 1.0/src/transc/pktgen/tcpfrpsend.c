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

#ifndef  TRANSC_BUILDCFG_LIBPCAP
/***************************************************************************
* function: tcPfrPsendInitIsTxIntfLinkUp
*
* description: Check if Tx link is up
****************************************************************************/
CCUR_PROTECTED(BOOL)
tcPfrPsendInitIsTxIntfLinkUp(
        tc_pktgen_thread_ctxt_t * pCntx)
{
    BOOL                        _bIntfUp;
    tc_outintf_out_t*        _pOutIntf;
    U16                         _i;

    CCURASSERT(pCntx);

    _bIntfUp        = FALSE;
    _pOutIntf       = NULL;
    for(_i=0;_i<pCntx->tIntfX.nOutIntfOpen;_i++)
    {
        _pOutIntf = &(pCntx->tIntfX.tOutIntfTbl[_i]);
        if('\0' != _pOutIntf->tIntf.strIntfName[0])
        {
            if(0 ==
                    pfring_get_link_status(
                            _pOutIntf->pRingOutIntf))
            {
                evLogTrace(
                        pCntx->pQPktGenToBkgrnd,
                        evLogLvlError,
                        &(pCntx->tLogDescSys),
                        "Checking, Outgoing link \"%s:%s\" is down!, error - link is not yet ready",
                        _pOutIntf->tIntf.strIntfName,
                        _pOutIntf->tIntf.strIntfVal);
                _pOutIntf->tIntf.bIntfRdy = FALSE;
            }
            else
            {
                evLogTrace(
                        pCntx->pQPktGenToBkgrnd,
                        evLogLvlInfo,
                        &(pCntx->tLogDescSys),
                        "Checking, Outgoing link \"%s:%s\" Up!",
                        _pOutIntf->tIntf.strIntfName,
                        _pOutIntf->tIntf.strIntfVal);
                _pOutIntf->tIntf.bIntfRdy = TRUE;
            }
        }
    }
    /* Transc won't be up if one interface is down
     * during data structure init */
    if(pCntx->tIntfX.nOutIntfOpen > 0)
        _bIntfUp        = TRUE;
    else
        _bIntfUp        = FALSE;
    for(_i=0;_i<pCntx->tIntfX.nOutIntfOpen;_i++)
    {
        if(FALSE ==
                pCntx->tIntfX.tOutIntfTbl[_i].tIntf.bIntfRdy)
        {
            _bIntfUp = FALSE;
            break;
        }
    }

    return _bIntfUp;
}


/***************************************************************************
 * function: tcPfrPsendInitLibPfringTx
 *
 * description: Init pfring Tx ring
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPfrPsendInitLibPfringTx(tc_pktgen_thread_ctxt_t*  pCntx,
                          U16                       nIntfIdx)
{
    int                     _rc;
    unsigned int            _version;
    CHAR                    _strBuff[128];
    tc_outintf_out_t*      _pOutIntf = NULL;
    /*struct timespec       _ltime; For Hw timestamp*/
    tresult_t               _result;

    do
    {
        _result = EINVAL;
        if(nIntfIdx >= TRANSC_INTERFACE_MAX)
            break;
        _pOutIntf = &(pCntx->tIntfX.tOutIntfTbl[nIntfIdx]);
        if('\0' == _pOutIntf->tIntf.strIntfName[0])
            break;
        _pOutIntf->pRingOutIntf = pfring_open(
                        _pOutIntf->tIntf.strIntfName,
                        TRANSC_PKTGEN_NETSOCKOPTSIZE,
                        PF_RING_PROMISC);
        if(NULL == _pOutIntf->pRingOutIntf)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "pfring_open on %s error - errno:[%s]",
                    _pOutIntf->tIntf.strIntfName, strerror(errno));
            break;
        }
        _rc = pfring_version(_pOutIntf->pRingOutIntf,&_version);
        if(0 != _rc )
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, unable to get pfring version number");
            break;
        }
        if(_version < TRANSC_COMPAT_PFRING_VER)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlFatal,
                    &(pCntx->tLogDescSys),
                    "Invalid pfring library version! "
                    "minimal compat needed %s, currently used %s",
                    tcUtilsGetPfringVer(_strBuff,TRANSC_COMPAT_PFRING_VER),
                    tcUtilsGetPfringVer(_strBuff,RING_VERSION_NUM));
            break;
        }
        sprintf(_strBuff,"transc_tx_%d",nIntfIdx);
        _rc = pfring_set_application_name(
                _pOutIntf->pRingOutIntf,_strBuff);
        if(0 != _rc )
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error,Tx Unable to set application name");
            break;
        }

        /* int pfring_set_device_clock(pfring *ring, struct timespec *ts)
          Sets the time in the device hardware clock, when the adapter supports
          hardware timestamping. */
        /*if((_nFlags & PF_RING_HW_TIMESTAMP) &&
           (clock_gettime(CLOCK_REALTIME, &_ltime) != 0 ||
            pfring_set_device_clock(g_ringtx,&_ltime) < 0))
        {
        }*/
       /* int pfring_set_direction(pfring *ring, packet_direction direction)
          Tell PF_RING to consider only those packets matching the specified
          direction. If the application does not call this function, all the
          packets (regardless of the direction, either RX or TX) are returned.
          typedef enum {
              rx_and_tx_direction = 0,
              rx_only_direction,
              tx_only_direction
            } packet_direction;*/
        _rc = pfring_set_direction(_pOutIntf->pRingOutIntf,tx_only_direction);
        if(_rc != 0)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, Tx Unable to set direction");
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
        _rc = pfring_set_socket_mode(_pOutIntf->pRingOutIntf,send_only_mode);
        if(_rc != 0)
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, Tx Unable to set socket mode");
            break;
        }
        if(pfring_enable_ring(_pOutIntf->pRingOutIntf))
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktGenToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Error, Tx Unable to enable ring");
            break;
        }
        _result = ESUCCESS;
    }while(FALSE);

    if(_pOutIntf && ESUCCESS != _result)
        tcPfrPsendShutdownLibPfringTx(_pOutIntf);

    return _result;
}

/***************************************************************************
 * function: tcPfrPsendShutdownLibPfringTx
 *
 * description: Shutdown Pfring Tx Ring
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcPfrPsendShutdownLibPfringTx(
        tc_outintf_out_t*    pOutIntf)
{
    pfring_close(pOutIntf->pRingOutIntf);
    pOutIntf->pRingOutIntf = NULL;

    return ESUCCESS;
}

/***************************************************************************
 * function: tcPfrPsendLibPfringTx
 *
 * description: Send Raw packet through Pfring Tx Ring
 ***************************************************************************/
CCUR_PROTECTED(I32)
tcPfrPsendLibPfringTx(
        tc_pktgen_thread_ctxt_t*        pCntx,
        tc_outintf_out_t*               pOutIntf,
        tc_pktgen_pktinj_t*             pkt)
{
    CCURASSERT(pkt);

    return((I32)pfring_send(pOutIntf->pRingOutIntf,
            (CHAR*)(pkt->pkt),pkt->pktlen,TRUE));
}

/***************************************************************************
 * function: tcPfrPsendGetStatPfringTx
 *
 * description: Get Pfring Tx Stat
 ***************************************************************************/
CCUR_PROTECTED(CHAR*)
tcPfrPsendGetStatPfringTx(
        tc_pktgen_thread_ctxt_t*    pCntx,
        CHAR*                       strBuff,
        U32                         nstrBuff)
{
    pfring_stat                 _stats;
    CHAR                        _strbuff[256];
    tc_outintf_out_t*          _pOutIntf;
    U16                         _i;

    strBuff[0] = '\0';
    for(_i=0;_i<pCntx->tIntfX.nOutIntfOpen;_i++)
    {
        _pOutIntf = &(pCntx->tIntfX.tOutIntfTbl[_i]);
        if(NULL == _pOutIntf)
            break;
        pfring_stats(_pOutIntf->pRingOutIntf,&_stats);
        snprintf(_strbuff,
                sizeof(_strbuff),
                "       Egress:%d\n"
                "       %s Rx Dropped:%lu\n"
                "       %s Rx Recv:%lu\n"
                "       ********************\n",
                _i,
                _pOutIntf->tIntf.strIntfName,_stats.drop,
                _pOutIntf->tIntf.strIntfName,_stats.recv);
        _strbuff[sizeof(_strbuff)-1] = '\0';
        ccur_strlcat(strBuff,_strbuff,nstrBuff);
    }

    return strBuff;
}
#endif /* TRANSC_BUILDCFG_LIBPCAP */

