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

#ifndef SIMPRC_H
#define SIMPRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tcgendefn.h"

#define TRANSC_SIM_CURLTIMEOUT_MS                       2000
#define TRANSC_SIM_WKRDUMPSTATS_CNTR                    15000
#define TRANSC_SIM_WKRDUMPSTATS_TIME_SEC                60
#define TRANSC_SIM_DUMPSTATS_CNTR                       15000
#define TRANSC_SIM_DUMPSTATS_TIME_SEC                   60
/* SIM proc start wait timout */
#define TRANSC_SIM_WAITTIMEOUT_MS                       100
/* Q error counters */
#define TRANSC_SIMPRC_RDROVFLW_CNT                      2000
#define TRANSC_SIMSNDPRC_RDROVFLW_CNT                   2000
/* Mempool */
#define TRANSC_SIMCKEY_DLFT_CNT                         2500
#define TRANSC_SIMCKEY_GROW_CNT                         5
/* Hash Table Size */
#define TRANSC_SIMCKEY_DFLTBIN_CNT                      8000
#define TRANSC_SIMCKEY_UNCORRINACTIVETIME_SEC           20
#define TRANSC_SIMCKEY_TTL_CNT                          4000
#define TRANSC_SIMCKEY_CTLSRCENTRIES_NUM                400
/* buffer length */
#define TRANSC_SIMCKEY_URL_MAX_SIZE                     2048
#define TRANSC_SIMCKEY_HTTPBUFF_MAX_SIZE                2048
#define TRANSC_SIMCKEY_HTTPCONTENTLEN_BUFF_SIZE         102400

#include "tcldcfg.h"
#include "tcsimutil.h"
#include "tcsimsend.h"
#include "tcsim.h"
#include "tcshared.h"
#include "tcutils.h"

#ifdef __cplusplus
}
#endif
#endif /* SIMPRC_H */
