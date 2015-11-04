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

#ifndef  TCUTIL_H_
#define  TCUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "helper.h"

/* Http Parser Mapping */
#define tcHttpParserExecute          http_parser_execute
#define tcHttpParserInit             http_parser_init

#define TCUTIL_IS_URLABSPATH(x) ('/' == x[0])
#define TCUTIL_IS_URLHTTPSTRING(x) ('h' == x[0] && \
                                    't' == x[1] && \
                                    't' == x[2] && \
                                    'p' == x[3] && \
                                    ':' == x[4] && \
                                    '/' == x[5] && \
                                    '/' == x[6])

/* Tom's good'ol Link lists operations */

/* Single  linked list */
typedef struct  _sll_node_s_
{
    struct _sll_node_s_  *sllNext;
}  sll_node_t;

/* Single linked list Head Node*/
typedef struct  _sll_hdnode_s_
{
    sll_node_t*     phdNode;
    U32             nCntr;
}  sll_hdnode_t;

/* Null Terminated Double-Link List */
typedef struct  _ndll_node_s_
{
    struct _ndll_node_s_  *dllPrev;
    struct _ndll_node_s_  *dllNext;
}  ndll_node_t;

/* Null doubly linked list Head Node */
typedef struct  _ndll_hdnode_s_
{
    ndll_node_t*    phdNode;
    U32             nCntr;
}  ndll_hdnode_t;

/* Circular doubly linked list */
typedef struct  _cdll_node_s_
{
    struct _cdll_node_s_  *dllPrev;
    struct _cdll_node_s_  *dllNext;
}  cdll_node_t;

/* Circular doubly linked list Head Node */
typedef struct  _cdll_hdnode_s_
{
    cdll_node_t*    phdNode;
    U32             nCntr;
}  cdll_hdnode_t;

#ifndef CCUR_SIG_LLINLINE_UNSUPPORTED
#define CcurSllInsertToHeadList(head, new_node)                   \
        if((*((sll_node_t **) (head))))                                     \
        {                                                                   \
            ((sll_node_t *) (new_node)) -> sllNext =                        \
                                (*((sll_node_t **) (head)));                \
            (*((sll_node_t **) (head))) = ((sll_node_t *) (new_node));      \
        }                                                                   \
        else                                                                \
        {                                                                   \
            (*((sll_node_t **) (head))) = ((sll_node_t *) (new_node));      \
        }                                                                   \

#define  CcurSllInsertToEndList(node, new_node)                   \
        if((*((sll_node_t **) (node))))                                     \
        {                                                                   \
            (*((sll_node_t **) (node))) -> sllNext =                        \
                                ((sll_node_t *) (new_node));                \
            (*((sll_node_t **) (node))) =                                   \
                                ((sll_node_t *) (new_node));                \
        }                                                                   \
        else                                                                \
        {                                                                   \
            (*((sll_node_t **) (node))) =                                   \
                               ((sll_node_t *) (new_node));                 \
        }                                                                   \

#define CcurNdllInsertPrevToNodeList(head,node,new_node)          \
        if (*((ndll_node_t **) (head)))                                     \
        {                                                                   \
            ((ndll_node_t *) (new_node)) -> dllNext =                       \
                            ((ndll_node_t *) (node));                       \
            ((ndll_node_t *) (new_node)) -> dllPrev =                       \
                            ((ndll_node_t *) (node)) -> dllPrev;            \
            if(((ndll_node_t *) (node)) -> dllPrev)                         \
                (((ndll_node_t *) (node)) -> dllPrev) -> dllNext =          \
                            ((ndll_node_t *) (new_node));                   \
            else                                                            \
                (*((ndll_node_t **) (head))) =                              \
                            ((ndll_node_t *) (new_node));                   \
            ((ndll_node_t *) (node)) -> dllPrev =                           \
                            ((ndll_node_t *) (new_node));                   \
        }                                                                   \
        else                                                                \
        {                                                                   \
            if((ndll_node_t *) (node))                                      \
                (*((ndll_node_t **) (head))) =                              \
                        ((ndll_node_t *) (node));                           \
            else                                                            \
                (*((ndll_node_t **) (head))) =                              \
                        ((ndll_node_t *) (new_node));                       \
            ((ndll_node_t *) (new_node)) -> dllPrev = NULL;                 \
            ((ndll_node_t *) (new_node)) -> dllNext = NULL;                 \
        }                                                                   \

