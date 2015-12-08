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

#include "health.h"

/**************************PRIVATE******************************************/
/***************************************************************************
 * function: _tcHealthFindMonIntf
 *
 * description: Find monitoring interface within mon interface table.
 ***************************************************************************/
CCUR_PRIVATE(tc_mtxtbl_ingress_t*)
_tcHealthFindMonIntf(
        tc_mtxtbl_ipptohlth_t* pPPToHealthMonIntfMsg,
        CHAR*                  str)
{
    U32                     _i;
    tc_mtxtbl_ingress_t*    _pMonIntf = NULL;

    for(_i=0;_i<pPPToHealthMonIntfMsg->nTotal;_i++)
    {
        if(!strcmp(
                pPPToHealthMonIntfMsg->tIntfITbl[_i].strIntfName,str))
        {
            _pMonIntf =
                    &(pPPToHealthMonIntfMsg->tIntfITbl[_i]);
            break;
        }
    }

    return _pMonIntf;
}

/***************************************************************************
 * function: _tcHealthGetModeOfOperationTblSz
 *
 * description: Get mode of operation table size.
 ***************************************************************************/
CCUR_PROTECTED(U16)
_tcHealthGetModeOfOperationTblSz(tc_health_thread_ctxt_t* pCntx)
{
    return pCntx->tMtxPktGenToHealthMapIntfMsg.nTotal;
}

/***************************************************************************
 * function: _tcHealthGetModeOfOperationEntry
 *
 * description: Get mode of operation Entry given index size and string.
 ***************************************************************************/
