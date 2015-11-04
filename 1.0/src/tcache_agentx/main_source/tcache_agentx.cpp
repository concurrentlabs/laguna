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

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <syslog.h>
#include <regex.h>
#include <unistd.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <climits>
#include <cstdlib>
#include <fstream>
#include <ostream>
#include <string>
#include <sstream>
#include <thread>

#include <czmq.h>

#include "tcache_agentx.h"
#include "clientTable.h"
#include "clientRedirectTable.h"
#include "domainTable.h"
#include "domainRedirectTable.h"
#include "redirectedServiceTable.h"
#include "tcPlaneStatus.h"
#include "trafficCount.h"
#include "redirectCount.h"
#include "maxTableXmit.h"
#include "edgeProbeDuration.h"
#include "tablePurgeDuration.h"
#include "tcPlaneStartTime.h"
#include "tcPlaneVersion.h"
#include "modeTable.h"
#ifdef __UNSUPPORTED_TABLES__
#include "clientTopDeviceTable.h"
#include "videoTable.h"
#include "videoRedirectTable.h"
#endif

#include "../util_source/io_rtns.h"
#include "../util_source/tcp_rtns.h"

using namespace std;

static bool daemon_init(void);
static bool init_oid_handlers(void);

static void init_logger(uint32_t level);
#ifdef __SIGNAL_HAMDLER_THREAD__
static void sig_handl(void);
#endif

// Uncomment to add in handling of management services.
// #define HNDL_MGT_SVCS 1
#ifdef HNDL_MGT_SVCS
static int tcache_agentx_process(int mgt_sock, int block);

static void handle_mgt_request(int mgt_sock);
static void handle_purge_request(int mgt_sock, string & request);
static void handle_config_request(int mgt_sock);

const string PURGE_REQUEST  = "components/Transparentcache?command=purge&host=";
const string CONFIG_REQUEST = "components/Transparentcache?command=reread-config";

const std::string OK          = "HTTP/1.0 200 OK\r\n";
const std::string BAD_REQUEST = "HTTP/1.0 400 Bad Request\r\n";
#endif

//****************************************************************************************
//****************************************************************************************
int main (int argc, char **argv)
{
    try
    {
        int log_level = LOG_CRIT;
        if(getopt(argc, argv, "l:") == 'l')
            log_level  = atoi(optarg);
        init_logger(log_level);

        if(!daemon_init())
            throw(string("daemon_init() failed."));

        //*** write pid to file, purpose of sending signals ***
        ofstream stream("/var/run/tcache_agentx.pid");
        stream << getpid() << endl;
        stream.close();

#ifdef __SIGNAL_HAMDLER_THREAD__
        //*** Block signals in main thread ***
        sigset_t mask;
        if(sigfillset(&mask))
            throw(string("sigfillset failed in main thread."));
        if(sigprocmask(SIG_BLOCK, &mask, NULL))
            throw(string("sigprocmask failed in main thread."));

        //*** detach thread to handle signals ***
        thread signal_thread(sig_handl);
        signal_thread.detach();
#endif
    }
    catch(const string & err)
    {
        syslog(LOG_CRIT, "EXCEPTION: %s terminating.", err.c_str());
        terminate();
    }
    catch(exception & e)
    {
        syslog(LOG_CRIT, "EXCEPTION: %s terminating.", e.what());
        terminate();
    }

    netsnmp_enable_subagent();
    netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID,
        NETSNMP_DS_AGENT_X_SOCKET, "tcp:0.0.0.0:705");
        // NETSNMP_DS_AGENT_X_SOCKET, "tcp:localhost:705");
    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
        NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL, 240);
    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
        NETSNMP_DS_AGENT_AGENTX_RETRIES, 0);
    // netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
    //     NETSNMP_DS_AGENT_AGENTX_TIMEOUT, 30);
    SOCK_STARTUP;
    init_agent("tcache_agentx");
    init_snmp("tcache_agentx");

#ifdef HNDL_MGT_SVCS
    int mgt_sock = socket_setup(MGT_SERVICE);
    if(mgt_sock < 0)
    {
        syslog(LOG_CRIT, "EERROR: socket_setup(MGT_SERVICE) %d terminating.", errno);
        terminate();
    }
#endif

    if(!init_oid_handlers())
        return -1;

    syslog(LOG_INFO, "%s", "tcache_agentx begin processing...");
    while(true)
        agent_check_and_process(1);

    snmp_shutdown("tcache-agentx");
    SOCK_CLEANUP;
#ifdef HNDL_MGT_SVCS
    shutdown(mgt_sock, 2);
    close(mgt_sock);
#endif
    exit(0);
}

