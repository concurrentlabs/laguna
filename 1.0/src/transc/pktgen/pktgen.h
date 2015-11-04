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

#ifndef  PKTGEN_H
#define  PKTGEN_H

#include "tcgendefn.h"

#ifdef  TRANSC_BUILDCFG_LIBPCAP
#define TRANSC_PKTGEN_NETSOCKOPTSIZE        4096
#else
#define TRANSC_PKTGEN_NETSOCKOPTSIZE        2048
#endif /* TRANSC_BUILDCFG_LIBPCAP */
#define TRANSC_PKTGEN_DUMPSTATS_TIME_SEC    30
#define TRANSC_PKTGEN_SHVALUPD_TIME_SEC     10
#define PKTGEN_OUTPUT_FRAMESIZE             1536
#define TRANSC_PKTGEN_OUTPUT_PYLDSIZE       2048
#define TRANSC_PKTGEN_OUTPUT_L1L4           2048
#define TRANSC_PKTGEN_MACADDRHDR_SZ         14
#define TRANSC_PKTGEN_IPV4HDR_SZ            20
#define TRANSC_PKTGEN_IPV6HDR_SZ            40
#define TRANSC_PKTGEN_TCPHDR_SZ             20
#define TRANSC_PKTGEN_TCPPROTO_BYTE         (0x06)

#include "tcldcfg.h"
#include "tcshared.h"
#include "tcintf.h"
#include "tcoutintf.h"
#include "tcpktgen.h"
#include "tcpktgeninit.h"
#include "tcpktinj.h"
#ifdef  TRANSC_BUILDCFG_LIBPCAP
#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include "tcplpsend.h"
#else
#include "tcpfrpsend.h"
#endif /* TRANSC_BUILDCFG_LIBPCAP */
#include "tcpktiotx.h"
#include "tcutils.h"

#endif