CCUR_PROTECTED(void)
_tcHealthGetModeOfOperationEntry(tc_health_thread_ctxt_t* pCntx,
        U16 idx,CHAR* strMsg,U32 nstrMsg,BOOL* pbIsActv)
{
    BOOL                         _bIsActv       = FALSE;
    tc_mtxtbl_map_pgentohlth_t*  _pMap          = NULL;
    tc_mtxtbl_ingress_t*         _pMonIntf      = NULL;
    tc_mtxtbl_ingress_t          _pNullMonIntf;
    tc_mtxtbl_egress_t*          _pOutIntf      = NULL;
    tc_mtxtbl_egress_t           _pNullOutIntf;

    ccur_memclear(&_pNullMonIntf,sizeof(tc_mtxtbl_ingress_t));
    ccur_memclear(&_pNullOutIntf,sizeof(tc_mtxtbl_egress_t));

    if(idx<pCntx->tMtxPktGenToHealthMapIntfMsg.nTotal)
    {
        _pMap = &(pCntx->tMtxPktGenToHealthMapIntfMsg.tIntfMapTbl[idx]);
        if(_pMap->nEgressIdx < TRANSC_INTERFACE_MAX)
        {
            /* format=
             * Link:<index>/<redir address>/<ingress>:<status>-<egress>:<status>/<link status>
             * Link:%d/%s/%s:%s-%s:%s/<status>
             */
            if('\0' != _pMap->strMonIntfName[0] && _pMap->bEgressIdxSet)
            {
                _pMonIntf = _tcHealthFindMonIntf(
                        &(pCntx->tMtxPPToHealthMonIntfMsg),
                        _pMap->strMonIntfName);
                _pOutIntf = &(pCntx->tMtxPktGenToHealthOutIntfMsg.tIntfETbl[_pMap->nEgressIdx]);
                if(NULL == _pMonIntf)
                    _pMonIntf = &_pNullMonIntf;
                if(NULL == _pOutIntf)
                    _pOutIntf = &_pNullOutIntf;
                if(_pMonIntf->bIntfRdy && !_pOutIntf->bIntfRdy)
                {
                    if('\0' != _pMonIntf->strRedirAddr[0])
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                _pMonIntf->strRedirAddr,
                                _pMonIntf->strIntfName,
                                "up",
                                _pOutIntf->strIntfName,
                                "down");
                    else
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                "null",
                                _pMonIntf->strIntfName,
                                "up",
                                _pOutIntf->strIntfName,
                                "down");
                    _bIsActv = FALSE;
                }
                else if(!_pMonIntf->bIntfRdy && _pOutIntf->bIntfRdy)
                {
                    if('\0' != _pMonIntf->strRedirAddr[0])
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                _pMonIntf->strRedirAddr,
                                _pMonIntf->strIntfName,
                                "down",
                                _pOutIntf->strIntfName,
                                "up");
                    else
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                "null",
                                _pMonIntf->strIntfName,
                                "down",
                                _pOutIntf->strIntfName,
                                "up");
                    _bIsActv = FALSE;
                }
                else if(_pMonIntf->bIntfRdy && _pOutIntf->bIntfRdy)
                {
                    if(_pMap->pMonActv)
                    {
                        if(_pMap->pMonActv->bIsRedirUp)
                        {
                            if('\0' != _pMonIntf->strRedirAddr[0])
                            {
                                snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                        idx,
                                        _pMonIntf->strRedirAddr,
                                        _pMonIntf->strIntfName,
                                        "up",
                                        _pOutIntf->strIntfName,
                                        "up");
                                _bIsActv = TRUE;
                            }
                            else
                            {
                                snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                        idx,
                                        "null",
                                        _pMonIntf->strIntfName,
                                        "up",
                                        _pOutIntf->strIntfName,
                                        "up");
                                _bIsActv = FALSE;
                            }
                        }
                        else
                        {
                            if('\0' != _pMonIntf->strRedirAddr[0])
                                snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                        idx,
                                        _pMonIntf->strRedirAddr,
                                        _pMonIntf->strIntfName,
                                        "up",
                                        _pOutIntf->strIntfName,
                                        "up");
                            else
                                snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                        idx,
                                        "null",
                                        _pMonIntf->strIntfName,
                                        "up",
                                        _pOutIntf->strIntfName,
                                        "up");
                            _bIsActv = FALSE;
                        }
                    }
                    else
                    {
                        if('\0' != _pMonIntf->strRedirAddr[0])
                            snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                    idx,
                                    _pMonIntf->strRedirAddr,
                                    _pMonIntf->strIntfName,
                                    "up",
                                    _pOutIntf->strIntfName,
                                    "up");
                        else
                            snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                    idx,
                                    "null",
                                    _pMonIntf->strIntfName,
                                    "up",
                                    _pOutIntf->strIntfName,
                                    "up");
                        _bIsActv = FALSE;
                    }
                }
                else
                {
                    if('\0' != _pMonIntf->strRedirAddr[0])
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                _pMonIntf->strRedirAddr,
                                _pMonIntf->strIntfName,
                                "down",
                                _pOutIntf->strIntfName,
                                "down");
                    else
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                "null",
                                _pMonIntf->strIntfName,
                                "down",
                                _pOutIntf->strIntfName,
                                "down");
                    _bIsActv = FALSE;
                }
            }
            else if('\0' != _pMap->strMonIntfName[0] && FALSE == _pMap->bEgressIdxSet)
            {
                _pMonIntf = _tcHealthFindMonIntf(
                        &(pCntx->tMtxPPToHealthMonIntfMsg),
                        _pMap->strMonIntfName);
                _pOutIntf = NULL;
                if(NULL == _pMonIntf)
                    _pMonIntf = &_pNullMonIntf;
                if(NULL == _pOutIntf)
                    _pOutIntf = &_pNullOutIntf;
                if(_pMonIntf->bIntfRdy)
                {
                    if('\0' != _pMonIntf->strRedirAddr[0])
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                _pMonIntf->strRedirAddr,
                                _pMonIntf->strIntfName,
                                "up",
                                "null",
                                "down");
                    else
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                "null",
                                _pMonIntf->strIntfName,
                                "up",
                                "null",
                                "down");
                    _bIsActv = FALSE;
                }
                else
                {
                    if('\0' != _pMonIntf->strRedirAddr[0])
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                _pMonIntf->strRedirAddr,
                                _pMonIntf->strIntfName,
                                "down",
                                "null",
                                "up");
                    else
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                "null",
                                _pMonIntf->strIntfName,
                                "down",
                                "null",
                                "up");
                    _bIsActv = FALSE;
                }
            }
            else if('\0' == _pMap->strMonIntfName[0] && _pMap->bEgressIdxSet)
            {
                _pMonIntf = NULL;
                _pOutIntf = &(pCntx->tMtxPktGenToHealthOutIntfMsg.tIntfETbl[_pMap->nEgressIdx]);
                if(NULL == _pMonIntf)
                    _pMonIntf = &_pNullMonIntf;
                if(NULL == _pOutIntf)
                    _pOutIntf = &_pNullOutIntf;
                if(_pOutIntf->bIntfRdy)
                {
                    if('\0' != _pMonIntf->strRedirAddr[0])
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                "null",
                                "null",
                                "down",
                                _pOutIntf->strIntfName,
                                "up");
                    else
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                "null",
                                "null",
                                "down",
                                _pOutIntf->strIntfName,
                                "up");
                    _bIsActv = FALSE;
                }
                else
                {
                    if('\0' != _pMonIntf->strRedirAddr[0])
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                "null",
                                "null",
                                "up",
                                _pOutIntf->strIntfName,
                                "down");
                    else
                        snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                                idx,
                                "null",
                                "null",
                                "up",
                                _pOutIntf->strIntfName,
                                "down");
                    _bIsActv = FALSE;
                }

            }
            else
            {
                snprintf(strMsg,nstrMsg,"Link:%d/%s/%s:%s-%s:%s/",
                        idx,
                        "null",
                        "null",
                        "down",
                        "null",
                        "down");
                _bIsActv = FALSE;
            }
            if(pCntx->tMtxPktGenToHealthStsMsg.bActiveMode)
            {
                if(_bIsActv && !pCntx->tMtxPktGenToHealthStsMsg.bRedirCapReached)
                {
                    ccur_strlcat(strMsg,"active",nstrMsg);
                    _bIsActv = TRUE;
                }
                else
                {
                    ccur_strlcat(strMsg,"monitor",nstrMsg);
                    _bIsActv = FALSE;
                }
                *pbIsActv = _bIsActv;
            }
            else
            {
                ccur_strlcat(strMsg,"monitor",nstrMsg);
                *pbIsActv = FALSE;
            }
            strMsg[nstrMsg-1] = '\0';
        }
        else
        {
            evLogTrace(
                    pCntx->pQHealthToBkgrnd,
                    evLogLvlFatal,
                    &(pCntx->tLogDescSys),
                   "cdn_status, map interface ping"
                   " array out of bounds",
                   _pMap->pMonActv->strRedirAddr);
        }
    }
}