//****************************************************************************************
// Function: daemon_init
//
// Description: Daemonize the server, return true on success, false on error.
//****************************************************************************************
static bool daemon_init(void)
{
    auto close_fds = []()
    {
        int32_t maxfd = sysconf(_SC_OPEN_MAX);
        if(maxfd < 0)
            maxfd = 20;
        for(int32_t fd = 0; fd < maxfd; ++fd)
            close(fd);
    };

    switch(fork())
    {
        case -1:
            return false;
        case 0:
            break;
        default:
            exit(0);
    }
    if(setsid() < 0)
        return -1;
    if(chdir("/") < 0)
        return -1;
    (void) umask(0);

    close_fds();
    return true;
}

#ifdef __SIGNAL_HAMDLER_THREAD__
//****************************************************************************************
// function: sig_handl
//
// description: Signal catcher.
//****************************************************************************************
static void sig_handl(void)
{
    sigset_t mask;
    int32_t sig{};

    if(sigfillset(&mask))
    {
        syslog(LOG_CRIT, "sigfillset failed in signal thread.");
        terminate();
    }
    while(true)
    {
        if(sigwait(&mask, &sig))
        {
            syslog(LOG_CRIT, "sigwait failed in signal thread.");
            terminate();
        }
        syslog(LOG_INFO, "After sigwait, caught signal: %d.", sig);

        if(sig == SIGTERM)
        {
            syslog(LOG_INFO, "Signal Thread caught SIGTERM, halting tcache-agentx.");
            terminate();
        }
    }
    return;
}
#endif

//****************************************************************************************
// function: init_logger
//
// description: Initialize syslog to given level.
//****************************************************************************************
static void init_logger(uint32_t level)
{
    openlog("tcache_agentx: ", LOG_PID, LOG_LOCAL0); 
    int32_t log_mask{};
    for(uint32_t i = 0; i <= level; ++i)
        log_mask |= LOG_MASK(i);
    (void) setlogmask(log_mask);
    return;
}

//****************************************************************************************
// function: init_oid_handlers
//
// description: Initialize tcache-agentx handlers.
//****************************************************************************************
static bool init_oid_handlers(void)
{
    if(!init_domainTable())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_domainTable, exiting....");
        return false;
    }
    if(!init_domainRedirectTable())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_domainRedirectTable, exiting....");
        return false;
    }
    if(!init_clientTable())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_clientTable, exiting....");
        return false;
    }
    if(!init_clientRedirectTable())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_clientRedirectTable, exiting....");
        return false;
    }
    if(!init_redirectedServiceTable())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_redirectedServiceTable, exiting....");
        return false;
    }
    if(!init_tcPlaneStatus())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_cplaneStatus, exiting....");
        return false;
    }
    if(!init_trafficCount())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_trafficCount, exiting....");
        return false;
    }
    if(!init_redirectCount())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_redirectCount, exiting....");
        return false;
    }
    if(!init_maxTableXmit())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_maxTableXmit, exiting....");
        return false;
    }
    if(!init_edgeProbeDuration())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_edgeProbeDuration, exiting....");
        return false;
    }
    if(!init_tablePurgeDuration())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_tablePurgeDuration, exiting....");
        return false;
    }
    if(!init_tcPlaneStartTime())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_tcPlaneStartTime, exiting....");
        return false;
    }
    if(!init_tcPlaneVersion())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_tcPlaneVersion, exiting....");
        return false;
    }
    if(!init_modeTable())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_modeTable, exiting....");
        return false;
    }
#ifdef __UNSUPPORTED_TABLES__
    if(!init_clientTopDeviceTable())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_clientTopDeviceTable, exiting....");
        return false;
    }
    if(!init_videoTable())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_videoTable, exiting....");
        return false;
    }
    if(!init_videoRedirectTable())
    {
        syslog(LOG_CRIT, "ERROR: tcache-agentx: init_videoRedirectTable, exiting....");
        return false;
    }
#endif
    return true;
}

#ifdef HNDL_MGT_SVCS
//****************************************************************************************
// function: tcache_agentx_process
//
// description: Process tcache_agentx requests.
//****************************************************************************************
static int tcache_agentx_process(int mgt_sock, int block)
{
    int numfds = mgt_sock + 1, fakeblock{};
    fd_set rdset{};
    struct timeval timeout{ LONG_MAX, 0 }, *tvp = &timeout;
    FD_SET(mgt_sock, &rdset);

    snmp_select_info(&numfds, &rdset, tvp, &fakeblock);
    if(block != 0 && fakeblock != 0)
        tvp = NULL;
    else if(!block)
        tvp->tv_sec = tvp->tv_usec = 0;
    int count = select(numfds, &rdset, NULL, NULL, tvp);
    if(count > 0)
    {
        snmp_read(&rdset);

        if(FD_ISSET(mgt_sock, &rdset))
        {
            syslog(LOG_DEBUG, "%s", "management_event");
            int fd = tcp_accept(mgt_sock);
            if(fd > 0)
                handle_mgt_request(fd);
            else
                syslog(LOG_CRIT, "ERROR: tcp_accept, errno = %d", errno);
        }
    }
    else if(!count)
        snmp_timeout();
    else
        syslog(LOG_INFO, "ERROR: select(), errno == %d", errno); 

    run_alarms();
    netsnmp_check_outstanding_agent_requests();
    return count;
}

