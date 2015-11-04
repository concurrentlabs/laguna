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

#ifndef  TCBKGRND_H_
#define  TCBKGRND_H_

#ifdef __cplusplus
extern "C" {
#endif

enum _tc_bkgrnd_comptype_e
{
    tcBkgrndCompTypePktPrc,
    tcBkgrndCompTypeHealth,
    tcBkgrndCompTypeHttpPrc,
    tcBkgrndCompTypePktGen,
    tcBkgrndCompTypeSimMgr,
    tcBkgrndCompTypeMib,
    tcBkgrndCompTypeMax
};
typedef enum _tc_bkgrnd_comptype_e
             tc_bkgrnd_comptype_e;

#define TRANSC_BKGRNDLOG_QUEUE_MAX       tcBkgrndCompTypeMax+TRANSC_SIM_THD_MAX

/***********************Bkgrnd thd****************************************/

struct _tc_bkgrnd_evlogtbl_s
{
    U32                     nZLogCat;
    evlog_cat_t             tZLogCat[TRANSC_BKGRNDLOG_QUEUE_MAX];
};
typedef struct _tc_bkgrnd_evlogtbl_s
               tc_bkgrnd_evlogtbl_t;

struct _tc_bkgrnd_thread_ctxt_s
{
    /****** Thread Init ****************/
    U32                     tid;
    BOOL                    bExit;
    /****** Periodic time  ****************/
    U32                     nCfgLdTimeSec;
    /****** QUEUES Management ****************/
    U32                     nQEvLogTbl;
    evlog_t*                pQEvLogTbl[TRANSC_BKGRNDLOG_QUEUE_MAX];
    tc_bkgrnd_evlogtbl_t    tZLogCatComp;
    tc_bkgrnd_evlogtbl_t    tZLogCatSvc;
    evlog_desc_t            tLogDesc;
    /****** temporary variables Management */
    evlog_strblk_t          tStrBlkBuff;
};
typedef struct _tc_bkgrnd_thread_ctxt_s
               tc_bkgrnd_thread_ctxt_t;

/***********************main resources****************************************/

CCUR_PROTECTED(tresult_t)
tcBkgrndInitEventLog(
        tc_bkgrnd_thread_ctxt_t*    pCntx,
        CHAR*                       strCmdArgLogConfLoc);

CCUR_PROTECTED(tresult_t)
tcBkgrndEvLogReadFromPktProcQ(
        tc_bkgrnd_thread_ctxt_t*    pBkGnThd);

CCUR_PROTECTED(void)
tcBkgrndThreadEntry(
        tc_bkgrnd_thread_ctxt_t* pCntx);

#ifdef __cplusplus
}
#endif
#endif