/***************************************************************************
 * function: _tcHealthInitIntfActvTbl
 *
 * description: Init interface active table. This table contains redirection
 * addresses for pinging the edge or RR.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHealthInitIntfActvTbl(
        tc_health_thread_ctxt_t*    pCntx)
{
    U16                          _i;
    U16                          _j;
    BOOL                         _bInsert;
    tc_mtxtbl_ingress_t*         _pMonIntf;
    tc_mtxtbl_map_pgentohlth_t*  _pMap;

    if(pCntx->bMonIntfUpdate &&
       pCntx->bOutIntfUpdate)
    {
        ccur_memclear(pCntx->tMonActvTbl,
                sizeof(pCntx->tMonActvTbl));
        /* Sort redir address table for pinging */
        pCntx->nMonActvTbl = 0;
        for(_i=0;_i<pCntx->tMtxPktGenToHealthMapIntfMsg.nTotal;_i++)
        {
            _pMap = &(pCntx->tMtxPktGenToHealthMapIntfMsg.tIntfMapTbl[_i]);
            if(_pMap->bLinked && ('\0' != _pMap->strMonIntfName[0]))
            {
                /* Find in total */
                _pMonIntf = _tcHealthFindMonIntf(
                        &(pCntx->tMtxPPToHealthMonIntfMsg),
                        _pMap->strMonIntfName);
                if(_pMonIntf && _pMonIntf->bIntfRdy)
                {
                    _bInsert = TRUE;
                    for(_j=0;_j<pCntx->nMonActvTbl;_j++)
                    {
                        /* Find Match */
                        if( '\0' != pCntx->tMonActvTbl[_j].strRedirAddr[0] &&
                           !strcmp(pCntx->tMonActvTbl[_j].strRedirAddr,
                                _pMonIntf->strRedirAddr))
                        {
                            /* Found! don't insert */
                            _pMap->pMonActv = &(pCntx->tMonActvTbl[_j]);
                            _bInsert = FALSE;
                            break;
                        }
                    }
                    if(_bInsert)
                    {
                        /* Find empty entry within the table */
                        for(_j=0;_j<TRANSC_INTERFACE_MAX;_j++)
                        {
                            if('\0' == pCntx->tMonActvTbl[_j].strRedirAddr[0])
                            {
                                ccur_strlcpy(
                                        pCntx->tMonActvTbl[_j].strRedirAddr,
                                        _pMonIntf->strRedirAddr,
                                        sizeof(pCntx->tMonActvTbl[_j].strRedirAddr));
                                _pMap->pMonActv = &(pCntx->tMonActvTbl[_j]);
                                pCntx->nMonActvTbl++;
                                break;
                            }
                        }
                    }
                }
            }
        }
        pCntx->bMonIntfUpdate = FALSE;
        pCntx->bOutIntfUpdate  = FALSE;
    }
}

