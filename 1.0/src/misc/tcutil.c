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

#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <zlog.h>
#include "helper.h"
#include "tcpktparse.h"
#include "tcutil.h"

/**************** PRIVATE Functions **********************/

/**************** PROTECTED Functions **********************/

#if (FALSE == TRANSC_MEMPOOL)
void* tcUtilMemPoolAlloc(MemoryPool_t *pPool)
{
    return(calloc(1,pPool->uBlockSize));
}
void tcUtilMemPoolFree(MemoryPool_t *pPool, void *pPtr)
{
    free(pPtr);
}
#endif /* TRANSC_MEMPOOL */

CCUR_PROTECTED(U32)
tcUtilHashBytes(
    const U8*   pBytes,
    U32         nBytes
    )
{
    U32 _nHval = 0x811c9dc5;
    const U8* _pBytesEnd = pBytes + nBytes;

    while(pBytes < _pBytesEnd)
    {
        _nHval +=   (_nHval<<1) + (_nHval<<4) + (_nHval<<7)
                  + (_nHval<<8) + (_nHval<<24);
        _nHval ^= (U32)(*pBytes);
        pBytes++;
    }

    return _nHval;
}

CCUR_PROTECTED(tresult_t)
tUtilUTCTimeDiff(
    tc_gd_time_t*       pDiffTimeVal,
    const tc_gd_time_t* pTime1,
    const tc_gd_time_t* pTime2
    )
{
    tresult_t    _result;
    tc_gd_time_t _lt1;
    tc_gd_time_t _lt2;

    CCURASSERT(pTime1);
    CCURASSERT(pTime2);
    CCURASSERT(pDiffTimeVal);

    _lt1 = *pTime1;
    _lt2 = *pTime2;
    _result = ESUCCESS;
    ccur_memclear(pDiffTimeVal, sizeof(tc_gd_time_t));

    if ( (_lt1.reserved != _lt2.reserved) ||
         (_lt1.reserved != TRANSC_TIMEORIENT_UTC) )
    {
        _result = EFAILURE;
    }
    else
    {
        /* Subtract the times and handle the underruns. */
        _lt1.nMicroseconds -= _lt2.nMicroseconds;
        if (_lt1.nMicroseconds >= 1000000)
        {
            _lt1.nSeconds--;
            _lt1.nMicroseconds += 1000000;
        }
        _lt1.nSeconds -= _lt2.nSeconds;
        *pDiffTimeVal = _lt1;
    }
    return _result;
}

CCUR_PROTECTED(tresult_t)
tUtilUTCTimeGet(tc_gd_time_t *pTime)
{
    tresult_t       _result;
    struct timeval  _tv;

    _result = EFAILURE;
    ccur_memclear(
            pTime, sizeof(tc_gd_time_t));

    if (0 == gettimeofday(&_tv, NULL))
    {
        pTime->reserved         = TRANSC_TIMEORIENT_UTC;
        pTime->nSeconds         = _tv.tv_sec;
        pTime->nMicroseconds    =  _tv.tv_usec;
        _result = ESUCCESS;
    }

    return _result;
}

CCUR_PROTECTED(BOOL)
tcUtilCheckIPsEqual(
    const tc_iphdr_ipaddr_t* ptIP1,
    const tc_iphdr_ipaddr_t* ptIP2
    )
{
    BOOL _bIPsEqual = TRUE;

    if( ptIP1->eType == tcIpaddrTypeIPv4
     && ptIP1->eType == tcIpaddrTypeIPv4 )
    {
        if( 0 != memcmp(
                    &(ptIP1->ip.v4.octet[0]),
                    &(ptIP2->ip.v4.octet[0]),
                    4 )
                    )
        {
            _bIPsEqual = FALSE;
        }
    }
    else if( ptIP1->eType == tcIpaddrTypeIPv6
          && ptIP2->eType == tcIpaddrTypeIPv6 )
    {
        if( 0 != memcmp(
                    &(ptIP1->ip.v6.octet[0]),
                    &(ptIP2->ip.v6.octet[0]),
                    16 )
                    )
        {
            _bIPsEqual = FALSE;
        }
    }
    else
    {
        _bIPsEqual = FALSE;
    }

    return _bIPsEqual;
}

