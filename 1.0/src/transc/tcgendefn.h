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

#ifndef TCGENDEFN_H
#define TCGENDEFN_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  TRANSC_BUILDCFG_LIBPCAP
#include <netdb.h>
#include <poll.h>
#include <ctype.h>
#include <pcap.h>
#else
#include <ctype.h>
#include <pfring.h>
#endif /* TRANSC_BUILDCFG_LIBPCAP */
#include "tcconfig.h"
#include "http_parser.h"
#include "yaml.h"
#include "helper.h"
#include "msem.h"
#include "mthread.h"
#include "evlog.h"
#include "tcpktparse.h"
#include "tcregex.h"
#include "tcutil.h"
#include "tcqmsg.h"
#ifdef TRANSC_DEBUG
#define tcPrintSysLog(p,...)    printf(__VA_ARGS__)
#else
#define tcPrintSysLog(p,...)    syslog(p,"["TRANSC_PROCNAME"]:"__VA_ARGS__)
#endif /* TRANSC_DEBUG */

#ifdef __cplusplus
}
#endif
#endif /* GENDEFN_H_ */