/***************************************************************************
 * function: _tcHealthRWPktPrcMsg
 *
 * description: Read/Write message from/to packet processing.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHealthRWPktPrcMsg(
        tc_health_thread_ctxt_t*    pCntx,
        BOOL                        bRead)
{
    tc_shared_pktprcmsg_t*    _pPktPrcToHlthMsg;

    if(bRead)
    {
        pthread_mutex_lock(tcShDGetCompMutex(tcTRCompTypePktPrc));
        _pPktPrcToHlthMsg = tcShDGetPktPrcMsg();
        pCntx->bMonIntfUpdate =
                _pPktPrcToHlthMsg->bMonIntfUpdate;
        if(pCntx->bMonIntfUpdate)
        {
            memcpy(&(pCntx->tMtxPPToHealthMonIntfMsg),
                   &(_pPktPrcToHlthMsg->tMonIntf),
                   sizeof(pCntx->tMtxPPToHealthMonIntfMsg));
        }
        pthread_mutex_unlock(tcShDGetCompMutex(tcTRCompTypePktPrc));
    }
}

/***************************************************************************
 * function: _tcHealthRWPktGenMsg
 *
 * description: Read/Write message from/to packet generation.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHealthRWPktGenMsg(
        tc_health_thread_ctxt_t*    pCntx,
        BOOL                        bRead)
{
    tc_shared_healthmsg_t*    _pHlthToPktGenMsg;
    tc_shared_pktgenmsg_t*    _pPktGenToHlthMsg;

    if(bRead)
    {
        pthread_mutex_lock(tcShDGetCompMutex(tcTRCompTypePktGen));
        _pPktGenToHlthMsg = tcShDGetPktGenMsg();
        pCntx->bOutIntfUpdate =
                _pPktGenToHlthMsg->bOutIntfUpdate;
        if(pCntx->bOutIntfUpdate)
        {
            memcpy(&(pCntx->tMtxPktGenToHealthOutIntfMsg),
                   &(_pPktGenToHlthMsg->tOutIntf),
                   sizeof(pCntx->tMtxPktGenToHealthOutIntfMsg));
            memcpy(&(pCntx->tMtxPktGenToHealthMapIntfMsg),
                   &(_pPktGenToHlthMsg->tMapIntf),
                   sizeof(pCntx->tMtxPktGenToHealthMapIntfMsg));
        }
        if(_pPktGenToHlthMsg->bStsUpdate)
        {
            memcpy(&(pCntx->tMtxPktGenToHealthStsMsg),
                   &(_pPktGenToHlthMsg->tSts),
                   sizeof(pCntx->tMtxPktGenToHealthStsMsg));
        }
        pthread_mutex_unlock(tcShDGetCompMutex(tcTRCompTypePktGen));
    }
    else
    {
        pthread_mutex_lock(tcShDGetCompMutex(tcTRCompTypePktGen));
        _pHlthToPktGenMsg = tcShDGetPktGenHealthMsg(tcTRCompTypePktGen);
        memcpy(&(_pHlthToPktGenMsg->tPKtGenMapIntf),
               &(pCntx->tMtxHealthToPktGenActvMsg),
               sizeof(_pHlthToPktGenMsg->tPKtGenMapIntf));
        pthread_mutex_unlock(tcShDGetCompMutex(tcTRCompTypePktGen));
    }
}

/***************************************************************************
 * function: _tcHealthWMibMsg
 *
 * description: Write message bound for MIB thread.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHealthWMibMsg(
        tc_health_thread_ctxt_t*    pCntx)
{
    tc_shared_healthmsg_t*    _pHlthToMibMsg;

    pthread_mutex_lock(tcShDGetCompMutex(tcTRCompTypeMib));
    _pHlthToMibMsg = tcShDGetPktGenHealthMsg();
    memcpy(&(_pHlthToMibMsg->tTrHealth),
           &(pCntx->tMtxHealthToMibMsg),
           sizeof(_pHlthToMibMsg->tTrHealth));
    pthread_mutex_unlock(tcShDGetCompMutex(tcTRCompTypeMib));
}

/***************************************************************************
 * function: _tcHealthUpdateStsInfo
 *
 * description: Update System or transparent caching status info.
 * The status will reflect the entire system health status whether it is
 * down or up.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHealthUpdateStsInfo(
        tc_health_thread_ctxt_t*    pCntx)
{
    tc_tr_comptype_e   _i;
    tc_tr_sts_e    _eTrStatusTbl[tcTRCompTypeMax];

    for(_i=0;_i<tcTRCompTypeMax;_i++)
    {
        _eTrStatusTbl[_i] =
                tcShProtectedDGetCompSts(_i);
        if(tcTrStsDown ==
                _eTrStatusTbl[_i])
        {
            evLogTrace(
                    pCntx->pQHealthToBkgrnd,
                    evLogLvlError,
                    &(pCntx->tLogDescSys),
                    "Component %d Down",_i);
        }
        else
            evLogTrace(
                    pCntx->pQHealthToBkgrnd,
                    evLogLvlInfo,
                    &(pCntx->tLogDescSys),
                    "Component %d Up",_i);
    }
    pCntx->tMtxHealthToMibMsg.eTrSts = tcTrStsUp;
    for(_i=0;_i<tcTRCompTypeMax;_i++)
    {
        if(tcTrStsDown ==
                _eTrStatusTbl[_i])
        {
            pCntx->tMtxHealthToMibMsg.eTrSts = tcTrStsDown;
            break;
        }
    }
    if(tcTrStsUp ==
            pCntx->tMtxHealthToMibMsg.eTrSts)
        evLogTrace(
                pCntx->pQHealthToBkgrnd,
                evLogLvlWarn,
                &(pCntx->tLogDescSys),
                "Control Plane is Up");
    else
        evLogTrace(
                pCntx->pQHealthToBkgrnd,
                evLogLvlError,
                &(pCntx->tLogDescSys),
                "Control Plane is Down");
}

/***************************************************************************
 * function: _tcHealthUpdateMOOSts
 *
 * description: Update TC Mode Of Opearation (MOO) status. The status is
 * either active or monitor.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHealthUpdateMOOSts(
        tc_health_thread_ctxt_t*    pCntx)
{
    U16                             _i;
    CHAR                            _strMsg[TRANSC_HEALTH_MOOMSG_LEN];
    BOOL                            _bIsActv = FALSE;
    U16                             _nTblSz = 0;

    cdn_status(pCntx);
    _nTblSz = _tcHealthGetModeOfOperationTblSz(pCntx);
    pCntx->tMtxHealthToPktGenActvMsg.nTotal =  _nTblSz;
    pCntx->tMtxHealthToMibMsg.nTotal        =  _nTblSz;
    for(_i=0;_i<_nTblSz;_i++)
    {
        _tcHealthGetModeOfOperationEntry(pCntx,
                _i,_strMsg,sizeof(_strMsg),&_bIsActv);
        pCntx->tMtxHealthToPktGenActvMsg.tIntfMapActvTbl[_i] = _bIsActv;
        ccur_strlcpy(pCntx->tMtxHealthToMibMsg.tIntfMapActvTbl[_i],
                _strMsg,sizeof(pCntx->tMtxHealthToMibMsg.tIntfMapActvTbl[_i]));
        evLogTrace(
                pCntx->pQHealthToBkgrnd,
                evLogLvlInfo,
                &(pCntx->tLogDescSys),
                _strMsg);
    }
}

/***************************************************************************
 * function: _tcHealthCollectAndProcessMsgs
 *
 * description: Collect and process messages from other threads.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcHealthCollectAndProcessMsgs(
        tc_health_thread_ctxt_t* pCntx,
        BOOL                     bNoLoop,
        U16                      nPingIntvl)
{
    U16 _nPing;

    CCURASSERT(pCntx);

    _nPing      = 0;
    while(!pCntx->bExit)
    {
        _tcHealthRWPktPrcMsg(pCntx,TRUE);
        _tcHealthRWPktGenMsg(pCntx,TRUE);
        if(_nPing >= nPingIntvl)
        {
            _tcHealthUpdateStsInfo(pCntx);
            _tcHealthInitIntfActvTbl(pCntx);
            _tcHealthUpdateMOOSts(pCntx);
            _tcHealthRWPktGenMsg(pCntx,FALSE);
            _tcHealthWMibMsg(pCntx);
            _nPing = 0;
        }
        _nPing++;
        sleep(1);
        if(bNoLoop)
            break;
    }
}

/**************************PROTECTED******************************************/
/***************************************************************************
 * function: tcHealthThreadEntry
 *
 * description: Entry point for health processing thread
 ***************************************************************************/