//****************************************************************************************
// function: handle_mgt_request
//
// description: Handle request from Management Frontend.
//****************************************************************************************
static void handle_mgt_request(int mgt_sock)
{
    char buffer[512];
    int ret;

    bzero(buffer, sizeof(buffer));
    if((ret = timed_recv(mgt_sock, buffer, sizeof(buffer), 0)) <= 0)
    {
        if(!ret)
            syslog(LOG_DEBUG, "mgt_request: socket %d hung up.", mgt_sock);
        else
            syslog(LOG_DEBUG, "mgt_request timed_recv ERROR: %d.", errno);
    }
    syslog(LOG_DEBUG, "mgt_request: Recieved: \"%s\".", buffer);

    string s = buffer;
    if((s.find("POST") != string::npos) && (s.find(PURGE_REQUEST) != string::npos))
    {
        syslog(LOG_DEBUG, "%s", "purge event.");
        handle_purge_request(mgt_sock, s);
    }
    else if((s.find("POST") != string::npos) && (s.find(CONFIG_REQUEST) != string::npos))
    {
        syslog(LOG_DEBUG, "%s", "reread-config event.");
        handle_config_request(mgt_sock);
    }
    close(mgt_sock);
    return;
}

//****************************************************************************************
// function: handle_purge_request
//
// description: Send purge purge request to Transparent Cache Edge Server.
//****************************************************************************************
static void handle_purge_request(int mgt_sock, string & request)
{
    size_t idx{};
    string s, host, target, expression;
    ostringstream ss;

    expression = "target=";
    idx = request.rfind(expression);
    if(idx != string::npos)
    {
        s = request.substr(idx);
        idx = s.find("=");
        if(idx != string::npos)
            target = s.substr(idx + 1);
    }

    expression = "host=";
    idx = request.rfind(expression);
    s = request.substr(idx);
    idx = s.find("&");
    if((idx != string::npos))
        s.resize(idx);
    idx = s.find("=");
    host = s.substr(idx + 1);

    ss << "tcp://" << host.c_str() << ':' << HTTP_SERVICE;
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, ss.str().c_str());

    ss.str("");
    if(target.size())
        ss << "PURGE /tcspurge/ccur/" << target << "/* " << "HTTP/1.0\r\n";
    else
        ss << "PURGE /tcspurge/ccur/*" << " HTTP/1.0\r\n";
    zmq_send(requester, ss.str().c_str(), ss.str().length(), 0);

    ss.str("");
    ss << "Host: "  << host;
    zmq_send(requester, ss.str().c_str(), ss.str().length(), 0);
 
    char buffer[64];
    bzero(buffer, sizeof(buffer));
    if(timed_read(requester, buffer, sizeof(buffer), READ_TIMEOUT))
    {
        s = buffer;
        if(s == OK)
            send(mgt_sock, OK.c_str(), OK.length(), 0);
        else
            send(mgt_sock, BAD_REQUEST.c_str(), BAD_REQUEST.length(), 0);
    }
    zmq_close(requester);
    zmq_ctx_destroy(context);
    return;
}

//****************************************************************************************
// function: handle_config_request
//
// description: Tell Control Plane to re-read configuration file.
//****************************************************************************************
static void handle_config_request(int mgt_sock)
{
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);

    char buffer[64];
    sprintf(buffer, "tcp://localhost:%d", TCPLANE_SERVICE);
    zmq_connect(requester, buffer);

    sprintf(buffer, "%d", rereadCplaneConfig);
    zmq_send(requester, buffer, strlen(buffer), 0);
    size_t size = timed_read(requester, buffer, sizeof(buffer), READ_TIMEOUT);
    if(size && (strcmp(buffer, "END") == 0)) 
    {
        syslog(LOG_DEBUG, "%s", "reread config successful.");
        send(mgt_sock, OK.c_str(), OK.length(), 0);
    }
    else
        send(mgt_sock, BAD_REQUEST.c_str(), BAD_REQUEST.length(), 0);
    zmq_close(requester);
    zmq_ctx_destroy(context);
    return;
}
#endif