CCUR_PROTECTED(CHAR*)
tcUtilIPAddrtoAscii(
        tc_iphdr_ipaddr_t* addr,
        CHAR*              buffer,
        U32                buflen)
{
    CHAR*           _ipAddr;
    CCURASSERT(buffer);

    do
    {
        _ipAddr   = NULL;
        buffer[0] = '\0';
        switch(addr->eType)
        {
            case tcIpaddrTypeIPv4:
                inet_ntop(AF_INET, &(addr->ip),buffer, buflen);
                _ipAddr = buffer;
                break;

            case tcIpaddrTypeIPv6:
                inet_ntop(AF_INET6, &(addr->ip),buffer, buflen);
                _ipAddr = buffer;
                break;

            default:
                _ipAddr = NULL;
        }
    }while(FALSE);

    return _ipAddr;
}

CCUR_PROTECTED(tresult_t)
tcUtilAsciitoIPAddr(
        tc_iphdr_ipaddr_t*  pIpAddr,
        const CHAR*         strIpAddr)
{
    struct addrinfo     _hint;
    struct addrinfo*    _res;
    struct sockaddr_in  _sa;
    struct sockaddr_in6 _sa6;
    int                 _retVal;
    tresult_t           _result;

    CCURASSERT(strIpAddr);
    CCURASSERT(pIpAddr);

    do
    {
        ccur_memclear(&_hint,sizeof(_hint));
        _res = NULL;
        _hint.ai_family = PF_UNSPEC;
        _hint.ai_flags  = AI_NUMERICHOST;
        _result = getaddrinfo(strIpAddr, NULL, &_hint, &_res);
        if (ESUCCESS != _result)
        {
            /*fprintf(stderr,"%s is not ip format\n",
                    strIpAddr);*/
            _result = EFAILURE;
            break;
        }
        if(_res->ai_family == AF_INET)
        {
            _retVal = inet_pton(AF_INET,strIpAddr, &(_sa.sin_addr));
            if (0 == _retVal ||
                4 != sizeof(_sa.sin_addr))
            {
                _result = EFAILURE;
                break;
            }
            memcpy(&(pIpAddr->ip.v4.octet[0]),
                    &_sa.sin_addr,sizeof(_sa.sin_addr));
            pIpAddr->eType = tcIpaddrTypeIPv4;
        }
        else if (_res->ai_family == AF_INET6)
        {
            _retVal = inet_pton(AF_INET6,strIpAddr,&(_sa6.sin6_addr));
            if (0 == _retVal ||
                16 != sizeof(_sa6.sin6_addr))
            {
                _result = EFAILURE;
                break;
            }
            memcpy(&(pIpAddr->ip.v6.octet[0]),
                    &_sa6.sin6_addr,sizeof(_sa6.sin6_addr));
            pIpAddr->eType = tcIpaddrTypeIPv6;
        } else
        {
            _result = EFAILURE;
            /*fprintf(stderr,"%s is an is unknown address format %d\n",
                    strIpAddr,_res->ai_family);*/
        }
    }while(FALSE);
   if(_res)
       freeaddrinfo(_res);

   return _result;
}

