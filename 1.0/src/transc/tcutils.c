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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tcinit.h"
#include "tcutils.h"

static BOOL                        g_DaemonParentExit;
/* Any shared utilities should be defined here */
/**************** PRIVATE Functions **********************/

/***************************************************************************
 * function: _tcUtilsDaemonSignalHandler
 *
 * description: signal handler whenever the user terminates the program in
 * daemon mode.
 ***************************************************************************/
CCUR_PROTECTED(void)
_tcUtilsDaemonSignalHandler(
    int nSignalId
    )
{
    switch (nSignalId)
    {
        case SIGTERM:
        case SIGQUIT:
        {
            g_DaemonParentExit = TRUE;
            break;
        }

        default:
        {
            break;
        }
    }
}

/***************************************************************************
 * function: _tcUtilsDaemonComplete
 *
 * description: redirecting standard I/O to null.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcUtilsDaemonComplete(void)
{
    int _fd = -1;

    tcInitUsageMsgPrintBanner();
    /*
     * when complete, redirect stdin, stdout,
     *   stderr file descriptors into appropriate file devices.
     */
    _fd = open("/dev/null", O_RDWR);
    /* stdin */
    (void)dup2(_fd, STDIN_FILENO);
    close(_fd);
#if 1
    /* stdout */
    (void)dup2(STDIN_FILENO, STDOUT_FILENO);
    /* stderr */
    (void)dup2(STDIN_FILENO, STDERR_FILENO);
#else
    /*open stdout/stderr to console*/
    _fd = open("/dev/console", O_RDWR);
    /* stdout */
    (void)dup2(_fd, STDOUT_FILENO);
    /* stdout */
    (void)dup2(_fd, STDERR_FILENO);
    close(_fd);
#endif
}

/**************** PROTECTED Functions **********************/
/***************************************************************************
 * function: tcUtilsGetPfringVer
 *
 * description: get pfring version.
 ***************************************************************************/
CCUR_PROTECTED(CHAR*)
tcUtilsGetPfringVer(CHAR* strBuff,I32 version)
{
    sprintf(strBuff,
     "Supporting pfring v.%d.%d.%d",
     (version & 0xFFFF0000) >> 16,
     (version & 0x0000FF00) >> 8,
     version & 0x000000FF);

    return strBuff;
}

/***************************************************************************
 * function: _tcUtilsSignalHandlerSetup
 *
 * description: Sets up background thread signal handler
 ***************************************************************************/
