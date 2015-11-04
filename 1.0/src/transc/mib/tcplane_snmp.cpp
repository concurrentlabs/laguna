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

#include <czmq.h>
#include <jansson.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "tcplane_snmp.h"
#include "evlog.h"

#define ESUCCESS 0

extern "C" int16_t edge_status(void *  pCntx);

extern "C" int tcPlaneSnmpGetTrSts(void * pCntx);

extern "C" int tcPlaneSnmpReLoadCfgYaml(void * pCntx);

extern "C" int tcPlaneSnmpReLoadSysYaml(void * pCntx);

extern "C" tc_snmpplane_qmsgtype_e tcPlaneSnmpReadPPToMibQ(void * pCntx,
    tc_qmsgtbl_comptomib_t * pMsg);

extern "C" tc_snmpplane_qmsgtype_e tcPlaneSnmpReadHPToMibQ(void * pCntx,
    tc_qmsgtbl_comptomib_t * pMsg);

extern "C" tc_snmpplane_qmsgtype_e tcPlaneSnmpReadPGToMibQ(void* pCntx,
    tc_qmsgtbl_comptomib_t*  pMsg);

extern "C" char tcUtilIPAddrtoAscii(void * addr, char * buffer, size_t size);

extern "C" void tcPlaneSnmpInitRes(void * context);

extern "C" unsigned char tcPlaneSnmpGetThreadExit(void* pCntx);

extern "C"  void tcPlaneSnmpLogString(void* pCntx,evlog_loglvl_e lvl, CHAR* msg);

extern "C" char * tcPlaneSnmpGetVersion(void * pCntx,char *  buffer, size_t size);

extern "C" void tcPlaneSnmpGetModeOfOperationEntry(void* pCntx,
                                U16 idx,CHAR* strMsg,U32 nstrMsg);

extern "C" U16 tcPlaneSnmpGetModeOfOperationTblSz(void* pCntx);

extern "C" void tcPlaneSnmpUpdateHealthSts(void* pCntx);

extern "C" void tcPlaneSnmpExit(void* pCntx);

using namespace std;

static string UTCTimeString(const time_t t);

static void split(const string & s, char c, vector<string> & v);

snmp_processor g_processor;

//************************************************************************************
// function: tcplane_snmp
//
// description: tcplane snmp thread routine.
//************************************************************************************
extern "C" void tcplane_snmp(void * context)
{
    time_t edge_probe_timer = time(NULL);
    time_t purge_timer      = time(NULL);
    zctx_t * zctx = zctx_new();
    void * agentx_sock = zsocket_new(zctx, ZMQ_REP);
    tc_snmpplane_qmsgtype_e sts = tcSnmpPlaneQMsgNotAvail;
    zmq_pollitem_t items [] = { { agentx_sock, 0, ZMQ_POLLIN, 0 } };
    zsocket_bind(agentx_sock, "tcp://*:%d", AGENTX_SERVICE);

    /* Sync Queues */
    tcPlaneSnmpInitRes(context);

    while(!zctx_interrupted)
    {
        if(tcPlaneSnmpGetThreadExit(context))
            break;
        sts = g_processor.read_queue(context);

        if(zmq_poll(items, 1, 100) > 0)
        {// poll agentx_sock in microseconds
            if(items[0].revents & ZMQ_POLLIN)
            {
                tcPlaneSnmpLogString(context,evLogLvlDebug, (char*) "Processing agent event.");
                char *s = zstr_recv(agentx_sock);
                if(s)
                {
                    /* 1.4.1 - uses free() instead of zstr_free() */
                    g_processor.handl_agentx_event(context, s, agentx_sock);
                    zstr_free(&s);
                }
            }
        }
        else
        {
            if(tcSnmpPlaneQMsgNotAvail == sts)
                usleep(1000);
        }

        if(difftime(time(NULL), purge_timer) > g_processor.purge_duration())
        {
            tcPlaneSnmpLogString(context,evLogLvlDebug, (char*) "Purging table entries.");
            g_processor.purge_tables();
            purge_timer = time(NULL);
        }

        if(difftime(time(NULL), edge_probe_timer) > g_processor.edge_probe_duration())
        {
            g_processor.read_mode(context);
            g_processor.set_mode(context);
            edge_probe_timer = time(NULL);
        }
    }
    zsocket_destroy(zctx, agentx_sock);
    zctx_destroy(&zctx);
    tcPlaneSnmpExit(context);
    return;
}

