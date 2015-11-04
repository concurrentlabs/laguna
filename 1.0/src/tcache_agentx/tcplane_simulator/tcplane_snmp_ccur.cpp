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

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>

#include <czmq.h>
#include <jansson.h>

#include <iostream>
#include <list>
#include <string>
#include <utility>
#include <unordered_map>

#include "edge_status.h"
#include "tcplane_snmp.h"

#include "dummy_code.h"

using namespace std;

snmp_processor p;

//************************************************************************************
// function: tcplane_snmp
//
// description: tcplane snmp thread routine.
//************************************************************************************
extern "C" void tcplane_snmp(void * context)
{
    uint16_t loop_count = 0;
    tc_gd_thread_ctxt_t* _pCntx  =
            (tc_gd_thread_ctxt_t*)context;
    zctx_t * zctx = zctx_new();
    void * agentx_sock = zsocket_new(zctx, ZMQ_REP);
    zmq_pollitem_t items [] = { { agentx_sock, 0, ZMQ_POLLIN, 0 } };
    zsocket_bind(agentx_sock, "tcp://*:%d", AGENTX_SERVICE);

    while(!zctx_interrupted)
    {
        evLogTraceSys(evLogLvlDebug, "%s", "top of tcplane_snmp loop...");
        p.read_queue();

        zmq_poll(items, 1, 1000);
        if(items[0].revents & ZMQ_POLLIN)
        {
            evLogTraceSys(evLogLvlDebug, "%s", "Processing agent event.");
            char * s = zstr_recv(agentx_sock);
            int request = atoi(s);
            p.handl_agentx_event(_pCntx,request, agentx_sock);
            free(s);
        }

        //*** poll edges every thirty seconds ***
        if(loop_count == 30)
        {
            if(edge_status(_pCntx))
            {
                evLogTraceSys(evLogLvlDebug, "%s", "Edge status active mode.");
                tcBkgrndSetRedirNode(_pCntx, 1);
            }
            else
            {
                evLogTraceSys(evLogLvlDebug, "%s", "Edge status monitor mode.");
                tcBkgrndSetRedirNode(_pCntx, 0);
            }
            loop_count = 0;
        }
        else
            ++loop_count;
        p.purge_tables();
    }
    zsocket_destroy (zctx, agentx_sock);
    zctx_destroy(&zctx);
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::read_queue(void)
{
    // for(int i = 0; i < 500000; ++i)
    for(int i = 0; i < 500; ++i)
    {
        struct in_addr ip_addr;
        ip_addr.s_addr = rand();
        string ip = inet_ntoa(ip_addr);
        insert_or_update_table(client_table, ip);
        insert_or_update_table(client_redirect_table, ip);
    }
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::handl_agentx_event(
        tc_gd_thread_ctxt_t* pCntx, int request, void * sock)
{
    switch(request)
    {
        case STATUS:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved STATUS request.");
        zstr_sendf(sock, "%d", tcBkgrndGetTrSts(pCntx));
        break;

        case TRAFFIC_COUNT:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved TRAFFIC_COUNT request.");
        zstr_sendf(sock, "%d", traffic_count);
        break;

        case REDIRECT_COUNT:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved REDIRECT_COUNT request.");
        zstr_sendf(sock, "%d", redirect_count);
        break;

        case DOMAIN_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved DOMAIN_TABLE request.");
        transmit_table(domain_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case DOMAIN_REDIRECT_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved DOMAIN_REDIRECT_TABLE request.");
        transmit_table(domain_redirect_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case VIDEO_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved VIDEO_TABLE request.");
        transmit_table(video_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case VIDEO_REDIRECT_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved VIDEO_REDIRECT_TABLE request.");
        transmit_table(video_redirect_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case CLIENT_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved CLIENT_TABLE request.");
        transmit_table(client_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case CLIENT_REDIRECT_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved CLIENT_REDIRECT_TABLE request.");
        transmit_table(client_redirect_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case CLIENT_TOP_DEVICE_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved CLIENT_TOP_DEVICE_TABLE request.");
        transmit_table(client_top_device_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case READ_CPLANE_CONFIG:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved READ_CPLANE_CONFIG request.");
        if(tcBkgrndReLoadCfg(pCntx) != ESUCCESS)
            zmq_send(sock, "FAIL", 4, 0);
        else
            zmq_send(sock, "END", 3, 0);
        break;

        case REDIRECTED_SERVICE_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved REDIRECTED_SERVICE_TABLE request.");
        transmit_table(redirected_service_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        default:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved INVALID tcplane_agentx request.");
        zmq_send(sock, "END", 3, 0);
        break;
    }
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::transmit_table(table_t & table, void *sock)
{
    for(auto it = table.cbegin(); it != table.cend(); ++it)
        zstr_sendfm(sock, "%s|%d", it->first.c_str(), it->second.second);
    return;
}

//****************************************************************************
//****************************************************************************
void snmp_processor::insert_or_update_table(table_t & table, string & key)
{
    auto it = table.find(key);
    if(it != table.cend())
    {
        it->second.first = time(NULL);
        ++it->second.second;
        return;
    }
    table[key] = make_pair(time(NULL), 1);
    return;
}

//****************************************************************************
//****************************************************************************
void snmp_processor::purge_tables(void)
{
    evLogTraceSys(evLogLvlDebug, "%s", "Enter purge_tables.");
    purge_table(client_table);
    purge_table(client_redirect_table);
    return;
}

//****************************************************************************
//****************************************************************************
void snmp_processor::purge_table(table_t & table)
{
    evLogTraceSys(evLogLvlDebug, "%s", "Enter purge_table.");
    time_t now = time(NULL);
    list<string> expired;
    for(auto it = table.cbegin(); it != table.cend(); ++it)
    {
        if(difftime(now, it->second.first) > 10)
            expired.push_back(it->first);
    }
    for(auto it = expired.cbegin(); it != expired.cend(); ++it)
        table.erase(*it);
    return;
}

//****************************************************************************
//****************************************************************************
void snmp_processor::print_table(table_t & table)
{
    for(auto it = table.cbegin(); it != table.cend(); ++it)
    {
        if(it->second.second > 1)
            cout << it->first << '\t' << it->second.second << '\n';
    }
    return;
}
