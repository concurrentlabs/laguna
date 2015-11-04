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

#ifndef HELPER_H
#define HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MISC_STDDEF_MAPPING           1
#define TRANSC_MEMPOOL                1
#if MISC_STDDEF_MAPPING
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <syslog.h>
#include <glib.h>
#include <pcre.h>
#define TRANSC_LKFQ_LOCKCK  FALSE
typedef char                CHAR;
typedef unsigned char       U8;
typedef unsigned char       BOOL;
typedef unsigned short      U16;
typedef signed short        S16;
typedef unsigned long       U32;
typedef unsigned long long  U64;
typedef int                 I32;
typedef signed long         S32;
typedef signed long         tresult_t;
typedef S32                 hlpr_httpsite_hndl;
#define ccur_memclear(x,z)      memset(x,0,z)
#define ccur_strlcpy            g_strlcpy
#define ccur_strlcat            g_strlcat
#define ccur_strnstr(a,b,c)     g_strstr_len(a,c,b)
#define ccur_regex_new          g_regex_new
#define ccur_regex_free         g_regex_free
#define ccur_strcasecmp         g_ascii_strcasecmp
#define ccur_strncasecmp        g_ascii_strncasecmp
#define tcOpenSysLog            openlog
#ifdef TRANSC_DEBUG
#define CCURASSERT(x)           assert(x)
#define tcPrintSysLog(p,...)    printf(__VA_ARGS__)
#else
#define CCURASSERT(x)           (void)0
#define tcPrintSysLog(p,...)    syslog(p,"["TRANSC_PROCNAME"]:"__VA_ARGS__)
#endif
#define ESUCCESS        0
#define EFAILURE        -1
#define EIGNORE         50
#define ENOSUPPORT      51
#define ENOVID          53
#ifndef TRUE
#define TRUE            1
#endif /* TRUE */
#ifndef FALSE
#define FALSE           0
#endif /* FALSE */
#define CCUR_PRIVATE(_t)                    static _t
#define CCUR_THREAD_ENTRY(_t)               _t
#define CCUR_PROTECTED(_t)                  _t
#define CCUR_PUBLIC(_t)                     _t
#endif /* MISC_STDDEF_MAPPING */

#define TRANSC_TIMEORIENT_UTC           0x1
#define TRANSC_TIMEORIENT_LOCALTZ       0x2

/*
 * Time storage definition.
 */
struct _tc_gd_time_s
{
    U32 nSeconds;
    U32 nMicroseconds;
    U32 reserved;
};
typedef struct _tc_gd_time_s
               tc_gd_time_t;

#ifdef __cplusplus
}
#endif
#endif /* HELPER_H */