CCUR_PROTECTED(void)
tcUtilsSignalHandlerSetup(void)
{
    /*http://zguide2.zeromq.org/page:all
    Getting applications to properly shut-down when you send them Ctrl-C can be tricky.
    If you use the zctx class it'll automatically set-up signal handling, but your code
    still has to cooperate.You must break any loop if zmq_poll returns -1 or if any of
    the recv methods (zstr_recv, zframe_recv, zmsg_recv) return NULL. If you have nested loops,
    it can be useful to make the outer ones conditional on !zctx_interrupted.*/

    struct sigaction _tNewAction;
    sigset_t         _tSigSet;

    sigfillset(&_tSigSet);

    sigdelset(&_tSigSet, SIGHUP);
    sigdelset(&_tSigSet, SIGINT);
    sigdelset(&_tSigSet, SIGQUIT);
    sigdelset(&_tSigSet, SIGFPE);
    sigdelset(&_tSigSet, SIGSEGV);
    sigdelset(&_tSigSet, SIGPIPE);
    sigdelset(&_tSigSet, SIGALRM);
    sigdelset(&_tSigSet, SIGTERM);
    sigdelset(&_tSigSet, SIGUSR1);
    sigdelset(&_tSigSet, SIGUSR2);
    sigdelset(&_tSigSet, SIGCHLD);

    ccur_memclear(&_tNewAction, sizeof(struct sigaction));
    _tNewAction.sa_handler = tcUtilsSignalHandler;
    _tNewAction.sa_mask    = _tSigSet;
    _tNewAction.sa_flags   = SA_NOCLDSTOP | SA_RESTART;

    sigaction(SIGHUP,  &_tNewAction, NULL);
    sigaction(SIGINT,  &_tNewAction, NULL);
    sigaction(SIGQUIT, &_tNewAction, NULL);
    sigaction(SIGPIPE, &_tNewAction, NULL);
    sigaction(SIGALRM, &_tNewAction, NULL);
    sigaction(SIGTERM, &_tNewAction, NULL);
    sigaction(SIGUSR1, &_tNewAction, NULL);
    sigaction(SIGUSR2, &_tNewAction, NULL);
    sigaction(SIGCHLD, &_tNewAction, NULL);
}
/***************************************************************************
 * function: tcUtilsDaemonize
 *
 * description: Daemonize the process by forking the current process.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcUtilsDaemonize()
{
    tresult_t   _result;
    pid_t       _pid;

    do
    {
        _result = ESUCCESS;
        if ( getppid() == 1 )
        {
            evLogTraceSys(
                   evLogLvlFatal,
                    "Error, already daemonize.\n"
                );
        }
        _pid = fork();
        switch (_pid)
        {
           case -1:
               /* error forking */
               evLogTraceSys(
                      evLogLvlFatal,
                   "Error, unable to daemonize.\n"
                   );
               _result = EFAILURE;
               break;
           case 0:
               /* child process */
               break;
           default:
           {
               /* parent process */
              /* detect signal from child to let parent process know
               * that it should exit.
               */
               signal(SIGTERM, _tcUtilsDaemonSignalHandler);
               signal(SIGCHLD, _tcUtilsDaemonSignalHandler);
               pause();
               if (FALSE == g_DaemonParentExit)
               {
                   exit(EFAILURE);
               }
              exit(ESUCCESS);
           }
        }

        /*
        * Set file mask for creating/opening files/directories.
        */
        umask(0);

        /*
        * Set the session identifier.
        */
        if(0 != _pid)
        {
            _pid = setsid();
            if (-1 == _pid)
            {
                evLogTraceSys(
                       evLogLvlFatal,
                    "Error, unable to set session id for daemonize \n");
               _result = EFAILURE;
               break;
            }
        }
        /*
        * redirecting standard I/O.
        */
        _tcUtilsDaemonComplete();

    }while(FALSE);

    return(_result);
}

/***************************************************************************
 * function: tcUtilsSignalHandler
 *
 * description: Signal handler whenever the user exits the program.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcUtilsSignalHandler(
        I32 nSignalId)
{
    switch (nSignalId)
    {
        case SIGINT:
        case SIGTERM:
        case SIGSTOP:
        case SIGQUIT:
        {
            tcShDMsgBkgrndSetExitSts(TRUE);
            break;
        }

        case SIGKILL:
        {
            exit(0);
        }

        default:
        {
            break;
        }
    }
}

/***************************************************************************
 * function: tcUtilsPopulateKeyValuePair
 *
 * description: Populate key value pair of monitoring interface and
 * filter argument.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcUtilsPopulateKeyValuePair(
        tc_utils_keyvalue_t*          pKeyValTbl,
        U16*                         nKeyValTblActive,
        U16                          nKeyValTblMax,
        CHAR*                        strCmdArgMonIntf)
{
    tresult_t   _result;
    U16         _i;
    CHAR*       _arg1;
    CHAR*       _arg2;
    CHAR*       _endStr1;
    CHAR*       _endStr2;
    CHAR*       _tmpBuff1;
    CHAR*       _tmpBuff2;

    _i = 0;
    *nKeyValTblActive = 0;
    _tmpBuff1 = strdup(strCmdArgMonIntf);
    if(_tmpBuff1)
    {
        _arg1 = strtok_r(
                _tmpBuff1,";\"",&_endStr1);
        _result = EINVAL;
        while(_arg1)
        {
            if(_i > nKeyValTblMax)
            {
                _result = ENOBUFS;
                break;
            }
            else
                _result = ESUCCESS;
            _tmpBuff2 = strdup(_arg1);
            if(_tmpBuff2)
            {
                _arg2 = strtok_r(
                        _tmpBuff2,":",&_endStr2);
                if(_arg2)
                {
                    ccur_strlcpy(pKeyValTbl[_i].strKey,
                            _arg2,sizeof(pKeyValTbl[_i].strKey));
                    ccur_strlcpy(pKeyValTbl[_i].strValue,
                            _endStr2,sizeof(pKeyValTbl[_i].strValue));
                }
            }
            if(_tmpBuff2)
                free(_tmpBuff2);
            if(ESUCCESS != _result)
                break;
            _arg1 = strtok_r( NULL, ";\"" ,&_endStr1);
            _i++;
            (*nKeyValTblActive)++;
        }
    }
    else
        _result = ENOMEM;
    if(_tmpBuff1)
        free(_tmpBuff1);
}


/***************************************************************************
 * function: tcUtilsValidConfigVersion
 *
 * description: Validates config version number.
 ***************************************************************************/
