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

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

#include "edge_status.h"
#include "tcplane_snmp.h"

using namespace std;

string UTCTimeString(const time_t t);

static void split(const string & s, char c, vector<string> & v);

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

    p.read_queue();
    while(!zctx_interrupted)
    {
        // cout << "top of tcplane_snmp loop...\n";

        zmq_poll(items, 1, 1000);
        if(items[0].revents & ZMQ_POLLIN)
        {
            cout << "Processing agent event.\n";
            char * s = zstr_recv(agentx_sock);
            p.handl_agentx_event(_pCntx, s, agentx_sock);
            zstr_free(&s);
        }

        //*** poll edges every thirty seconds ***
        if(loop_count == 30)
        {
            if(edge_status(_pCntx))
            {
                cout << "Edge status active mode.\n";
                // tcBkgrndSetRedirNode(_pCntx, ACTIVE);
                p.set_mode(_pCntx, ACTIVE);
            }
            else
            {
                cout << "Edge status monitor mode.\n";
                // tcBkgrndSetRedirNode(_pCntx, MONITOR);
                p.set_mode(_pCntx, MONITOR);
            }
            loop_count = 0;
        }
        else
            ++loop_count;
        // p.purge_tables();
    }
    zsocket_destroy (zctx, agentx_sock);
    zctx_destroy(&zctx);
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::read_queue(void)
{
    for(int i = 0; i < 5; ++i)
    {
        struct in_addr ip_addr;
        ip_addr.s_addr = rand();
        string ip = inet_ntoa(ip_addr);
        insert_or_update_table(client_table, ip);
        insert_or_update_table(client_redirect_table, ip);
        insert_or_update_table(domain_table, ip);
        insert_or_update_table(domain_redirect_table, ip);
        insert_or_update_table(redirected_service_table, ip);
    }
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::handl_agentx_event(
        tc_gd_thread_ctxt_t* pCntx, char * raw_request, void * sock)
{
    string request(raw_request);
    std::vector<string> v;

    split(request, '|', v);
    int32_t _req = atoi(v[0].c_str());

    switch(_req)
    {
        case STATUS:
        cout << "Recieved STATUS request." << endl;
        zstr_sendf(sock, "%d", tcBkgrndGetTrSts(pCntx));
        break;

        case TRAFFIC_COUNT:
        cout << "Recieved TRAFFIC_COUNT request." << endl;
        zstr_sendf(sock, "%d", m_traffic_count);
        break;

        case REDIRECT_COUNT:
        cout << "Recieved REDIRECT_COUNT request." << endl;
        zstr_sendf(sock, "%d", m_redirect_count);
        break;

        case DOMAIN_TABLE:
        cout << "Recieved DOMAIN_TABLE request." << endl;
        transmit_table(domain_table, sock);
        break;

        case DOMAIN_REDIRECT_TABLE:
        cout << "Recieved DOMAIN_REDIRECT_TABLE request." << endl;
        transmit_table(domain_redirect_table, sock);
        break;

        case VIDEO_TABLE:
        cout << "Recieved VIDEO_TABLE request." << endl;
        zmq_send(sock, "END", 3, 0);
        break;

        case VIDEO_REDIRECT_TABLE:
        cout << "Recieved VIDEO_REDIRECT_TABLE request." << endl;
        zmq_send(sock, "END", 3, 0);
        break;

        case CLIENT_TABLE:
        cout << "Recieved CLIENT_TABLE request." << endl;
        transmit_table(client_table, sock);
        break;

        case CLIENT_REDIRECT_TABLE:
        cout << "Recieved CLIENT_REDIRECT_TABLE request." << endl;
        transmit_table(client_redirect_table, sock);
        break;

        case CLIENT_TOP_DEVICE_TABLE:
        cout << "Recieved CLIENT_TOP_DEVICE_TABLE request." << endl;
        zmq_send(sock, "END", 3, 0);
        break;

        case READ_CPLANE_CONFIG:
        cout << "Recieved READ_CPLANE_CONFIG request." << endl;
        if(tcBkgrndReLoadCfg(pCntx) != ESUCCESS)
            zmq_send(sock, "FAIL", 4, 0);
        else
            zmq_send(sock, "END", 3, 0);
        break;

        case REDIRECTED_SERVICE_TABLE:
        cout << "Recieved REDIRECTED_SERVICE_TABLE request." << endl;
        transmit_table(redirected_service_table, sock);
        break;

        case GET_MAX_TABLE_XMIT:
        cout << "Recieved GET_MAX_TABLE_XMIT request." << endl;
        zstr_sendf(sock, "%d", m_max_table_xmit);
        break;

        case SET_MAX_TABLE_XMIT:
        cout << "Recieved SET_MAX_TABLE_XMIT request." << endl;
        set_maxTableXmit(atoi(v[1].c_str()));
        zmq_send(sock, "END", 3, 0);
        break;

        case GET_EDGE_PROBE_DURATION:
        cout << "Recieved GET_EDGE_PROBE_DURATION request." << endl;
        zstr_sendf(sock, "%d", (int32_t) m_edge_probe_duration);
        break;

        case SET_EDGE_PROBE_DURATION:
        cout << "Recieved SET_EDGE_PROBE_DURATION request." << endl;
        set_edgeProbeDuration((double) atoi(v[1].c_str()));
        zmq_send(sock, "END", 3, 0);
        break;

        case GET_TABLE_PURGE_DURATION:
        cout << "Recieved GET_TABLE_PURGE_DURATION request." << endl;
        zstr_sendf(sock, "%d", (int32_t) m_table_purge_duration);
        break;

        case SET_TABLE_PURGE_DURATION:
        cout << "Recieved SET_TABLE_PURGE_DURATION request." << endl;
        set_tablePurgeDuration((double) atoi(v[1].c_str()));
        zmq_send(sock, "END", 3, 0);
        break;

        case START_TIME:
        cout << "Recieved START_TIME request." << endl;
        zstr_sendf(sock, "%s", UTCTimeString(m_start_time).c_str());
        break;

        case VERSION:
        cout << "Recieved VERSION request." << endl;
        zstr_sendf(sock, "%s", m_version.c_str());
        break;

        case MODE:
        cout << "Recieved MODE request." << endl;
        //*** simulation for transmission of mode array. ***
        for(int i = 0; i < 5; ++i)
        {
            ostringstream oss;
            struct in_addr ip_addr;
            ip_addr.s_addr = rand();
            oss << "Link:" << i << '/' << string(inet_ntoa(ip_addr)) <<
               "/eth" << i << ":up-ethx:up/active";
            zstr_sendfm(sock, "%s", oss.str().c_str());
        }
        zmq_send(sock, "END", 3, 0);
        break;

        default:
        cout << "Recieved INVALID tcplane_agentx request." << endl;
        zmq_send(sock, "END", 3, 0);
        break;
    }
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::transmit_table(table_t & table, void *sock)
{
    typedef pair<string, unsigned int> xmit_item_t;
    vector<xmit_item_t> xmit_vec;

    for(auto it = table.cbegin(); it != table.cend(); ++it)
        xmit_vec.push_back(make_pair(it->first, it->second.second));

    auto comp = [](const xmit_item_t & a, const xmit_item_t & b)
    { return a.second > b.second; };
    stable_sort(xmit_vec.begin(), xmit_vec.end(), comp);

    auto xmit_row = [&](const xmit_item_t & row)
    { zstr_sendfm(sock, "%s|%d", row.first.c_str(), row.second); };

    xmit_vec.size() > m_max_table_xmit ?
    for_each(xmit_vec.cbegin(), xmit_vec.cbegin() + m_max_table_xmit, xmit_row) :
    for_each(xmit_vec.cbegin(), xmit_vec.cend(), xmit_row);

    zmq_send(sock, "END", 3, 0);
    return;
}

//************************************************************************************
//************************************************************************************
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

//************************************************************************************
//************************************************************************************
void snmp_processor::purge_tables(void)
{
    cout << "Enter purge_tables.\n";
    purge_table(client_table);
    purge_table(client_redirect_table);
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::purge_table(table_t & table)
{
    cout << "Enter purge_table.\n";
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

//************************************************************************************
//************************************************************************************
void snmp_processor::print_table(table_t & table)
{
    for(auto it = table.cbegin(); it != table.cend(); ++it)
    {
        if(it->second.second > 1)
            cout << it->first << '\t' << it->second.second << '\n';
    }
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::set_mode(void * context, const unsigned short m)
{
    return;
}

//************************************************************************************
// function: split
//
// description: split string into vector based on char c.
//************************************************************************************
static void split(const string & s, char c, vector<string> & v)
{
    string::size_type i = 0;
    string::size_type j = s.find(c);

    if(j == string::npos) { v.push_back(s); return; }

    while(j != string::npos)
    {
        v.push_back(s.substr(i, j - i));
        i = ++j;
        j = s.find(c, j);
        if (j == string::npos)
            v.push_back(s.substr(i, s.length()));
    }
    return;
}

//****************************************************************************
// function: UTCTimeString
//
// description: returns time_t as W3C UTC time string.
//****************************************************************************
string UTCTimeString(const time_t t)
{
    ostringstream oss;
    struct tm * ptm = gmtime(&t);

    oss        << setw(4) << setfill('0') << ptm->tm_year + 1900
        << '-' << setw(2) << setfill('0') << ptm->tm_mon + 1
        << '-' << setw(2) << setfill('0') << ptm->tm_mday
        << 'T' << setw(2) << setfill('0') << ptm->tm_hour
        << ':' << setw(2) << setfill('0') << ptm->tm_min
        << ':' << setw(2) << setfill('0') << ptm->tm_sec
        << 'Z';
    return oss.str();
}
