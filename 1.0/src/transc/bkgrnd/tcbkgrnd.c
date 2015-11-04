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

#include <czmq.h>
#include "bkgrnd.h"
/**************************PRIVATE******************************************/
/***************************************************************************
 * function: _tcBkgrndThreadListenMsg
 *
 * description: Background thread listening on queue requests from pkt proc
 * thread. TCS signal handler is being hijacked by snmp plane zero MQ therefore
 * snmp plane will catch the signal instead of background thread.
 * There are two ways the application can be terminated
 * 1. signal caught by snmp plane and it will set the
 * zctx_interrupted to non-negative number.
 * 2. thread initialization error, which will
 * set zctx_interrupted to non-negative number.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcBkgrndThreadListenMsg(tc_bkgrnd_thread_ctxt_t * pCntx)
{
    tresult_t           _PollSts;
    BOOL                _bAppExitSts;

    /* Reading from pkt proc thread */
    _PollSts = tcBkgrndEvLogReadFromPktProcQ(pCntx);
    if(ENODATA == _PollSts)
        usleep(100);
    /* A thread is exiting, exit all threads */
    if(tcShProtectedDGetAppExitSts(FALSE))
    {
        _bAppExitSts =
                tcShProtectedDGetAppExitSts(TRUE);
        if(_bAppExitSts)
            zctx_interrupted =  1;
    }
    /* Overwrites boolean information */
    tcShDMsgBkgrndSetExitSts(zctx_interrupted);
}

/**************************PROTECTED******************************************/
/***************************************************************************
 * function: tcBkgrndInitEventLog
 *
 * description: Initialize main event logging, which belongs to background
 * thread.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcBkgrndInitEventLog(
        tc_bkgrnd_thread_ctxt_t*    pCntx,
        CHAR*                       strCmdArgLogConfLoc)
{
    tresult_t                   _result;

    CCURASSERT(pCntx);
    CCURASSERT(strCmdArgLogConfLoc);

    do
    {
        _result = (tresult_t)
                zlog_init(strCmdArgLogConfLoc);
        if(ESUCCESS != _result)
            break;
        evLogOpenLogicalSysLog(
                &(pCntx->tLogDesc),
                TRANSC_LOGCLASS_COMP_BACKGRND);
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: tcBkgrndEvLogReadFromPktProcQ
 *
 * description: pull message from queue of
 * all logs from all components.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcBkgrndEvLogReadFromPktProcQ(
        tc_bkgrnd_thread_ctxt_t*    pCntx)
{
    tresult_t                                   _result;
        U32                                     _i;
        BOOL                                    _bNoData;
        BOOL                                    _bData;
        evlog_strblk_t*                         _pStrBlk = NULL;
        zlog_category_t*                        _pLog = NULL;

        CCURASSERT(pCntx);

        _bNoData = TRUE;
        _bData   = FALSE;
        for(_i=0;_i<pCntx->nQEvLogTbl;_i++)
        {
            _pStrBlk = (evlog_strblk_t*)
                    lkfqRead(&(pCntx->pQEvLogTbl[_i]->tLkfq));
            if(_pStrBlk)
            {
                pCntx->tStrBlkBuff.bHex         = _pStrBlk->bHex;
                pCntx->tStrBlkBuff.eLogClass    = _pStrBlk->eLogClass;
                pCntx->tStrBlkBuff.eLogTypeHndl = _pStrBlk->eLogTypeHndl;
                pCntx->tStrBlkBuff.eLvl         = _pStrBlk->eLvl;
                pCntx->tStrBlkBuff.nStrWrSz     = _pStrBlk->nStrWrSz;
                ccur_strlcpy(pCntx->tStrBlkBuff.pSb,
                        _pStrBlk->pSb,sizeof(pCntx->tStrBlkBuff.pSb));
                lkfqReadRelease(
                    &(pCntx->pQEvLogTbl[_i]->tLkfq),
                    (lkfq_data_p)_pStrBlk);
                if(pCntx->tStrBlkBuff.nStrWrSz)
                {
                    if(evLogLvlClassCompSys ==
                            pCntx->tStrBlkBuff.eLogClass)
                    {
                        if(pCntx->tStrBlkBuff.eLogTypeHndl >= 0 &&
                                pCntx->tStrBlkBuff.eLogTypeHndl < TRANSC_BKGRNDLOG_QUEUE_MAX)
                        {
                            _pLog =
                                    evLogGetLogicalLogFile(
                                            pCntx->tZLogCatComp.tZLogCat,
                                            pCntx->tStrBlkBuff.eLogTypeHndl);
                        }
                    }
                    else if(evLogLvlClassServices ==
                            pCntx->tStrBlkBuff.eLogClass)
                    {
                        if(pCntx->tStrBlkBuff.eLogTypeHndl >= 0 &&
                                pCntx->tStrBlkBuff.eLogTypeHndl < TRANSC_BKGRNDLOG_QUEUE_MAX)
                        {
                            _pLog =
                                    evLogGetLogicalLogFile(
                                            pCntx->tZLogCatSvc.tZLogCat,
                                            pCntx->tStrBlkBuff.eLogTypeHndl);
                        }
                    }
                    else
                        _pLog = NULL;
                    if(_pLog)
                    {
                        if(pCntx->tStrBlkBuff.bHex)
                        {
                            evLogZlogHex(
                                    _pLog,
                                    pCntx->tStrBlkBuff.eLvl,
                                    pCntx->tStrBlkBuff.pSb,
                                    pCntx->tStrBlkBuff.nStrWrSz);
                        }
                        else
                        {
                            evLogZlog2(
                                    _pLog,
                                    pCntx->tStrBlkBuff.eLvl,
                                    "%s",
                                    pCntx->tStrBlkBuff.pSb);
                        }
                    }
                }
                _bData = TRUE;
            }
            else
                _bNoData = TRUE;
        }
    #if 1
        if(FALSE==_bData &&
           TRUE == _bNoData)
            _result = ENODATA;
        else
            _result = ESUCCESS;
    #endif

        return _result;
}

/***************************************************************************
 * function: tcBkgrndThreadEntry
 *
 * description: Thread entry for background thread.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcBkgrndThreadEntry(
        tc_bkgrnd_thread_ctxt_t* pCntx)
{
    do
    {
        /* Call any background processing here */
        _tcBkgrndThreadListenMsg(pCntx);
    }while(!tcShDMsgBkgrndGetExitSts());
    tcShProtectedDSetCompSts(tcTRCompTypeLog,tcTrStsDown);
}

