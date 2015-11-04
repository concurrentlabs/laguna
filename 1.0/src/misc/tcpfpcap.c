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
#include "tcpktgen.h"
#include "tcutil.h"
#include "tctcpstrm.h"
#include "tchttpproc.h"
#include "tcsite.h"

#define BUFMAX                              16384
#define PCAP_PYLD_MINPARSELEN               100

#define PKTGEN_DEVEL_INPUT_PCAPNAME         ""
#define PKTGEN_DEVEL_OUTPUT_PCAPNAME        "./pktinject"

/* TODO: change strlen() to sizeof() */
CCUR_PROTECTED(U32)
tcUtilCkTcpPyldIntegrity(
        tc_pktdesc_t*           pPktDesc,
        U32                     nInspectedLen)
{

    BOOL            _bStopLooking   = FALSE;
    U32             _nInterestedPyldLen;
    U32             _i;
    U32             _nLst;
    U8*             _p;

    CCURASSERT(pPktDesc);

    _nInterestedPyldLen = 0;
    if(pPktDesc->tcpHdr.nPyldLen > nInspectedLen)
    {
        for( _i = 0; _i < nInspectedLen -
                          (sizeof("\r\n")-1)
            && FALSE == _bStopLooking; ++_i )
        {
            _p = &(pPktDesc->tcpHdr.pPyld[_i]);
            if( !isprint(*(_p))
             || '\r' == *(_p)
             || '\n' == *(_p) )
            {
                _bStopLooking = TRUE;
            }
            else
            {
                _nLst = _i;
                /* Check HTTP Payload Integrity
                 * to determine if we need to reassemble HTTP
                 * pkt. */

                /* If start is okay then
                 * check Tail of HTTP Payload */

                /* TODO: Check integrity on first line */
                /*...*/

                /* Check Tail and get info */
                for( _i = _nLst; _i <
                    (pPktDesc->tcpHdr.nPyldLen -
                            (sizeof("\r\n\r\n")-2)); ++_i )
                {
                    _p = &(pPktDesc->tcpHdr.pPyld[_i]);
                    if('\r' ==  *(_p)
                     && '\n' == *(_p+1)
                     && '\r' == *(_p+2)
                     && '\n' == *(_p+3))
                    {
                        _nInterestedPyldLen = _i+4;
                        break;
                    }
                }
                /* No reassembly,
                 * just pick up whatever we have */
                if(_nInterestedPyldLen == 0)
                {
                    _nInterestedPyldLen =
                            pPktDesc->tcpHdr.nPyldLen;
                }
                break;
            }
            /* Other protocol here */
        }
    }

    return(_nInterestedPyldLen);
}

CCUR_PRIVATE(void)
_tcPktGenPkthdr( FILE *f, U32 t, U32 pktlen )
{
    U32 _secs, _usecs, _len;

    CCURASSERT(f);

    /* assume t in milliseconds */
    _secs = t/1000;
    _usecs = (t % 1000) * 1000;
    _len = pktlen;

    fwrite( &_secs, 4, 1, f );
    fwrite( &_usecs, 4, 1, f );
    fwrite( &_len, 4, 1, f );
    _len = pktlen+2;
    fwrite( &_len, 4, 1, f );
}

CCUR_PRIVATE(tresult_t)
_tcPktIOWriteToPcap(
        tc_pktprc_thread_ctxt_t*    pCntx,
        FILE*                       f,
        tc_pktgen_pktinj_t*         pPkt,
        U32                         t )
{
    tresult_t _result;

    CCURASSERT(f);
    CCURASSERT(pPkt);

    do
    {
        _result = ESUCCESS;
        if( pPkt == NULL )
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlDebug,
                    EVNTLOG_CLASSDFLT,
                    "trying to write a null packet to pcap\n");
            break;
        }
        _tcPktGenPkthdr( f, t, pPkt->pktlen );
        fwrite( pPkt->pkt, 1, pPkt->pktlen, f );
        fflush(f);
    }while(FALSE);

    return _result;
}


