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

typedef unsigned int uint;

#include <czmq.h>
#include <jansson.h>

#include "edge_status.h"
#include "tcplane_snmp.h"

static int32_t g_traffic_count  = 0;
static int32_t g_redirect_count = 0;

static zhash_t * g_domain_table              = NULL;
static zhash_t * g_domain_redirect_table     = NULL;
static zhash_t * g_video_table               = NULL;
static zhash_t * g_video_redirect_table      = NULL;
static zhash_t * g_client_table              = NULL;
static zhash_t * g_client_redirect_table     = NULL;
static zhash_t * g_client_top_device_table   = NULL;
static zhash_t * g_redirected_service_table  = NULL;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int init_snmp_tables(void);
static int transmit_table_row(const char * key, void * item, void * sock);

static void handl_agentx_event(
        tc_gd_thread_ctxt_t* pCntx, int request, void * sock);
static void table_foreach(zhash_t * table, void *sock);
static void increment_table(zhash_t * table, char * key);
static void update_table(zhash_t * table, char * key, unsigned long value);

//************************************************************************************
// function: tcplane_snmp
//
// description: tcplane snmp thread routine.
//************************************************************************************
void * tcplane_snmp(void * context)
{
    uint16_t loop_count = 0;
    tc_gd_thread_ctxt_t* _pCntx  =
            (tc_gd_thread_ctxt_t*)context;
    zctx_t * zctx = zctx_new();
    void * agentx_sock = zsocket_new(zctx, ZMQ_REP);
    zmq_pollitem_t items [] = { { agentx_sock, 0, ZMQ_POLLIN, 0 } };
    zsocket_bind(agentx_sock, "tcp://*:%d", AGENTX_SERVICE);
    init_snmp_tables();

    while(!zctx_interrupted)
    {
        zmq_poll(items, 1, 1000);
        if(items[0].revents & ZMQ_POLLIN)
        {
            evLogTraceSys(evLogLvlDebug, "%s", "Processing agent event.");
            char * s = zstr_recv(agentx_sock);
            int request = atoi(s);
            handl_agentx_event(_pCntx,request, agentx_sock);
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
    }
    zsocket_destroy (zctx, agentx_sock);
    zctx_destroy(&zctx);
    return (void*)0;
}

//************************************************************************************
//************************************************************************************
void increment_traffic_count(void)
{
    pthread_mutex_lock(&lock);
    ++g_traffic_count;
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void update_traffic_count(unsigned long value)
{
    pthread_mutex_lock(&lock);
    g_traffic_count = value;
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_redirect_count(void)
{
    pthread_mutex_lock(&lock);
    ++g_redirect_count;
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void update_redirect_count(unsigned long value)
{
    pthread_mutex_lock(&lock);
    g_redirect_count = value;
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_domainTable(char* key)
{
    pthread_mutex_lock(&lock);
    increment_table(g_domain_table, key);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_clientdomainTable(char* keyclient, char * keydomain)
{
    pthread_mutex_lock(&lock);
    ++g_traffic_count;
    increment_table(g_client_table, keyclient);
    increment_table(g_domain_table, keydomain);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void update_domainTable(char * key, unsigned long value)
{
    pthread_mutex_lock(&lock);
    update_table(g_domain_table, key, value);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_domainRedirectTable(char * key)
{
    pthread_mutex_lock(&lock);
    increment_table(g_domain_redirect_table, key);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void update_domainRedirectTable(char * key, unsigned long value)
{
    pthread_mutex_lock(&lock);
    update_table(g_domain_redirect_table, key, value);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_videoTable(char * key)
{
    pthread_mutex_lock(&lock);
    increment_table(g_video_table, key);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void update_videoTable(char * key, unsigned long value)
{
    pthread_mutex_lock(&lock);
    update_table(g_video_table, key, value);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_videoRedirectTable(char * key)
{
    pthread_mutex_lock(&lock);
    increment_table(g_video_redirect_table, key);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void update_videoRedirectTable(char * key, unsigned long value)
{
    pthread_mutex_lock(&lock);
    update_table(g_video_redirect_table, key, value);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_clientTable(char * key)
{
    pthread_mutex_lock(&lock);
    increment_table(g_client_table, key);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void update_clientTable(char * key, unsigned long value)
{
    pthread_mutex_lock(&lock);
    update_table(g_client_table, key, value);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_clientRedirectTable(char * key)
{
    pthread_mutex_lock(&lock);
    increment_table(g_client_redirect_table, key);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void update_clientRedirectTable(char * key, unsigned long value)
{
    pthread_mutex_lock(&lock);
    update_table(g_client_redirect_table, key, value);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_clientTopDeviceTable(char * key)
{
    pthread_mutex_lock(&lock);
    increment_table(g_client_top_device_table, key);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void update_clientTopDeviceTable(char * key, unsigned long value)
{
    pthread_mutex_lock(&lock);
    update_table(g_client_top_device_table, key, value);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
int svcredirect_init_table(void)
{
    if(g_redirected_service_table)
        zhash_destroy(&g_redirected_service_table);
    g_redirected_service_table = zhash_new();
    if (g_redirected_service_table)
        return 0;
    else
        return -1;
}

//************************************************************************************
//************************************************************************************
void destroy_snmp_tables(void)
{
    if(g_domain_table)
        zhash_destroy(&g_domain_table);
    if(g_domain_redirect_table)
        zhash_destroy(&g_domain_redirect_table);
    if(g_video_table)
        zhash_destroy(&g_video_table);
    if(g_video_redirect_table)
        zhash_destroy(&g_video_redirect_table);
    if(g_client_table)
        zhash_destroy(&g_client_table);
    if(g_client_redirect_table)
        zhash_destroy(&g_client_redirect_table);
    if(g_client_top_device_table)
        zhash_destroy(&g_client_top_device_table);
    if(g_redirected_service_table)
        zhash_destroy(&g_redirected_service_table);
    return;
}

//************************************************************************************
//************************************************************************************
void svcredirect_set_svc(char * strSvcName)
{
    unsigned long * pl = NULL;

    pl = (unsigned long*) zhash_lookup(g_redirected_service_table, strSvcName);
    if(pl)
        *pl  = 0;
    else
    {
        pl = (unsigned long*) malloc(sizeof(unsigned long));
        *pl = 0;
        zhash_insert(g_redirected_service_table, strSvcName, pl);
    }
    return;
}

//************************************************************************************
//************************************************************************************
void increment_redirectedServiceTable(char * strSvcName)
{
    pthread_mutex_lock(&lock);
    increment_table(g_redirected_service_table, strSvcName);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
void increment_redirectedTable(char* srcAddr,char* strHostName,char * strSvcName)
{
    pthread_mutex_lock(&lock);
    ++g_redirect_count;
    increment_table(g_domain_redirect_table, strHostName);
    increment_table(g_client_redirect_table, srcAddr);
    increment_table(g_redirected_service_table, strSvcName);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
static int init_snmp_tables(void)
{
    g_domain_table             = zhash_new();
    g_domain_redirect_table    = zhash_new();
    g_video_table              = zhash_new();
    g_video_redirect_table     = zhash_new();
    g_client_table             = zhash_new();
    g_client_redirect_table    = zhash_new();
    g_client_top_device_table  = zhash_new();
    return 1;
}

//************************************************************************************
//************************************************************************************
static void handl_agentx_event(
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
        pthread_mutex_lock(&lock);
        zstr_sendf(sock, "%d", g_traffic_count);
        pthread_mutex_unlock(&lock);
        break;

        case REDIRECT_COUNT:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved REDIRECT_COUNT request.");
        pthread_mutex_lock(&lock);
        zstr_sendf(sock, "%d", g_redirect_count);
        pthread_mutex_unlock(&lock);
        break;

        case DOMAIN_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved DOMAIN_TABLE request.");
        table_foreach(g_domain_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case DOMAIN_REDIRECT_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved DOMAIN_REDIRECT_TABLE request.");
        table_foreach(g_domain_redirect_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case VIDEO_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved VIDEO_TABLE request.");
        table_foreach(g_video_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case VIDEO_REDIRECT_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved VIDEO_REDIRECT_TABLE request.");
        table_foreach(g_video_redirect_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case CLIENT_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved CLIENT_TABLE request.");
        table_foreach(g_client_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case CLIENT_REDIRECT_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved CLIENT_REDIRECT_TABLE request.");
        table_foreach(g_client_redirect_table, sock);
        zmq_send(sock, "END", 3, 0);
        break;

        case CLIENT_TOP_DEVICE_TABLE:
        evLogTraceSys(evLogLvlDebug, "%s", "Recieved CLIENT_TOP_DEVICE_TABLE request.");
        table_foreach(g_client_top_device_table, sock);
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
        table_foreach(g_redirected_service_table, sock);
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
static void table_foreach(zhash_t * table, void *sock)
{
    pthread_mutex_lock(&lock);
    zhash_foreach(table, transmit_table_row, sock);
    pthread_mutex_unlock(&lock);
    return;
}

//************************************************************************************
//************************************************************************************
static int transmit_table_row(const char * key, void * item, void * sock)
{
    int32_t * p_count = (int32_t *) item;
    zstr_sendfm(sock, "%s|%d", key, *p_count);
    return 0;
}

//************************************************************************************
// function: increment_table
//
// description: increment hash table repersentating snmp table.
//************************************************************************************
static void increment_table(zhash_t * table, char * key)
{
    unsigned long * pl = NULL;

    pl = (unsigned long*) zhash_lookup(table, key);
    if(pl)
        ++(*pl);
    else
    {
        pl = (unsigned long*) malloc(sizeof(unsigned long));
        *pl = 1;
        zhash_insert(table, key, pl);
    }
    return;
}

//************************************************************************************
// function: update_table
//
// description: update hash table repersentating snmp table.
//************************************************************************************
static void update_table(zhash_t * table, char * key, unsigned long value)
{
    unsigned long * pl = NULL;

    pl = (unsigned long*) zhash_lookup(table, key);
    if(pl)
        *pl = value;
    else
    {
        pl = (unsigned long*) malloc(sizeof(unsigned long));
        *pl = value;
        zhash_insert(table, key, pl);
    }
    return;
}
