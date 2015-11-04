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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <czmq.h>

#include "../util_source/io_rtns.h"
#include "tcache_agentx.h"

#define HANDLER_NAME "tablePurgeDuration"

static Netsnmp_Node_Handler tablePurgeDuration_handler;

static bool set_tablePurgeDuration(uint32_t duration);

static uint32_t get_tablePurgeDuration(void);

static void delayed_response(unsigned int clientreg, void * clientarg);

//********************************************************************************
// Function: init_tablePurgeDuration
//
// Description: Control Plane tablePurgeDuration initialization for mib.
//********************************************************************************
bool init_tablePurgeDuration(void)
{
    syslog(LOG_DEBUG, "Enter: %s.", __FUNCTION__);

    static oid statusOID[] = { 1, 3, 6, 1, 4, 1, 1457, 4, 1, 1, 15 };

    netsnmp_handler_registration * reg{};
    reg = netsnmp_create_handler_registration(HANDLER_NAME,
        tablePurgeDuration_handler, statusOID, OID_LENGTH(statusOID),
        HANDLER_CAN_RWRITE);
    if(!reg)
    {
        syslog(LOG_CRIT, "ERROR: netsnmp_create_handler_registration.");
        return false;
    }
    if(netsnmp_register_scalar(reg) != SNMPERR_SUCCESS)
    {
        syslog(LOG_CRIT, "ERROR: netsnmp_regester_scalar.");
        return false;
    }
    syslog(LOG_DEBUG, "Exit: %s.", __FUNCTION__);
    return true;
}

//********************************************************************************
// Function: tablePurgeDuration_handler
//
// Description: Retrieve or Set Table Purge Duration Value.
//********************************************************************************
static int tablePurgeDuration_handler(netsnmp_mib_handler * handler,
    netsnmp_handler_registration * reginfo,
    netsnmp_agent_request_info * reqinfo,
    netsnmp_request_info * requests)
{
    syslog(LOG_DEBUG, "Enter: %s", __FUNCTION__);

    syslog(LOG_DEBUG, "handler, continuing delayed request, mode = %d.",
        reqinfo->mode);

    requests->delegated = 1;

    snmp_alarm_register(0, 0, delayed_response,
        (void *) netsnmp_create_delegated_cache(handler, reginfo, reqinfo,
        requests, NULL));

    syslog(LOG_DEBUG, "Exit: %s", __FUNCTION__);
    return SNMP_ERR_NOERROR;
}

//********************************************************************************
//********************************************************************************
static void delayed_response(unsigned int clientreg, void * clientarg)
{
    syslog(LOG_DEBUG, "Enter: delayed_response, %s.", HANDLER_NAME); 

    netsnmp_delegated_cache *cache = (netsnmp_delegated_cache *) clientarg;
    if(!netsnmp_handler_check_cache(cache))
    {
        syslog(LOG_ERR, "%s", "illegal call to return delayed response.");
        return;
    }
    syslog(LOG_DEBUG, "delayed_instance, continuing delayed request, mode = %d.",
        cache->reqinfo->mode);

    cache->requests->delegated = 0;

    uint32_t tablePurgeDuration{};
    uint32_t * dupCache{};
    switch(cache->reqinfo->mode)
    {
        case MODE_GET:
        case MODE_GETNEXT:
        tablePurgeDuration = get_tablePurgeDuration();
        snmp_set_var_typed_integer(cache->requests->requestvb, ASN_UNSIGNED,
            tablePurgeDuration);
        break;

        case MODE_SET_RESERVE1:
        if(cache->requests->requestvb->type != ASN_UNSIGNED)
        {
            syslog(LOG_DEBUG, "%s: SNMP_ERR_WRONGTYPE", HANDLER_NAME);
            netsnmp_set_request_error(cache->reqinfo, cache->requests, SNMP_ERR_WRONGTYPE);
            netsnmp_free_delegated_cache(cache);
            return;
        }
        break;

        case MODE_SET_RESERVE2:
        memdup((uint8_t **) &dupCache,
            (uint8_t *) &tablePurgeDuration, sizeof(tablePurgeDuration));
        if(!dupCache)
        {
            syslog(LOG_DEBUG, "%s: SNMP_ERR_RESOURCEUNAVAILABLE", HANDLER_NAME);
            netsnmp_set_request_error(cache->reqinfo, cache->requests,
                SNMP_ERR_RESOURCEUNAVAILABLE);
            netsnmp_free_delegated_cache(cache);
        }
        netsnmp_request_add_list_data(cache->requests,
            netsnmp_create_data_list(HANDLER_NAME, dupCache, free));
        break;

        case MODE_SET_ACTION:
         tablePurgeDuration =  *(cache->requests->requestvb->val.integer);
        (void) set_tablePurgeDuration(tablePurgeDuration);
        syslog(LOG_DEBUG, "%s: updated tablePurgeDuration: %d", HANDLER_NAME,
            tablePurgeDuration);
        break;

        case MODE_SET_UNDO:
        tablePurgeDuration = *((uint32_t *) netsnmp_request_get_list_data(cache->requests,
            HANDLER_NAME));
        break;

        case MODE_SET_COMMIT:
        case MODE_SET_FREE:
        break;
    }
    netsnmp_free_delegated_cache(cache);

    syslog(LOG_DEBUG, "Exit: delayed_response, %s.", HANDLER_NAME); 
    syslog(LOG_DEBUG,"%s",
        "===============================================================");
    return;
}

//********************************************************************************
// Function: get_tablePurgeDuration
//
// Description: Retrieve get_tablePurgeDuration value from Control Plane.
//********************************************************************************
static uint32_t get_tablePurgeDuration(void)
{
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket(context, ZMQ_REQ);

    int32_t timeo{};
    zmq_setsockopt(requester, ZMQ_LINGER, (void*) &timeo, sizeof(timeo));

    char buffer[64];
    sprintf(buffer, "tcp://localhost:%d", TCPLANE_SERVICE);
    zmq_connect(requester, buffer);

    sprintf(buffer, "%d", getTablePurgeDuration);
    zmq_send(requester, buffer, strlen(buffer), 0);

    uint32_t u{};
    size_t size = timed_read(requester, buffer, sizeof(buffer), READ_TIMEOUT);
    if(size)
        u = (uint32_t) atoi(buffer);
    zmq_close(requester);
    zmq_ctx_destroy(context);
    return u;
}

//********************************************************************************
// Function: set_tablePurgeDuration
//
// Description: Set set_tablePurgeDuration value int Control Plane.
//********************************************************************************
static bool set_tablePurgeDuration(uint32_t duration)
{
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);

    int32_t timeo{};
    zmq_setsockopt(requester, ZMQ_LINGER, (void*) &timeo, sizeof(timeo));

    char buffer[64];
    sprintf(buffer, "tcp://localhost:%d", TCPLANE_SERVICE);
    zmq_connect(requester, buffer);

    sprintf(buffer, "%d|%u", setTablePurgeDuration, duration);
    zmq_send(requester, buffer, strlen(buffer), 0);

    bool ret = true;
    size_t size = timed_read(requester, buffer, sizeof(buffer), READ_TIMEOUT);
    if(size)
        ret = false;
    zmq_close(requester);
    zmq_ctx_destroy(context);
    return ret;
}