//************************************************************************************
//************************************************************************************
tc_snmpplane_qmsgtype_e snmp_processor::read_queue(void * pCntx)
{
    unsigned short          _i;
    tc_snmpplane_qmsgtype_e _sts[TRANSC_MIB_QUEUE_MAX];

    m_hunger_count = 0;
    for(_i=0;_i<TRANSC_MIB_QUEUE_MAX;_i++)
        _sts[_i] = tcSnmpPlaneQMsgNotAvail;
    tc_snmpplane_qmsgtype_e _retsts = tcSnmpPlaneQMsgNotAvail;

    while(1)
    {
        m_hunger_count++;
        /* Pull message from each component threads and process them
         * one thread at a time.
         */
        _sts[tcTRPlaneSnmpCompTypePktPrc] =
                tcPlaneSnmpReadPPToMibQ(pCntx, &m_queue[tcTRPlaneSnmpCompTypePktPrc]);
        _sts[tcTRPlaneSnmpCompTypeHttpPrc] =
                tcPlaneSnmpReadHPToMibQ(pCntx, &m_queue[tcTRPlaneSnmpCompTypeHttpPrc]);
        /* No msg available on all the threads, exit */
        if(_sts[tcTRPlaneSnmpCompTypePktPrc] != tcSnmpPlaneQMsgAvail &&
            _sts[tcTRPlaneSnmpCompTypeHttpPrc] != tcSnmpPlaneQMsgAvail)
        {
            _retsts = tcSnmpPlaneQMsgNotAvail;
            break;
        }
        else if(m_hunger_count == TC_SNMPPLANE_QHUNGER_MAX)
        {
            _retsts = tcSnmpPlaneQMsgAvail;
            break;
        }
        else
            _retsts = tcSnmpPlaneQMsgAvail;
        for(_i=0;_i<TRANSC_MIB_QUEUE_MAX;_i++)
        {
            /* Check to see which thread message needs to be serviced */
            if(_sts[_i] == tcSnmpPlaneQMsgAvail)
            {
                if(m_queue[_i].bIsRedir)
                {
                    ++m_redirect_count;
                    for(unsigned int i = 0; i < m_queue[_i].nHttpMsgInfo; ++i)
                    {
                        string domain(m_queue[_i].tHttpMsgInfo[i].strDomain);
                        string service(m_queue[_i].tHttpMsgInfo[i].strSvcName);
                        char buffer[64];
                        tcUtilIPAddrtoAscii(&(m_queue[_i].tHttpMsgInfo[i].tIpAaddr),
                            buffer, sizeof(buffer));
                        string client(buffer);
                        if(domain.length())
                            insert_or_update_table(m_domain_redirect_table, domain);
                        if(service.length())
                            insert_or_update_table(m_redirected_service_table, service);
                        if(client.length())
                            insert_or_update_table(m_client_redirect_table, client);
                    }
                }
                else
                {
                    ++m_traffic_count;
                    for(unsigned int i = 0; i < m_queue[_i].nHttpMsgInfo; ++i)
                    {
                        string domain(m_queue[_i].tHttpMsgInfo[i].strDomain);
                        string service(m_queue[_i].tHttpMsgInfo[i].strSvcName);
                        char buffer[64];
                        tcUtilIPAddrtoAscii(&(m_queue[_i].tHttpMsgInfo[i].tIpAaddr),
                            buffer, sizeof(buffer));
                        string client(buffer);
                        if(domain.length())
                            insert_or_update_table(m_domain_table, domain);
                        if(client.length())
                            insert_or_update_table(m_client_table, client);
                    }
                }
            }
        }
    }
    return _retsts;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::handl_agentx_event(
        void * pCntx, char * raw_request, void * sock)
{
    string request(raw_request);
    std::vector<string> v;

    split(request, '|', v);
    int32_t _req = atoi(v[0].c_str());

    switch(_req)
    {
        case STATUS:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved STATUS request.");
        zstr_sendf(sock, "%d", tcPlaneSnmpGetTrSts(pCntx));
        break;

        case TRAFFIC_COUNT:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved TRAFFIC_COUNT request.");
        zstr_sendf(sock, "%d", m_traffic_count);
        break;

        case REDIRECT_COUNT:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved REDIRECT_COUNT request.");
        zstr_sendf(sock, "%d", m_redirect_count);
        break;

        case DOMAIN_TABLE:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved DOMAIN_TABLE request.");
        transmit_table(m_domain_table, sock);
        break;

        case DOMAIN_REDIRECT_TABLE:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved DOMAIN_REDIRECT_TABLE request.");
        transmit_table(m_domain_redirect_table, sock);
        break;

        case VIDEO_TABLE:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved VIDEO_TABLE request.");
        transmit_table(m_video_table, sock);
        break;

        case VIDEO_REDIRECT_TABLE:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved VIDEO_REDIRECT_TABLE request.");
        transmit_table(m_video_redirect_table, sock);
        break;

        case CLIENT_TABLE:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved CLIENT_TABLE request.");
        transmit_table(m_client_table, sock);
        break;

        case CLIENT_REDIRECT_TABLE:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved CLIENT_REDIRECT_TABLE request.");
        transmit_table(m_client_redirect_table, sock);
        break;

        case CLIENT_TOP_DEVICE_TABLE:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved CLIENT_TOP_DEVICE_TABLE request.");
        transmit_table(m_client_top_device_table, sock);
        break;

        case READ_CPLANE_CONFIG:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved READ_CPLANE_CONFIG request.");
        if(tcPlaneSnmpReLoadCfgYaml(pCntx) != ESUCCESS)
            zmq_send(sock, "FAIL", 4, 0);
        else
            zmq_send(sock, "END", 3, 0);
        break;

        case REDIRECTED_SERVICE_TABLE:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Received REDIRECTED_SERVICE_TABLE request.");
        transmit_table(m_redirected_service_table, sock);
        break;

        case READ_CPLANE_SYSCONFIG:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Received READ_CPLANE_SYSCONFIG request.");
        if(tcPlaneSnmpReLoadSysYaml(pCntx) != ESUCCESS)
            zmq_send(sock, "FAIL", 4, 0);
        else
            zmq_send(sock, "END", 3, 0);
        break;

        case GET_MAX_TABLE_XMIT:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved GET_MAX_TABLE_XMIT request.");
        zstr_sendf(sock, "%d", m_max_table_xmit);
        break;

        case SET_MAX_TABLE_XMIT:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved SET_MAX_TABLE_XMIT request.");
        set_maxTableXmit(atoi(v[1].c_str()));
        zmq_send(sock, "END", 3, 0);
        break;

        case GET_EDGE_PROBE_DURATION:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved GET_EDGE_PROBE_DURATION request.");
        zstr_sendf(sock, "%d", (int32_t) m_edge_probe_duration);
        break;

        case SET_EDGE_PROBE_DURATION:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved SET_EDGE_PROBE_DURATION request.");
        set_edgeProbeDuration((double) atoi(v[1].c_str()));
        zmq_send(sock, "END", 3, 0);
        break;

        case GET_TABLE_PURGE_DURATION:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved GET_TABLE_PURGE_DURATION request.");
        zstr_sendf(sock, "%d", (int32_t) m_table_purge_duration);
        break;

        case SET_TABLE_PURGE_DURATION:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved SET_TABLE_PURGE_DURATION request.");
        set_tablePurgeDuration((double) atoi(v[1].c_str()));
        zmq_send(sock, "END", 3, 0);
        break;

        case START_TIME:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Received START_TIME request.");
        zstr_sendf(sock, "%s", UTCTimeString(m_start_time).c_str());
        break;

        case VERSION:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Received VERSION request.");
        {
            char buffer[64];
            tcPlaneSnmpGetVersion(pCntx, buffer, sizeof(buffer));
            zstr_sendf(sock, "%s", buffer);
        }
        break;

        case MODE:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Received MODE request.");
        for(int i = 0; i < tcPlaneSnmpGetModeOfOperationTblSz(pCntx); ++i)
            zstr_sendfm(sock, "%s", m_mode_table.strMsgTbl[i]);
        zmq_send(sock, "END", 3, 0);
        break;

        default:
        tcPlaneSnmpLogString(pCntx,evLogLvlDebug, (char*) "Recieved INVALID tcplane_agentx request.");
        zmq_send(sock, "END", 3, 0);
        break;
    }
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::transmit_table(table_t & table, void * sock)
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
    purge_table(m_domain_table);
    purge_table(m_domain_redirect_table);
    purge_table(m_client_table);
    purge_table(m_client_redirect_table);
    // purge_table(m_redirected_service_table);
    // purge_table(m_client_top_device_table);
    // purge_table(m_video_table);
    // purge_table(m_video_redirect_table);
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::purge_table(table_t & table)
{
    time_t now = time(NULL);
    for(auto it = table.begin(); it != table.end(); )
    {
        if(difftime(now, it->second.first) >= m_table_purge_duration)
            it = table.erase(it);
        else
            ++it;
    }
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::print_table(table_t & table)
{
    for(auto it = table.cbegin(); it != table.cend(); ++it)
        cout << it->first << '\t' << it->second.second << '\n';
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::read_mode(void * context)
{
    tcPlaneSnmpUpdateHealthSts(context);
    return;
}

//************************************************************************************
//************************************************************************************
void snmp_processor::set_mode(void * context)
{
    U16                             _i;
    CHAR                            _strMsg[TRANSC_SNMPPLANE_MOOMSG_LEN];
    U16                             _nTblSz;

    _nTblSz = tcPlaneSnmpGetModeOfOperationTblSz(context);
    for(_i=0;_i<_nTblSz;_i++)
    {
        tcPlaneSnmpGetModeOfOperationEntry(context,
                _i,_strMsg,sizeof(_strMsg));
        strncpy(m_mode_table.strMsgTbl[_i],_strMsg,sizeof(m_mode_table.strMsgTbl[_i])-1);
        m_mode_table.strMsgTbl[_i][sizeof(m_mode_table.strMsgTbl[_i])-1] = '\0';
        tcPlaneSnmpLogString(context,evLogLvlInfo, m_mode_table.strMsgTbl[_i]);
    }
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
static string UTCTimeString(const time_t t)
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
