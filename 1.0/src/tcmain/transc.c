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

#include "tcinit.h"

/**************** PRIVATE Functions **********************/

CCUR_PRIVATE(void)
_transCUsageMsgPrint()
{
    tcInitUsageMsgPrintBanner();
    tcPrintSysLog(LOG_INFO,"USAGE:  transc [-<flag> [<val>],...]\n");
    tcPrintSysLog(LOG_INFO,"-i      [name],         Monitor interface name\n");
    tcPrintSysLog(LOG_INFO,"-o      [name],         Output interface name\n");
    tcPrintSysLog(LOG_INFO,"-r      [filter],       packet capture filter\n");
    tcPrintSysLog(LOG_INFO,"-t      [ipaddr],       ccur caching server ip addr\n");
    tcPrintSysLog(LOG_INFO,"-c,     [config],       trouting config location\n");
    tcPrintSysLog(LOG_INFO,"-sc,    [config],       sys config location\n");
    tcPrintSysLog(LOG_INFO,"-lc,    [log-config],   log config location\n");
    tcPrintSysLog(LOG_INFO,"-m      [mode],         mode of operation: active or passive\n");
    tcPrintSysLog(LOG_INFO,"-b      [ipaddrs],      ip address black list ',' separated\n");
    tcPrintSysLog(LOG_INFO,"-st     [number],       number of simulation threads %d\n",TRANSC_SIM_THD_MAX);
    tcPrintSysLog(LOG_INFO,"-sm     ,               simulation mode\n");
    tcPrintSysLog(LOG_INFO,"-aml    ,               adding mpls label to output injection pkt\n");
    tcPrintSysLog(LOG_INFO,"-lq     ,               locked queue if specified\n");
    tcPrintSysLog(LOG_INFO,"-ppr    ,               process http proxy request\n");
    tcPrintSysLog(LOG_INFO,"-icr    ,               ignore CORS (Cross Origin Resource Sharing) request\n");
    tcPrintSysLog(LOG_INFO,"-nrrp   ,               No Request Router (RR) polling\n");
    tcPrintSysLog(LOG_INFO,"-bt     ,               block traffic\n");
    tcPrintSysLog(LOG_INFO,"-dmac   [address],      outgoing interface destination MAC address\n");
    tcPrintSysLog(LOG_INFO,"-smac   [address],      outgoing interface source MAC address\n");
    tcPrintSysLog(LOG_INFO,"-d,                     daemonize\n");
}

/**************** PUBLIC Functions **********************/
CCUR_PUBLIC(int)
main( int argc, char *argv[] )
{
    tresult_t               _result;
    BOOL                    _bUsageMsg;
    BOOL                    _bLdCfgFail;
    BOOL                    _bLdProcDFail;
    BOOL                    _bLdInitResFail;
    BOOL                    _bLdThdRunFail;
    mthread_result_t        _tThdExitSts;
    tc_gd_thread_ctxt_t*    _pGlbThdCntx;

    do
    {
        _bLdCfgFail         = TRUE;
        _bLdProcDFail       = TRUE;
        _bLdInitResFail     = TRUE;
        _bLdThdRunFail      = TRUE;
        _bUsageMsg          = TRUE;
        _pGlbThdCntx = tcInitGetGlobalThdContext();
        /* Command line will overwrites config file values. */
        _result = tcInitReadFromConsole(
                &(_pGlbThdCntx->tMibThd.tConfig),argc,argv);
        if(ESUCCESS != _result)
            break;
        _result =
                tcInitEventLog(_pGlbThdCntx);
        if(ESUCCESS != _result)
            break;
        _result = tcInitDaemonize(
                _pGlbThdCntx->tMibThd.tConfig.bDaemonize);
        if(ESUCCESS != _result)
            break;
        _bLdProcDFail = FALSE;
        _result =
                tcInitLoadConfigFiles(_pGlbThdCntx);
        if(ESUCCESS != _result)
            break;
        _bLdCfgFail = FALSE;
        /* Perform all resource Initialization */
        _result = tcInitRes(_pGlbThdCntx);
        if(ESUCCESS != _result)
            break;
        _bLdInitResFail = FALSE;
        /* Run Thread(s) */
        _result = tcInitRunThreads(_pGlbThdCntx);
        if(ESUCCESS != _result)
            break;
        _bLdThdRunFail = FALSE;
        /* Signal the parent process that it is now OK to exit if
         * in daemon mode. Otherwise just print banner and continue on.
         */
        tcInitIsSwitchDaemonMode(
                _pGlbThdCntx->tMibThd.tConfig.bDaemonize);
        tcBkgrndThreadEntry(&(_pGlbThdCntx->tBkGnThd));
        _bUsageMsg = FALSE;
    }while(FALSE);

    if(_bUsageMsg)
        _transCUsageMsgPrint();

    /* Cleanup all resources */
    tcInitCleanupRes(
            &_tThdExitSts,
            _pGlbThdCntx);

    if(ESUCCESS != _result ||
       ESUCCESS != _tThdExitSts)
    {
        if(_bLdCfgFail)
            _result = 2;
        else if(_bLdProcDFail)
            _result = 3;
        else if(_bLdInitResFail)
            _result = 4;
        if(_bLdThdRunFail)
            _result = 5;
        else
            _result = 1;
    }

    exit(_result);
}