#define CcurNdllInsertNextToNodeList(head,node,new_node)          \
        if (*((ndll_node_t **) (head)))                                     \
        {                                                                   \
            ((ndll_node_t *) (new_node)) -> dllPrev =                       \
                        ((ndll_node_t *) (node));                           \
            ((ndll_node_t *) (new_node)) -> dllNext =                       \
                        ((ndll_node_t *) (node)) -> dllNext;                \
            if(((ndll_node_t *) (node)) -> dllNext)                         \
                (((ndll_node_t *) (node)) -> dllNext) -> dllPrev =          \
                        ((ndll_node_t *) (new_node));                       \
            ((ndll_node_t *) (node)) -> dllNext =                           \
                    ((ndll_node_t *) (new_node));                           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            if((ndll_node_t *) (node))                                      \
                (*((ndll_node_t **) (head))) =                              \
                        ((ndll_node_t *) (node));                           \
            else                                                            \
                (*((ndll_node_t **) (head))) =                              \
                        ((ndll_node_t *) (new_node));                       \
            ((ndll_node_t *) (new_node)) -> dllPrev = NULL;                 \
            ((ndll_node_t *) (new_node)) -> dllNext = NULL;                 \
        }                                                                   \

#define CcurNdllRemoveNodeFromList(head,node)                     \
        {                                                                   \
            if((ndll_node_t *) (node) -> dllNext)                           \
                (((ndll_node_t *) (node)) -> dllNext) ->dllPrev =           \
                ((ndll_node_t *) (node))-> dllPrev;                         \
            if((ndll_node_t *) (node)-> dllPrev)                            \
                (((ndll_node_t *) (node)) -> dllPrev) ->dllNext =           \
                ((ndll_node_t *) (node))->dllNext;                          \
            if((*((ndll_node_t **) (head))) ==                              \
                            ((ndll_node_t *) (node)))                       \
                (*((ndll_node_t **) (head))) =                              \
                        ((ndll_node_t *) (node))->dllNext;                  \
        }                                                                   \

#define CcurCdllInsertToList(head, new_node);                     \
        if (*((cdll_node_t **) (head)))                                     \
        {                                                                   \
            ((cdll_node_t *) (new_node)) -> dllPrev=                        \
                    (*((cdll_node_t **) (head))) -> dllPrev;                \
            (((cdll_node_t *) (new_node)) -> dllPrev) -> dllNext =          \
                    (cdll_node_t *) (new_node);                             \
            ((cdll_node_t *) (new_node)) -> dllNext =                       \
                    (*((cdll_node_t **) (head)));                           \
            (((cdll_node_t *) (new_node)) -> dllNext) -> dllPrev =          \
                    ((cdll_node_t *) (new_node));                           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            (*((cdll_node_t **) (head))) =                                  \
                    ((cdll_node_t *) (new_node));                           \
            ((cdll_node_t *) (new_node)) -> dllPrev =                       \
                    ((cdll_node_t *) (new_node));                           \
            ((cdll_node_t *) (new_node)) -> dllNext =                       \
                    ((cdll_node_t *) (new_node));                           \
        }

#define CcurCdllRemoveFromList(head, node);                       \
        if (((cdll_node_t *) (node)) -> dllPrev ==                          \
                                    ((cdll_node_t *) (node)))               \
        {                                                                   \
            (*((cdll_node_t **) (head))) =  NULL;                           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            (((cdll_node_t *) (node)) -> dllPrev) -> dllNext =              \
                             ((cdll_node_t *) (node)) -> dllNext;           \
            (((cdll_node_t *) (node)) -> dllNext) -> dllPrev =              \
                             ((cdll_node_t *) (node)) -> dllPrev;           \
            if (((cdll_node_t *) (node)) == *((cdll_node_t **) (head)))     \
                *((cdll_node_t **) (head)) =                                \
                    ((cdll_node_t *) (node)) -> dllNext;                    \
        }

