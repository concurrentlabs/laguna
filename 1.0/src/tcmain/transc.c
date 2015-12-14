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

//*********************************************************************************
//*********************************************************************************
void transcUsageMsgPrint(void)
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
    tcPrintSysLog(LOG_INFO,"-hc     [number],       healthcheck  mode: (0 = none, 1 = ccur, 2 = cdn)\n");
    tcPrintSysLog(LOG_INFO,"-bt     ,               block traffic\n");
    tcPrintSysLog(LOG_INFO,"-dmac   [address],      outgoing interface destination MAC address\n");
    tcPrintSysLog(LOG_INFO,"-smac   [address],      outgoing interface source MAC address\n");
    tcPrintSysLog(LOG_INFO,"-d,                     daemonize\n");
	return;
}

//*********************************************************************************
//*********************************************************************************
int main(int argc, char *argv[])
{
    tresult_t               _result;
    tc_gd_thread_ctxt_t*    _pGlbThdCntx;

	while(1)
	{
        _pGlbThdCntx = tcInitGetGlobalThdContext();
        /* Command line will overwrites config file values. */
        _result = tcInitReadFromConsole(&(_pGlbThdCntx->tMibThd.tConfig), argc, argv);
        if(ESUCCESS != _result)
            break;
        _result = tcInitEventLog(_pGlbThdCntx);
        if(ESUCCESS != _result)
            break;
        _result = tcInitDaemonize(_pGlbThdCntx->tMibThd.tConfig.bDaemonize);
        if(ESUCCESS != _result)
            break;
        _result =
                tcInitLoadConfigFiles(_pGlbThdCntx);
        if(ESUCCESS != _result)
            break;
        /* Perform all resource Initialization */
        _result = tcInitRes(_pGlbThdCntx);
        if(ESUCCESS != _result)
            break;
        /* Run Thread(s) */
        _result = tcInitRunThreads(_pGlbThdCntx);
        if(ESUCCESS != _result)
            break;
        /* Signal the parent process that it is now OK to exit if
         * in daemon mode. Otherwise just print banner and continue on.
         */
        tcInitIsSwitchDaemonMode(_pGlbThdCntx->tMibThd.tConfig.bDaemonize);
        tcBkgrndThreadEntry(&(_pGlbThdCntx->tBkGnThd));
	}
    // Cleanup all resources
    // tcInitCleanupRes(&_tThdExitSts, _pGlbThdCntx);
	exit(0);
}
