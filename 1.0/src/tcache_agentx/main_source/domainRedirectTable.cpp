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

#include <syslog.h>
#include <czmq.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/mib_modules.h>

#include <vector>

#include "../util_source/io_rtns.h"
#include "tcache_agentx.h"
#include "domainRedirectTable.h"

#define HANDLER_NAME "domainRedirectTable"

using namespace std;

static vector<domainRedirectTable_t> g_domainRedirectTable;

static Netsnmp_Node_Handler handler;

static netsnmp_variable_list * first_datapt(void ** loop_context,
    void ** data_context, netsnmp_variable_list * put_index_data, 
    netsnmp_iterator_info * data);

static netsnmp_variable_list * next_datapt(void ** loop_context,
    void ** data_context, netsnmp_variable_list * put_index_data,
    netsnmp_iterator_info * data);

static void load_table(void);

static void delayed_response(unsigned int clientreg, void * clientarg);

//********************************************************************************
// Function: init_domainRedirectTable
//
// Description: domainRedirectTable initialization for mib.
//********************************************************************************
bool init_domainRedirectTable(void)
{
    syslog(LOG_DEBUG, "Enter: %s", __FUNCTION__); 

    static oid domainRedirectTableOID[] = { 1, 3, 6, 1, 4, 1, 1457, 4, 1, 1, 6 };

    netsnmp_handler_registration * reg{};
    reg = netsnmp_create_handler_registration(HANDLER_NAME,
        handler, domainRedirectTableOID, OID_LENGTH(domainRedirectTableOID),
        HANDLER_CAN_RONLY);

    netsnmp_table_registration_info * table_info{};
    table_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);
    if(!table_info)
    {
        syslog(LOG_CRIT, "ERROR: SNMP_MALLOC_TYPEDEF.");
        return false;
    }
    netsnmp_table_helper_add_indexes(table_info, ASN_INTEGER, 0);
    table_info->min_column = COLUMN_DOMAIN_REDIRECT;
    table_info->max_column = COLUMN_DOMAIN_REDIRECT_COUNT;

    netsnmp_iterator_info *iinfo{};
    iinfo = SNMP_MALLOC_TYPEDEF(netsnmp_iterator_info);
    iinfo->get_first_data_point = (Netsnmp_First_Data_Point *) first_datapt;
    iinfo->get_next_data_point = (Netsnmp_Next_Data_Point *) next_datapt;
    iinfo->table_reginfo = table_info;

    if(netsnmp_register_table_iterator(reg, iinfo) != SNMPERR_SUCCESS)
    {
        syslog(LOG_CRIT, "ERROR: netsnmp_register_table_iterator.");
        return false;
    }
    syslog(LOG_DEBUG, "Exit: %s", __FUNCTION__); 
    return true;
}

//********************************************************************************
// Function: first_datapt
//
// Description: Returns first data point of domainRedirectTable.
//********************************************************************************
static netsnmp_variable_list *
first_datapt(void ** loop_context, void **data_context,
    netsnmp_variable_list * put_index_data, netsnmp_iterator_info * data)
{
    load_table();
    if(!g_domainRedirectTable.size())
    {
        *data_context = (void*) 0;
        *loop_context = (void*) 0;
        return NULL;
    }
    *loop_context = (void*) g_domainRedirectTable.data();
    return next_datapt(loop_context, data_context, put_index_data, data);
}

//********************************************************************************
// Function: next_datapt
//
// Description: Returns next data point of domainRedirectTable.
//********************************************************************************
static netsnmp_variable_list *
next_datapt(void ** loop_context, void ** data_context,
    netsnmp_variable_list * put_index_data, netsnmp_iterator_info * data)
{
    if(!g_domainRedirectTable.size())
        return NULL;

    domainRedirectTable_t *  pd = (domainRedirectTable_t *) *loop_context;
    domainRedirectTable_t * end = g_domainRedirectTable.data();
    end += g_domainRedirectTable.size();
    if(pd == end)
        return NULL;

    netsnmp_variable_list *idx = put_index_data;
    snmp_set_var_typed_integer(idx, ASN_INTEGER, pd->domainRedirectIdx);
    *data_context = (void*) pd;
    *loop_context = (void*) ++pd;
    return put_index_data;
}