#else
/* Single link list  */
CCUR_PROTECTED(void)
CcurSllInsertToHeadList(
        sll_node_t** head,sll_node_t* new_node
        );
CCUR_PROTECTED(void)
CcurSllInsertToEndList(
        sll_node_t** node,sll_node_t* new_node
        );
/* Null terminated Dll */
CCUR_PROTECTED(void)
CcurNdllInsertPrevToNodeList(
        ndll_node_t** head,ndll_node_t* node,ndll_node_t* new_node
        );
CCUR_PROTECTED(void)
CcurNdllInsertNextToNodeList(
        ndll_node_t** head,ndll_node_t* node,ndll_node_t* new_node
        );
CCUR_PROTECTED(void)
CcurNdllRemoveNodeFromList(
        ndll_node_t** head,ndll_node_t* node
        );
CCUR_PROTECTED(void)
/* Circular Dll */
CcurCdllInsertToList(
        sll_node_t** head,sll_node_t* new_node
        );
CCUR_PROTECTED(void)
CcurCdllRemoveFromList(
        sll_node_t** head,sll_node_t* new_node
        );
#endif

/*tcUtilMemPoolGetStats      */

CCUR_PROTECTED(tresult_t)
tUtilUTCTimeGet(
        tc_gd_time_t *pTime);

CCUR_PROTECTED(tresult_t)
tUtilUTCTimeDiff(
    tc_gd_time_t*       pDiffTimeVal,
    const tc_gd_time_t* pTime1,
    const tc_gd_time_t* pTime2);

CCUR_PROTECTED(CHAR*)
tUtilUTCGetGmTime(
        CHAR* pTbuf,
        U16   nTBuf);

CCUR_PROTECTED(U32)
tcUtilHashBytes(
    const U8*   pBytes,
    U32         nBytes);

CCUR_PROTECTED(BOOL)
tcUtilCheckIPsEqual(
    const tc_iphdr_ipaddr_t* ptIP1,
    const tc_iphdr_ipaddr_t* ptIP2);

CCUR_PROTECTED(CHAR*)
tcUtilIPAddrtoAscii(
        tc_iphdr_ipaddr_t* addr,
        CHAR*              buffer,
        U32                buflen);

CCUR_PROTECTED(tresult_t)
tcUtilAsciitoIPAddr(
        tc_iphdr_ipaddr_t*  pIpAddr,
        const CHAR*         strIpAddr);

CCUR_PROTECTED(tresult_t)
tcUtilASymHashGet(
    U32*                pHash,
    tc_iphdr_ipaddr_t*  pIpAddr);

CCUR_PROTECTED(tresult_t)
tcUtilHostGetProcessName(
    U32             nPID,
    CHAR* const     strPName,
    U32*            nPNameBufSz);


CCUR_PROTECTED(tresult_t)
tcUtilHostCkProcessActive(
        CHAR* strProcName,
        CHAR* strPidFname,
        CHAR* strErr);

CCUR_PROTECTED(tresult_t)
tcUtilHostSaveActiveProcess(
        CHAR* strPidFname,
        CHAR* strErr);

CCUR_PROTECTED(U16)
tcUtilSkipGetHttpStringLen(
        CHAR*        pPyld,
        S32          nPyldLen);

CCUR_PROTECTED(void)
tcUtilMemBarrierBOOLSetStore(
    BOOL*      pMemory,
    const BOOL eNewSet);

CCUR_PROTECTED(BOOL)
tcUtilMemBarrierBOOLSetRead(BOOL* pMemory);


#if 0
/* Below is temporary function,
 * will be replaced with proper function. */
CCUR_PROTECTED(U32)
ccur_strlcat(
    CHAR *dst,
    const CHAR *src,
    U32 len
    );

CCUR_PROTECTED(U32)
ccur_strlcpy(
    CHAR *dst,
    const CHAR *src,
    U32 len
    );

CCUR_PROTECTED(CHAR *)
ccur_strnstr(
    const char *s,
    const char *find,
    size_t slen);
#endif
#ifdef __cplusplus
}
#endif
#endif
