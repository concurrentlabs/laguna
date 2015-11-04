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

#ifndef  PKTPRC_H
#define  PKTPRC_H

#include "tcgendefn.h"

#define TRANSC_PKTPRC_HTTP_MINPYLD_SZ       75
#define TRANSC_PKTPRC_BUFFCAPLEN            8192
#define TRANSC_PKTPRC_MINHTTPPYLD_SZ        sizeof("GET / HTTP/1.0\r\n\r\n")
#define TRANSC_PKTPRC_MAXHTTPPYLD_SZ        1536
#define TRANSC_PKTPRC_LOAD_REGEXSUBEX_NUM   36
#define TRANSC_PKTPRC_DUMPSTATS_TIME_SEC    10
#define TRANSC_PKTPRC_QUEUEFLUSH_TIME_SEC   5
#define TRANSC_PKTPRC_HUNGER_CNT            1000
#define TRANSC_PKTPRC_TCPSTRM_TTL_CNT       8000
#define TRANSC_PKTPRC_URL_STRRULESETLEN     128
#ifdef  TRANSC_BUILDCFG_LIBPCAP
/* Pcap config */
#define TRANSC_PKTPRC_BUFFWTRMRK            1
#define TRANSC_PKTPRC_FDNUM                 1
#define TRANSC_PKTPRC_SNAPLEN               2048
#define TRANSC_PKTPRC_POLLDURATION_MS       500
#define TRANSC_PCAP_READTIMEOUT_MS          200
#else
/* Pcap config */
#define TRANSC_PKTPRC_BUFFWTRMRK            1
#define TRANSC_PKTPRC_FDNUM                 1
#define TRANSC_PKTPRC_SNAPLEN               2048
#define TRANSC_PKTPRC_POLLDURATION_MS       1
#endif /* TRANSC_BUILDCFG_LIBPCAP */
/* PKT proc start wait timout */
#define TRANSC_PKTPRC_WAITTIMEOUT_MS        100
#define TRANSC_PKTPRC_URL_STRCKEYSIGLEN     512
/* Mib Overflow pkt counter */
#define TRANSC_PKTPRC_RDROVFLW_CNT          8000

#include "tcldcfg.h"
#include "tcshared.h"
#include "tcintf.h"
#include "tcmonintf.h"
#include "tcpktprc.h"
#include "tcpktprcinit.h"
#ifdef  TRANSC_BUILDCFG_LIBPCAP
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <pcap.h>
#include "tcplpcap.h"
#else
#include "tcpfrpcap.h"
#endif /* TRANSC_BUILDCFG_LIBPCAP */
#include "tcpktiorx.h"
#include "tcutils.h"

#endif /* PKTPRC_H */