//********************************************************************************
// Function: handler
//
// Description: Handle domainRedirectTable get request.
//********************************************************************************
static int handler(netsnmp_mib_handler * handler,
    netsnmp_handler_registration * reginfo, netsnmp_agent_request_info * reqinfo,
    netsnmp_request_info * requests)
{
    syslog(LOG_DEBUG, "Enter: handler, %s.", HANDLER_NAME); 
    syslog(LOG_DEBUG, "handler, continuing delayed request, mode = %d.",
        reqinfo->mode);

    requests->delegated = 1;
    snmp_alarm_register(0, 0, delayed_response,
        (void *) netsnmp_create_delegated_cache(handler, reginfo, reqinfo,
        requests, NULL));

    syslog(LOG_DEBUG, "Exit: handler, %s.", HANDLER_NAME); 
    return SNMP_ERR_NOERROR;
}

//********************************************************************************
// Function: delayed_response
//
// Description: handle table query in callback.
//********************************************************************************
static void delayed_response(unsigned int clientreg, void * clientarg)
{
    syslog(LOG_DEBUG, "Enter: delayed_response, %s.", HANDLER_NAME); 

    netsnmp_delegated_cache *cache = (netsnmp_delegated_cache *) clientarg;
    if(!netsnmp_handler_check_cache(cache))
    {
        syslog(LOG_ERR, "%s %s: bad_cache.", HANDLER_NAME, __FUNCTION__);
        return;
    }
    syslog(LOG_DEBUG, "delayed_instance, continuing delayed request, mode = %d.",
        cache->reqinfo->mode);

    cache->requests->delegated = 0;

    if(cache->reqinfo->mode != MODE_GET && cache->reqinfo->mode != MODE_GETNEXT)
        return;

    for(netsnmp_request_info * request = cache->requests; request;
        request = request->next)
    {
        domainRedirectTable_t * d =
            (domainRedirectTable_t *) netsnmp_extract_iterator_context(request);
        if(!d)
        {
            netsnmp_set_request_error(cache->reqinfo, request, SNMP_NOSUCHINSTANCE);
            g_domainRedirectTable.clear();
            continue;
        }

        netsnmp_table_request_info *table_info = netsnmp_extract_table_info(request);
        if(!table_info)
        {
            syslog(LOG_CRIT, "ERROR: netsnmp_tdata_extract_table_info");
            continue;
        }

        switch (table_info->colnum)
        {
            case COLUMN_DOMAIN_REDIRECT:
            snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR,
                (unsigned char*) d->domainRedirect, strlen(d->domainRedirect));
            break;

            case COLUMN_DOMAIN_REDIRECT_COUNT:
            snmp_set_var_typed_integer(request->requestvb, ASN_COUNTER,
                d->domainRedirectCount);
            break;

            default:
            netsnmp_set_request_error(cache->reqinfo, request, SNMP_NOSUCHOBJECT);
        }
    }
    netsnmp_free_delegated_cache(cache);

    syslog(LOG_DEBUG, "Exit: delayed_response, %s.", HANDLER_NAME); 
    syslog(LOG_DEBUG,"%s",
        "===============================================================");
    return;
}

//********************************************************************************
// Function: load_table
//
// Description: Pull elements from tcache_plane into domainRedirectTable.
//********************************************************************************
static void load_table(void)
{
    if(g_domainRedirectTable.size())
        return;

    void *context = zmq_ctx_new ();
    void *requester = zmq_socket(context, ZMQ_REQ);

    int32_t timeo{};
    zmq_setsockopt(requester, ZMQ_LINGER, (void*) &timeo, sizeof(timeo));

    char buffer[64];
    sprintf(buffer, "tcp://localhost:%d", TCPLANE_SERVICE);
    zmq_connect(requester, buffer);

    sprintf(buffer, "%d", domainRedirectTable);
    zmq_send(requester, buffer, strlen(buffer), 0);

    int idx = 0;
    while(true)
    {
        size_t size = timed_read(requester, buffer, sizeof(buffer), READ_TIMEOUT);
        if(!size)
            break;
        if(strcmp(buffer, "END") == 0)
            break;

        char * key, * value;
        key = strtok_r(buffer, "|", &value);
        if(value)
        {
            domainRedirectTable_t d;
            d.domainRedirectIdx = idx++;
            strcpy(d.domainRedirect, key);
            d.domainRedirectCount = atoi(value);
            g_domainRedirectTable.push_back(d);
        }
        size = sizeof(size_t);
        int rcvmore{};
        zmq_getsockopt(requester, ZMQ_RCVMORE, (void*) &rcvmore, &size);
        if(!rcvmore)
            break;
    }
    zmq_close(requester);
    zmq_ctx_destroy(context);
    return;
}