CCUR_PROTECTED(mthread_result_t)
tcHealthThreadEntry(void* pthdArg)
{
    time_t                              _tnow;
    tc_health_thread_ctxt_t*            _pCntx;
    _pCntx = (tc_health_thread_ctxt_t*)pthdArg;

    CCURASSERT(_pCntx);

    /* TODO: Move Queue init here */

    /****** 1. Thread Common Init ********/
    _tnow = time(0);
    _pCntx->tGMUptime =
            gmtime(&_tnow);
    tUtilUTCTimeGet(&(_pCntx->tUptime));
    _pCntx->tid = (U32)pthread_self();
    /****** 2. Thread Resource Init  ********/
    /* ... */
    /****** 3. Thread Synchronization  ********/
    while (!(_pCntx->bExit))
    {
       if(tcShProtectedDMsgIsCompSyncRdy(tcTRCompTypePktPrc) &&
           tcShProtectedDMsgIsCompSyncRdy(tcTRCompTypePktGen) &&
           tcShProtectedDMsgIsCompSyncRdy(tcTRCompTypeMib))
       {
           break;
       }
       else
       {
           evLogTrace(
                   _pCntx->pQHealthToBkgrnd,
                   evLogLvlInfo,
                   &(_pCntx->tLogDescSys),
                   "Health Thd syncing with other threads...");
           sleep(1);
       }
    }
    evLogTrace(
            _pCntx->pQHealthToBkgrnd,
            evLogLvlInfo,
            &(_pCntx->tLogDescSys),
            "Health Thd syncing finished!");
    _tcHealthCollectAndProcessMsgs(_pCntx,TRUE,0);
    tcShProtectedDMsgSetCompInitRdy(
            tcTRCompTypeHealth,TRUE);
    tcShProtectedDSetCompSts(tcTRCompTypeHealth,tcTrStsUp);
    /****** 4. Thread Process ********/
    /* Block here until signaled by main thread,
     * This will allow other thread, Queue
     * to be initialized. */
    while (!(_pCntx->bExit))
    {
        if (ESUCCESS == mSemCondVarSemWaitTimed(
                      &(_pCntx->tCondVarSem),
                      TRANSC_HEALTH_WAITTIMEOUT_MS
                      ))
            break;
    }
    _tcHealthUpdateStsInfo(_pCntx);
    if(FALSE == _pCntx->bExit)
    {
        evLogTrace(
                _pCntx->pQHealthToBkgrnd,
                evLogLvlInfo,
                &(_pCntx->tLogDescSys),
                "Health Thd is running with TID#%x",_pCntx->tid);
        _tcHealthCollectAndProcessMsgs(_pCntx,FALSE,TRANSC_HEALTH_PING_INTERVAL);
        tUtilUTCTimeGet(&(_pCntx->tDowntime));
    }
    tcShProtectedDSetCompSts(tcTRCompTypeHealth,tcTrStsDown);
    /****** 5. Thread Resource Destroy ********/
    /* ... */
    /****** 6. Exit application ********/
    tcShProtectedDSetAppExitSts(_pCntx->bExit);

    return ESUCCESS;
}

