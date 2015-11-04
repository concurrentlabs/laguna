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

#ifndef  HTTPPRC_H
#define  HTTPPRC_H

#include "tcgendefn.h"

/* Sites max limits */
#define TRANSC_HTTPPRC_SITE_MAXHTTPHDRMATCH_LST         8
#define TRANSC_HTTPPRC_SITE_MAXHOSTYPE_LST              8
#define TRANSC_HTTPPRC_SITE_MAXREFTYPE_LST              4
#define TRANSC_HTTPPRC_SITE_STRHOSTSIGLEN               128
#define TRANSC_HTTPPRC_SITE_STRTYPELEN                  64
#define TRANSC_HTTPPRC_URL_STRCKEYSIGLEN                512
#define TRANSC_HTTPPRC_DUMPSTATS_TIME_SEC               10
#define TRANSC_HTTPPRC_HUNGER_CNT                       500
/* PKT proc start wait timout */
#define TRANSC_HTTPPRC_WAITTIMEOUT_MS                   100
/* URL max limits */
#define TRANSC_HTTPPRC_URL_CKEYARGS_LST                 8
/* Regex stack args and bufsize */
#define TRANSC_HTTPPRC_REGEXSUBEX_NUM                   36
#define TRANSC_HTTPPRC_REGEXSUBEX_BUFFSZ                1024
/* TCP Video Stream Sess configurations*/
#define TRANSC_HTTPPRC_TCPSTRM_DLFT_CNT                 5000
#define TRANSC_HTTPPRC_TCPSTRM_GROW_CNT                 4
#define TRANSC_HTTPPRC_TCPSTRM_UNCORRINACTIVETIME_SEC   10
#define TRANSC_HTTPPRC_TCPSTRM_CORRINACTIVETIME_SEC     30
#define TRANSC_HTTPPRC_TCPSTRM_TTLTIME_SEC              10
#define TRANSC_HTTPPRC_TCPSTRM_CTLSRCENTRIES_NUM        500
#define TRANSC_HTTPPRC_TCPSTRM_DFLTBIN_CNT              5000
/* Mib Overflow pkt counter */
#define TRANSC_HTTPPRC_RDROVFLW_CNT                     5000

#include "tcldcfg.h"
#include "tchttpproc.h"
#include "tchttpinit.h"
#include "tchttpparse.h"
#include "tctcpstrm.h"
#include "tcshared.h"
#include "tcutils.h"

#endif