CCUR_PROTECTED(tresult_t)
tcPktIOOpenPcap(
        tc_pktprc_thread_ctxt_t*    pCntx,
        FILE *                      f)
{
    U32 a;
    U32 b;
    tresult_t _result;

    CCURASSERT(f);

    do
    {
        _result = ESUCCESS;
        if( f == NULL )
        {
            _result = EFAILURE;
            evLogTrace(
                    pCntx->pQPktProcToBkgrnd,
                    evLogLvlDebug,
                    EVNTLOG_CLASSDFLT,
                    "attempt to write pcap to file that is not open\n");
            break;
        }

        a = 0xa1b2c3d4;
        fwrite( &a, 4, 1, f );
        b = 0x0002;
        fwrite( &b, 2, 1, f );
        b = 0x0004;
        fwrite( &b, 2, 1, f );
        a = 0x00000000;
        fwrite( &a, 4, 1, f );
        a = 0x00000000;
        fwrite( &a, 4, 1, f );
        a = 0xFFFF0000;
        fwrite( &a, 4, 1, f );
        a = 0x00000001;
        fwrite( &a, 4, 1, f );
    }while(FALSE);

    return _result;
}



/**************** PRIVATE Functions **********************/

/* Open PCAP file */
CCUR_PRIVATE(FILE *)
_tcPfpcapOpenPcap(
        tc_pktprc_thread_ctxt_t*    pCntx,
        CHAR*                       fname,
        U8*                         reverseByteOrder )
{
	U8      _pPcapBuffer[4];
	S32     _c;
	FILE*   _fp;
	S32     _i;

    CCURASSERT(fname);
    CCURASSERT(reverseByteOrder);

	_fp = fopen( fname, "rb" );
	if( _fp == NULL )
    {
	    evLogTrace(
	            pCntx->pQPktProcToBkgrnd,
	            evLogLvlFatal,
                EVNTLOG_CLASSDFLT,
                "File error\n");
		return (NULL);
    }

	for( _i=0; _i<4; _i++ )
    {
	    _c = fgetc( _fp );
		if( _c == EOF )
        {
	        evLogTrace(
	                pCntx->pQPktProcToBkgrnd,
                    evLogLvlFatal,
                    EVNTLOG_CLASSDFLT,
                    "Unexpected end of file\n");
			fclose(_fp);
			return (NULL);
    }
		_pPcapBuffer[_i] = _c;
    }

	*reverseByteOrder = -1;
	if( _pPcapBuffer[0] == 0xa1 && _pPcapBuffer[1] == 0xb2 && _pPcapBuffer[2] == 0xc3 && _pPcapBuffer[3] == 0xd4 )
		*reverseByteOrder = 0;
	if( _pPcapBuffer[3] == 0xa1 && _pPcapBuffer[2] == 0xb2 && _pPcapBuffer[1] == 0xc3 && _pPcapBuffer[0] == 0xd4 )
		*reverseByteOrder = 1;
	if( *reverseByteOrder == 0xFF )
    {
        evLogTrace(
                pCntx->pQPktProcToBkgrnd,
                evLogLvlFatal,
                EVNTLOG_CLASSDFLT,
                "Unknown file type\n");
		fclose(_fp);
		return (NULL);
    }
	for( _i=4; _i<24; _i++ )
    {
		_c = fgetc( _fp );
		if( _c == EOF )
        {
	        evLogTrace(
	                pCntx->pQPktProcToBkgrnd,
                    evLogLvlFatal,
                    EVNTLOG_CLASSDFLT,
                    "Unexpected end of file\n");
			fclose(_fp);
			return (NULL);
        }
    }

	return ( _fp );
}

