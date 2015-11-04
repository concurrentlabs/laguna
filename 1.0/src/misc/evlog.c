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

#include "evlog.h"

zlog_category_t*    g_pSysLog;

CCUR_PUBLIC(void)
evLogOpenLogicalSysLog(
        evlog_desc_t*       pLogDesc,
        CHAR*               strLogLogicalName)
{
    g_pSysLog =
            zlog_get_category(strLogLogicalName);
    pLogDesc->eLogTypeHndl =
            EVNTLOG_CLASS_BKGRND;
    pLogDesc->eLogClass =
            evLogLvlClassCompSys;
}

/* event log handle  maps to catagory table, which is stored
 * within bkground thread. Log desriptor can be passed to
 * background thread through Queue, which then used by
 * background thread to find the correct catagory.
 */
CCUR_PUBLIC(tresult_t)
evLogOpenLogicalLog(
        evlog_cat_t*        pLogCatTbl,
        U16                 nLogCatTbl,
        evlog_desc_t*       tLogDesc,
        evlog_class_e       eLogClass,
        CHAR*               strLogLogicalName)
{
    tresult_t           _result;
    BOOL                _bFnd;
    U16                 _nClassSz;
    evlog_type_hndl  _eLogTypeHndl;
    evlog_cat_t*        _pCatTbl;

    do
    {
        _result         = EFAILURE;
        _eLogTypeHndl   = 0;
        _pCatTbl         = &(pLogCatTbl[0]);
        _nClassSz       = nLogCatTbl;
        if((evLogLvlClassCompSys != eLogClass &&
           evLogLvlClassServices != eLogClass))
            break;
        _bFnd = FALSE;
        /* Find available atagory from the catagory table */
        for(_eLogTypeHndl=0;
                _eLogTypeHndl<_nClassSz;
                _eLogTypeHndl++)
        {
            if(FALSE == _pCatTbl[_eLogTypeHndl].bSet)
            {
                _bFnd = TRUE;
                break;
            }
        }
        if(FALSE == _bFnd)
            break;
        _pCatTbl[_eLogTypeHndl].pLogCat =
                zlog_get_category(strLogLogicalName);
        if(NULL == _pCatTbl[_eLogTypeHndl].pLogCat)
            break;
        else
        {
            /* Populate log descriptors */
            tLogDesc->eLogTypeHndl  = _eLogTypeHndl;
            tLogDesc->eLogClass     = eLogClass;
            memset(tLogDesc->LvlBitmap, 0xff, sizeof(tLogDesc->LvlBitmap));
        }
        _pCatTbl[_eLogTypeHndl].bSet = TRUE;
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

CCUR_PUBLIC(zlog_category_t*)
evLogGetLogicalLogFile(
        evlog_cat_t*            pLogCatTbl,
        evlog_type_hndl      eLogTypeHndl)
{
    zlog_category_t*    pLogCat;

    pLogCat = NULL;
    if(pLogCatTbl[eLogTypeHndl].bSet)
        pLogCat  = (pLogCatTbl[eLogTypeHndl].pLogCat);

    return pLogCat;
}

CCUR_PUBLIC(void)
evLogCloseLogicalLog()
{
    zlog_fini();
}

CCUR_PUBLIC(void)
evLogZlog2(
        zlog_category_t*      pLog,
        evlog_loglvl_e        lvl,
        CHAR*                 format,
        ...)
{
    va_list args;

    CCURASSERT(pLog);
    CCURASSERT(format);

    va_start(args, format);
    switch(lvl)
    {
        case evLogLvlDebug:
            vzlog_debug(pLog,format,args);
            break;
        case evLogLvlInfo:
            vzlog_info(pLog,format,args);
            break;
        case evLogLvlNotice:
            vzlog_notice(pLog,format,args);
            break;
        case evLogLvlWarn:
            vzlog_warn(pLog,format,args);
            break;
        case evLogLvlError:
            vzlog_error(pLog,format,args);
            break;
        case evLogLvlFatal:
            vzlog_fatal(pLog,format,args);
            break;
        default:
            break;
    }
    va_end(args);
}

CCUR_PUBLIC(void)
evLogZlog(
        zlog_category_t*      pLog,
        evlog_loglvl_e        lvl,
        CHAR*                 format,
        va_list               args)
{
    CCURASSERT(pLog);
    CCURASSERT(format);

    switch(lvl)
    {
        case evLogLvlDebug:
            vzlog_debug(pLog,format,args);
            break;
        case evLogLvlInfo:
            vzlog_info(pLog,format,args);
            break;
        case evLogLvlNotice:
            vzlog_notice(pLog,format,args);
            break;
        case evLogLvlWarn:
            vzlog_warn(pLog,format,args);
            break;
        case evLogLvlError:
            vzlog_error(pLog,format,args);
            break;
        case evLogLvlFatal:
            vzlog_fatal(pLog,format,args);
            break;
        default:
            break;
    }
}

CCUR_PUBLIC(void)
evLogZlogHex(
        zlog_category_t*      pLog,
        evlog_loglvl_e        lvl,
        CHAR*                 str,
        U32                   nlen)
{
    CCURASSERT(pLog);
    CCURASSERT(str);

    switch(lvl)
    {
        case evLogLvlDebug:
            hzlog_debug(pLog,str,nlen);
            break;
        case evLogLvlInfo:
            hzlog_info(pLog,str,nlen);
            break;
        case evLogLvlNotice:
            hzlog_notice(pLog,str,nlen);
            break;
        case evLogLvlWarn:
            hzlog_warn(pLog,str,nlen);
            break;
        case evLogLvlError:
            hzlog_error(pLog,str,nlen);
            break;
        case evLogLvlFatal:
            hzlog_fatal(pLog,str,nlen);
            break;
        default:
            break;
    }
}

CCUR_PUBLIC(tresult_t)
evLogTraceHex(
        evlog_t*              pEvLog,
        evlog_loglvl_e        lvl,
        evlog_desc_t*         pLogDesc,
        CHAR*                 str,
        U32                   nStrLen)
{
    tresult_t                        _result;
    evlog_strblk_p                   _pStrBlk;

    CCURASSERT(str);

    _result = ESUCCESS;
    if(pEvLog)
    {
        if (TRANSC_ZLOGCATEGORY_BITMAP_CKPTR(pLogDesc, lvl))
            return _result;
        _pStrBlk = (evlog_strblk_p)lkfqMalloc(&(pEvLog->tLkfq));
        if(_pStrBlk)
        {
            if(nStrLen > sizeof(_pStrBlk->pSb)-1)
                nStrLen = sizeof(_pStrBlk->pSb)-1;
            memcpy(_pStrBlk->pSb,str,nStrLen);
            /* Pass log descriptor along with other values,
             * never pass any pointer to a queue, it will
             * defeats the purpose of using queue. */
            _pStrBlk->pSb[sizeof(_pStrBlk->pSb)-1] = '\0';
            _pStrBlk->pSb[nStrLen]          = '\0';
            _pStrBlk->nStrWrSz              = nStrLen;
            _pStrBlk->bHex                  = TRUE;
            _pStrBlk->eLvl                  = lvl;
            _pStrBlk->eLogClass             = pLogDesc->eLogClass;
            _pStrBlk->eLogTypeHndl          = pLogDesc->eLogTypeHndl;
            /* write and commit to mutex free queue */
            lkfqWrite(&(pEvLog->tLkfq),(lkfq_data_p)_pStrBlk);
        }
        else
            _result = ENOMEM;
    }
    else
    {
        if(g_pSysLog)
            evLogZlogHex(g_pSysLog,lvl,str,nStrLen);
        else
            _result = EFAILURE;
    }

    return _result;
}

CCUR_PUBLIC(tresult_t)
evLogTrace(
        evlog_t*              pEvLog,
        evlog_loglvl_e        lvl,
        evlog_desc_t*         pLogDesc,
        CHAR*                 format,
        ...)
{
    va_list                          _args;
    tresult_t                        _result;
    evlog_strblk_p                   _pStrBlk;
    U32                              _nStrLen;

    CCURASSERT(format);

    _result = ESUCCESS;
    if(pEvLog)
    {
        /* quick check without invoking zlog */
        if (TRANSC_ZLOGCATEGORY_BITMAP_CKPTR(pLogDesc, lvl))
            return _result;
        _pStrBlk = (evlog_strblk_p)lkfqMalloc(&(pEvLog->tLkfq));
        if(_pStrBlk)
        {
            va_start(_args, format);
            _nStrLen = (U32)
                vsnprintf(_pStrBlk->pSb,
                          sizeof(_pStrBlk->pSb),format,_args);
            va_end(_args);
            if(_nStrLen > sizeof(_pStrBlk->pSb)-1)
                _nStrLen = sizeof(_pStrBlk->pSb)-1;
            /* Pass log descriptor along with other values,
             * never pass any pointer to a queue, it will
             * defeats the purpose of using queue. */
            _pStrBlk->pSb[sizeof(_pStrBlk->pSb)-1] = '\0';
            _pStrBlk->nStrWrSz              = _nStrLen;
            _pStrBlk->bHex                  = FALSE;
            _pStrBlk->eLvl                  = lvl;
            _pStrBlk->eLogClass             = pLogDesc->eLogClass;
            _pStrBlk->eLogTypeHndl          = pLogDesc->eLogTypeHndl;
            /* write and commit to mutex free queue */
            lkfqWrite(&(pEvLog->tLkfq),(lkfq_data_p)_pStrBlk);
        }
        else
            _result = ENOMEM;
    }
    else
    {
        if(g_pSysLog)
        {
            va_start(_args, format);
            evLogZlog(g_pSysLog,lvl,format,_args);
            va_end(_args);
        }
        else
            _result = EFAILURE;
    }

    return _result;
}
