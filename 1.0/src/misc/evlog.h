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

#ifndef EVLOG_H
#define EVLOG_H

#include <zlog.h>
#include <lkfq.h>
#include "helper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EVNTLOG_CLASS_BKGRND         0
#define TRANSC_EVLOG_LOGBUFFER_SZ    8192
#define TRANSC_EVLOG_BITMAP_SZ       32

#define TRANSC_ZLOGCATEGORY_BITMAP_CKPTR(pLogDesc, lv) \
        !((pLogDesc->LvlBitmap[lv/8] >> (7 - lv % 8)) & 0x01)

#define TRANSC_ZLOGCATEGORY_BITMAP_CK(pLogDesc, lv) \
        !((pLogDesc.LvlBitmap[lv/8] >> (7 - lv % 8)) & 0x01)

typedef hlpr_httpsite_hndl  evlog_type_hndl;

enum _evlog_loglvl_e
{
    evLogLvlDebug  = ZLOG_LEVEL_DEBUG,
    evLogLvlInfo   = ZLOG_LEVEL_INFO ,
    evLogLvlNotice = ZLOG_LEVEL_NOTICE,
    evLogLvlWarn   = ZLOG_LEVEL_WARN,
    evLogLvlError  = ZLOG_LEVEL_ERROR,
    evLogLvlFatal  = ZLOG_LEVEL_FATAL
};
typedef enum _evlog_loglvl_e
             evlog_loglvl_e;

enum _evlog_class_e
{
    evLogLvlClassNone,
    evLogLvlClassCompSys,
    evLogLvlClassServices,
    evLogLvlClassMax
};
typedef enum _evlog_class_e
             evlog_class_e;

struct _evlog_desc_s
{
    evlog_class_e        eLogClass;
    evlog_type_hndl      eLogTypeHndl;
    unsigned char        LvlBitmap[TRANSC_EVLOG_BITMAP_SZ];
};
typedef struct _evlog_desc_s
               evlog_desc_t;

struct _evlog_cat_s
{
    BOOL                bSet;
    zlog_category_t*    pLogCat;
};
typedef struct _evlog_cat_s
               evlog_cat_t;

struct _evlog_strblk_s
{
    /* Perhaps make this buffer as malloc */
    CHAR                    pSb[TRANSC_EVLOG_LOGBUFFER_SZ];
    U32                     nStrWrSz;
    BOOL                    bHex;
    evlog_loglvl_e          eLvl;
    evlog_class_e           eLogClass;
    evlog_type_hndl         eLogTypeHndl;
};
typedef struct _evlog_strblk_s evlog_strblk_t;

struct _evlog_s
{
    lkfq_tc_t               tLkfq;
};
typedef struct _evlog_s evlog_t;

typedef evlog_strblk_t*  evlog_strblk_p;

CCUR_PUBLIC(tresult_t)
evLogOpenLogicalLog(
        evlog_cat_t*        pLogCatTbl,
        U16                 nLogCatTbl,
        evlog_desc_t*       tLogDesc,
        evlog_class_e       eLogClass,
        CHAR*               strLogLogicalName);

CCUR_PUBLIC(void)
evLogOpenLogicalSysLog(
        evlog_desc_t*       pLogDesc,
        CHAR*               strLogLogicalName);

CCUR_PUBLIC(zlog_category_t*)
evLogGetLogicalLogFile(
        evlog_cat_t*            pLogCatTbl,
        evlog_type_hndl      eLogTypeHndl);

CCUR_PUBLIC(void)
evLogCloseLogicalLog();

CCUR_PUBLIC(void)
evLogZlog(
        zlog_category_t*      pLog,
        evlog_loglvl_e        lvl,
        CHAR*                 format,
        va_list               args);

CCUR_PUBLIC(void)
evLogZlog2(
        zlog_category_t*      pLog,
        evlog_loglvl_e        lvl,
        CHAR*                 format,
        ...);

CCUR_PUBLIC(void)
evLogZlogHex(
        zlog_category_t*      pLog,
        evlog_loglvl_e        lvl,
        CHAR*                 str,
        U32                   nlen);

CCUR_PUBLIC(tresult_t)
evLogTrace(
        evlog_t*              pEvLog,
        evlog_loglvl_e        lvl,
        evlog_desc_t*         tLogDesc,
        CHAR*                 format,
        ...);

CCUR_PUBLIC(tresult_t)
evLogTraceHex(
        evlog_t*              pEvLog,
        evlog_loglvl_e        lvl,
        evlog_desc_t*         tLogDesc,
        CHAR*                 str,
        U32                   nStrLen);

#define evLogTraceSys(lvl,format,args...) \
    evLogTrace(NULL,lvl,NULL,format,##args)


#ifdef __cplusplus
}
#endif
#endif /* EVLOG_H */