/* Get next packet */
CCUR_PRIVATE(BOOL)
 _tcPfpcapGetNextPacket(
         FILE*  pFp,
         U8*    pBuffer,
         U32*   pCaplen,
         U8    reverseByteOrder )
{
    S32     _i, _j;
	BOOL    _nIpDataValid;
	S32     _c;
	U32     _hdr[4];

    CCURASSERT(pFp);
    CCURASSERT(pBuffer);
    CCURASSERT(pCaplen);

	for( _j=0; _j<4; _j++ )
    {
		for( _i=0; _i<4; _i++ )
        {
		    _c = fgetc( pFp );
			if( _c == EOF ) break;
			if( reverseByteOrder )
			    pBuffer[_i] = _c;
			else
			    pBuffer[4-_i] = _c;
        }
		_hdr[_j] = (U32) pBuffer[3];
		_hdr[_j] = (_hdr[_j] << 8) | (U32) pBuffer[2];
		_hdr[_j] = (_hdr[_j] << 8) | (U32) pBuffer[1];
		_hdr[_j] = (_hdr[_j] << 8) | (U32) pBuffer[0];
		if( _c == EOF ) break;
    }
	do
	{
	    _nIpDataValid = FALSE;
        if( _c == EOF )
        {
            break;
        }
        /*ipHdr->tstampSecs  = hdr[0];
        ipHdr->tstampUsecs = hdr[1];*/
        *pCaplen = _hdr[2];
        _j = fread( pBuffer, 1, _hdr[2], pFp );
        if( feof( pFp ) )
        {
            break;
        }
        _nIpDataValid = TRUE;
	}while(FALSE);

	return( _nIpDataValid );
}

/**************** PROTECTED Functions **********************/
CCUR_PRIVATE(tresult_t)
        _tcPfpcapInit(
                tc_gd_thread_ctxt_t*     pCntx)
{
    if( pCntx->tConfig.bPktGenToPcapFile )
    {
        if('\0' == PKTGEN_DEVEL_OUTPUT_PCAPNAME[0])
        {
            evLogTraceSys(evLogLvlFatal,EVNTLOG_CLASSDFLT,
                    "please specify filename\n");
            _result = EINVAL;
            break;
        }
        snprintf( _param, sizeof(_param)-1,
                "%s.pcap", PKTGEN_DEVEL_OUTPUT_PCAPNAME );
        pCntx->tConfig.pPktGenOutPfcap = fopen( _param, "w" );
        if(NULL == pCntx->tConfig.pPktGenOutPfcap)
        {
            evLogTrace(
                   NULL,
                   evLogLvlFatal,
                   EVNTLOG_CLASSDFLT,
                    "unable to open pcap file\n");
            _result = EFAILURE;
            break;
        }
        evLogTrace(
               NULL,
               evLogLvlInfo,
               EVNTLOG_CLASSDFLT,
                "Output to PCAP file enabled\n");
    }
}

CCUR_PROTECTED(BOOL)
tcHttpTranscHttpFilter(
        tc_pktprc_thread_ctxt_t*    pCntx,
        tc_pktdesc_t*               pPktDesc,
        CHAR*                       strFilter)
{
    BOOL                            _bProcPkt;

    CCURASSERT(pCntx);
    CCURASSERT(pPktDesc);

    do
    {
        _bProcPkt = FALSE;
        /* Remove below if added in pcap filter */
        if((pPktDesc->tcpHdr.nTcpFlags == TCTCPFLG_FIN) ||
           (pPktDesc->tcpHdr.nTcpFlags == TCTCPFLG_SYN) ||
           (pPktDesc->tcpHdr.nTcpFlags == TCTCPFLG_RST)
           )
        {
            break;
        }
        if(80 != pPktDesc->tcpHdr.nSrcPort &&
           80 != pPktDesc->tcpHdr.nDstPort)
        {
            break;
        }
        /* Add other filters here */
        if( (pPktDesc->tcpHdr.nPyldLen > 3) &&
             strFilter[0] == pPktDesc->tcpHdr.pPyld[0] &&
             strFilter[1] == pPktDesc->tcpHdr.pPyld[1] &&
             strFilter[2] == pPktDesc->tcpHdr.pPyld[2])
        {
            _bProcPkt  = TRUE;
            break;
        }
    } while(FALSE);

    return _bProcPkt;
}