CCUR_PROTECTED(tresult_t)
tcUtilASymHashGet(
    U32*                pHash,
    tc_iphdr_ipaddr_t*  pIpAddr)
{
    U8          _aHashBytes[32];
    U8*         _pSrcIP;
    U32         _nByteCt;
    U32         _nHash;
    tresult_t   _result;
    U16         _nIPSize;

    CCURASSERT(pIpAddr);

    do
    {
        _result  = ESUCCESS;
        _nByteCt = 0;
        *pHash   = 0;
        _nHash   = 0;
        if( tcIpaddrTypeIPv4 == pIpAddr->eType )
        {
            _nIPSize = 4;
            _pSrcIP = &(pIpAddr->ip.v4.octet[0]);
        }
        else if( tcIpaddrTypeIPv6 == pIpAddr->eType )
        {
            _nIPSize = 16;
            _pSrcIP = &(pIpAddr->ip.v6.octet[0]);
        }
        else
        {
            _result = EFAILURE;
            break;
        }
        memcpy(
                &_aHashBytes[_nByteCt],
                _pSrcIP,
                _nIPSize
                );
        _nByteCt += _nIPSize;
        _nHash = tcUtilHashBytes( _aHashBytes, _nByteCt );
        *pHash = _nHash;
    }while(FALSE);

    return _result;
}

CCUR_PROTECTED(tresult_t)
tcUtilHostGetProcessName(
    U32             nPID,
    CHAR* const     strPName,
    U32*            nPNameBufSz)
{
    U32         _nTemp = 0;
    CHAR*       _pstrTemp = NULL;
    tresult_t   _result = EINVAL;

    if ((nPID > 0) && (nPNameBufSz))
    {

        FILE* _hStatFile;
        CHAR _strTemp[80];

        snprintf(_strTemp, sizeof(_strTemp) - 1,
                     "/proc/%lu/stat", nPID);
        _result = EFAILURE;
        _hStatFile = fopen(_strTemp, "r");
        while (NULL != _hStatFile)
        {
            int     _pid;
            CHAR    _comm[255];
            CHAR    _state;
            int     _ppid;
            int     _pgrp;

            if (0 != fscanf(
                        _hStatFile,
                        "%d %s %c %d %d",
                        &_pid, &_comm[0], &_state, &_ppid, &_pgrp))
            {
                _result = ESUCCESS;
            }
            else
            {
                break;
            }
            fclose(_hStatFile);
            if (ESUCCESS != _result)
                break;
            if (_comm[0] == '(')
            {
                _pstrTemp = &_comm[1];
                _nTemp = strlen(&_comm[1]);
                _comm[_nTemp] = '\0';
            }
            else
            {
                _pstrTemp = &_comm[0];
                _nTemp = strlen(&_comm[0]) + 1;
            }
            if (NULL != strPName)
            {
                if (_nTemp <= *nPNameBufSz)
                {
                    ccur_memclear(strPName, *nPNameBufSz);
                    strcpy(strPName, _pstrTemp);
                    _result = ESUCCESS;
                }
                else
                {
                    _result = ENOBUFS;
                }
            }
            *nPNameBufSz = _nTemp;
            break;
        }
    }
    return _result;
}

CCUR_PROTECTED(tresult_t)
tcUtilHostCkProcessActive(
        CHAR* strProcName,
        CHAR* strPidFname,
        CHAR* strErr)
{
    CHAR        _strBuf[255];
    BOOL        _bProcRun = FALSE;
    U32         _nBufSz = sizeof(_strBuf) -1 ;
    U32         _pid;
    FILE*       _pidFile;
    FILE*       _hStatFile;
    tresult_t   _result;

    _result = ESUCCESS;
    if (0 < strlen(strPidFname))
    {
        _pidFile = fopen(strPidFname, "r");
        if (_pidFile)
        {
            if (1 == fscanf(_pidFile, "%lu", &_pid))
            {
                snprintf(_strBuf, sizeof(_strBuf)-1,
                             "/proc/%lu/stat", _pid);
                _hStatFile = fopen(_strBuf, "r");
                if (NULL != _hStatFile)
                {
                    _bProcRun = TRUE;
                    fclose(_hStatFile);
                }
                if (_bProcRun)
                {
                    ccur_memclear(&_strBuf, sizeof(_strBuf));
                    if(ESUCCESS == tcUtilHostGetProcessName(_pid,
                                                          _strBuf,
                                                          &_nBufSz))
                    {
                        /*
                         * Is this real matched process name?
                         */
                        if ((0 < _nBufSz) &&
                            (NULL != strstr(_strBuf,strProcName)))
                        {
                            _bProcRun = TRUE;
                        }
                    }
                    else
                    {
                        _bProcRun = TRUE;
                    }
                    if (_bProcRun)
                    {
                        _result = EFAILURE;
                        sprintf(strErr,
                                "another copy of %s (pid %lu) "
                                "is already running.\n",
                                strProcName,
                                _pid);
                        fclose(_pidFile);
                        _pidFile = NULL;
                    }
                }
                if(FALSE == _bProcRun)
                {
                    /* delete pid from pid file */
                    if (0 < strlen(strPidFname))
                    {
                        _pidFile = fopen(strPidFname, "w");
                        if (NULL != _pidFile)
                        {
                            fclose(_pidFile);
                        }
                    }
                }
            }
        }
    }

    return _result;
}