CCUR_PROTECTED(BOOL)
tcUtilsValidConfigVersion(CHAR* strVer)
{
    BOOL    _bMatch;

    _bMatch = FALSE;
    if(!strncmp(strVer,
            TRANSC_MAIN_CONF_VER,
            strlen(TRANSC_MAIN_CONF_VER)))
        _bMatch = TRUE;

    return _bMatch;
}

/***************************************************************************
 * function: tcUtilsLogW3cFormat
 *
 * description: Log data in W3c format.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcUtilsLogW3cFormat(
        U32                         tid,
        evlog_loglvl_e              lvl,
        tc_g_qmsghptopg_t*          pInjMsg,
        tc_pktdesc_t*               pPktDesc,
        evlog_t*                    pEvLog,
        evlog_desc_t*               pLogDescSys)
{
    CHAR        _strDstddr[64];
    CHAR        _strSrcddr[64];
    CHAR        _strtmbuf[64];
    CHAR        _strUrlBuf[TRANSC_SIM_URLBUFF];
    CHAR*       _pHostName;
    time_t      _tNowTime;
    struct tm*  _pNowTm;

    if(TRANSC_ZLOGCATEGORY_BITMAP_CKPTR(pLogDescSys,lvl))
        return;

    if('\0' != pInjMsg->strHostName[0])
        _pHostName = pInjMsg->strHostName;
    else
    {
        tcUtilIPAddrtoAscii(
            &(pPktDesc->ipHdr.tDstIP),
            _strDstddr,
            sizeof(_strDstddr));
        _pHostName = _strDstddr;
    }
    tcUtilIPAddrtoAscii(
        &(pPktDesc->ipHdr.tSrcIP),
        _strSrcddr,
        sizeof(_strSrcddr));
    /*The common logfile format is as follows:
    remotehost rfc931 authuser [date] "request" status bytes */
    time( &_tNowTime );
    _pNowTm = localtime(&(_tNowTime));
    strftime(_strtmbuf, sizeof(_strtmbuf), "%d/%b/%Y:%T %z",_pNowTm);
    _strUrlBuf[0] = '\0';
    if(pInjMsg->nUrlLen < sizeof(_strUrlBuf)-1)
    {
        strncpy(_strUrlBuf,pInjMsg->pUrl,pInjMsg->nUrlLen);
        _strUrlBuf[pInjMsg->nUrlLen] = '\0';
    }
    else
        ccur_strlcpy(_strUrlBuf,pInjMsg->pUrl,sizeof(_strUrlBuf)-1);

    if('\0' == pInjMsg->strUAgent[0])
    {
        evLogTrace(
                pEvLog,
                lvl,
                pLogDescSys,
                "TID:%x %s %s - [%s] \"GET %s HTTP/1.1\" - - \"-\" \"-\"\n",
                tid,
                _strSrcddr,
                _pHostName,
                _strtmbuf,
                _strUrlBuf
                );
    }
    else
    {
        evLogTrace(
                pEvLog,
                lvl,
                pLogDescSys,
                "TID:%x %s %s - [%s] \"GET %s HTTP/1.1\" - - \"-\" \"%s\"\n",
                tid,
                _strSrcddr,
                _pHostName,
                _strtmbuf,
                _strUrlBuf,
                pInjMsg->strUAgent
                );
    }
}