CCUR_PROTECTED(U32)
tcPfpcapProcessPcap(
        tc_pktprc_thread_ctxt_t* pCntx)
{
    FILE*                           _fp;
    U8                              _buffer[BUFMAX];
    U8                              _nRevByteOrder;
    BOOL                            _IpDataValid;
    U32                             _nInterestedPyldLen;
    hlpr_httpsite_hndl              _ePossibleSiteHost;
    tc_pktdesc_t                    _pPktDesc;
    U16                             _nUriSigIdx;
    tresult_t                       _result;
    BOOL                            _bFnd;
    U32                             _nCapLen;

    CCURASSERT(pCntx);

    _result = EFAILURE;

    _fp = _tcPfpcapOpenPcap(
            pCntx,
            PKTGEN_DEVEL_INPUT_PCAPNAME,
            &_nRevByteOrder );
    if( _fp == NULL )
    {
        evLogTrace(
                pCntx->pQPktProcToBkgrnd,
                evLogLvlFatal,
                EVNTLOG_CLASSDFLT,
                "failure opening pcap file input: %s\n",
                PKTGEN_DEVEL_INPUT_PCAPNAME);
    }
    else
    {
        evLogTrace(
                pCntx->pQPktProcToBkgrnd,
                evLogLvlFatal,
                EVNTLOG_CLASSDFLT,
                "Reading from PCAP file: %s\n",
                PKTGEN_DEVEL_INPUT_PCAPNAME);
        do
        {
            if(pCntx->bExit)
                break;
            do
            {
                pCntx->nPackets++;
                _IpDataValid = _tcPfpcapGetNextPacket( _fp,_buffer, &_nCapLen, _nRevByteOrder );
                if(FALSE == _IpDataValid)
                {
                    pCntx->nPcapParseErr++;
                    break;
                }
                _pPktDesc.pMsgStrt  = (U8*)_buffer;
                _pPktDesc.nCaplen   = _nCapLen;
                _pPktDesc.nWireLen  = _nCapLen;
                _result = tcPktParse(pCntx,&_pPktDesc);
                if(EIGNORE == _result)
                {
                    pCntx->nPcapParseIgnored++;
                    break;
                }
                if(ESUCCESS != _result)
                {
                    pCntx->nPcapParseErr++;
                    break;
                }
                if(tcHttpTranscHttpFilter(
                        pCntx,&_pPktDesc,"GET"))
                {
                    _ePossibleSiteHost = EVNTLOG_CLASSDFLT;
                    _bFnd = tcHttpParseIsValidsite(
                                &_ePossibleSiteHost,
                                &_nUriSigIdx,
                                pCntx,
                                (CHAR*)_pPktDesc.tcpHdr.pPyld,
                                _pPktDesc.tcpHdr.nPyldLen);
                    if(FALSE == _bFnd)
                    {
                        pCntx->nHttpPktIgnored++;
                        break;
                    }
                    /* Protocol only Http min parser now. */
                    _nInterestedPyldLen =
                        tcUtilCkTcpPyldIntegrity(
                                &_pPktDesc,
                                PCAP_PYLD_MINPARSELEN);
                    /* Check to see if TCP is fragmented
                     * and need to be reassembled. */
                    if(_nInterestedPyldLen == 0)
                    {
                        /* Handle Fragmented TCP GET HTTP packet  */
                        pCntx->nHttpPktIgnored++;
                        break;
                    }
                    else
                    {
                        _result =
                            tcHttpProcessGetRequest(
                                    pCntx,
                                    _ePossibleSiteHost,
                                    _nUriSigIdx,
                                    &_pPktDesc,
                                    _nInterestedPyldLen);
                        if(ESUCCESS != _result)
                            pCntx->nHttpPktProcErr++;
                        else
                            pCntx->nHttpPktProcessed++;
                    }
                }
            }while(FALSE);
        }while( !feof(_fp));
        fclose( _fp );
    }

    return (_result);
}