CCUR_PROTECTED(tresult_t)
tcUtilHostSaveActiveProcess(
        CHAR* strPidFname,
        CHAR* strErr)
{
    FILE*       _pidFile;
    tresult_t   _result;

    /*
     * save current PID into the process id file .
     */
    _result = ESUCCESS;
    if (0 < strlen(strPidFname))
    {
        _pidFile = fopen(strPidFname, "w");
        if (_pidFile)
        {
            fprintf(_pidFile, "%u", getpid());
            fclose(_pidFile);
        }
        else
        {
            _result = EFAILURE;
            sprintf(strErr,
                    "unable to open %s",
                    strPidFname);
        }
        _pidFile = NULL;
    }
    else
    {
        _result = EFAILURE;
        sprintf(strErr,
                "must specify process filename");
    }
    return _result;
}

/***************************************************************************
 * function: tcUtilSkipGetHttpStringLen
 *
 * description: Skips URL "GET http://.../" string and returns the length
 ***************************************************************************/
CCUR_PROTECTED(U16)
tcUtilSkipGetHttpStringLen(
        CHAR*        pPyld,
        S32          nPyldLen)
{
    CHAR*       _pPyld;
    U16         _nSkipLen;

    _pPyld    = memchr(
            pPyld+sizeof("GET")+sizeof("http://")-1,'/',nPyldLen);
    if(_pPyld &&  '/' == _pPyld[0])
        _nSkipLen = _pPyld-pPyld;
    else
        _nSkipLen = 0;

    return _nSkipLen;
}

/***************************************************************************
 * function: tcUtilMemBarrierBOOLSetStore
 *
 * description: Boolean operation Memory Barrier Write
 ***************************************************************************/
CCUR_PROTECTED(void)
tcUtilMemBarrierBOOLSetStore(
    BOOL*      pMemory,
    const BOOL eNewSet)
{
    BOOL _eOldSet;
    BOOL _bWritten;


    /* Built-in functions for atomic memory access
     * We want to replace the current feature set and guarantee atomicity.
     * First we will use an atomic opperator to read the current value,
     * then we will do a compare-and-swap using the old value.
     *
     * If another thread had already changed the value then the compare-and-
     * swap would fail, so we just loop and repeat the process.
     */
    _bWritten = FALSE;
    do
    {
        /* Atomically get the old value */
        _eOldSet = __sync_fetch_and_add(pMemory, 0);

        /* If old and new are the same we are done */
        if (_eOldSet == eNewSet)
            break;

        /* Attempt to write out the new set atomically */
        _bWritten = __sync_bool_compare_and_swap(
                        pMemory,
                        _eOldSet,
                        eNewSet
                        );
    }
    while (!_bWritten);
}

/***************************************************************************
 * function: tcUtilMemBarrierBOOLSetRead
 *
 * description: Boolean operation Memory Barrier Read
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcUtilMemBarrierBOOLSetRead(BOOL* pMemory)
{
    /* Atomically get the old value */
    return __sync_fetch_and_add(pMemory, 0);
}
