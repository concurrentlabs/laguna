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

#include <string>

#include "../util_source/io_rtns.h"
#include "tcache_agentx.h"

#define HANDLER_NAME "tcPlaneVersion"

using namespace std;

static Netsnmp_Node_Handler tcPlaneVersion_handler;

static void get_tcPlaneVersion(string & s);

static void delayed_response(unsigned int clientreg, void * clientarg);

//********************************************************************************
// Function: init_tcPlaneVersion
//
// Description: Control Plane Version initialization for mib.
//********************************************************************************
bool init_tcPlaneVersion(void)
{
    syslog(LOG_DEBUG, "Enter: %s.", __FUNCTION__);

    static oid statusOID[] = { 1, 3, 6, 1, 4, 1, 1457, 4, 1, 1, 17 };

    netsnmp_handler_registration * reg{};
    reg = netsnmp_create_handler_registration(HANDLER_NAME,
        tcPlaneVersion_handler, statusOID, OID_LENGTH(statusOID),
        HANDLER_CAN_RONLY);
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
// Function: tcPlaneVersion_handler
//
// Description: Retrieve Control Plane Version.
//********************************************************************************
static int tcPlaneVersion_handler(netsnmp_mib_handler * handler,
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

    switch(cache->reqinfo->mode)
    {
        case MODE_GET:
        case MODE_GETNEXT:
        {
            string s;
            get_tcPlaneVersion(s);
            snmp_set_var_typed_value(cache->requests->requestvb,
                ASN_OCTET_STR, (unsigned char*) s.c_str(), s.size());
        }
        break;
    }
    netsnmp_free_delegated_cache(cache);

    syslog(LOG_DEBUG, "Exit: delayed_response, %s.", HANDLER_NAME); 
    syslog(LOG_DEBUG,"%s",
        "===============================================================");
    return;
}

//********************************************************************************
// Function: get_tcPlaneVersion
//
// Description: Retrieve Version value from Control Plane.
//********************************************************************************
static void get_tcPlaneVersion(string & s)
{
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket(context, ZMQ_REQ);

    int32_t timeo{};
    zmq_setsockopt(requester, ZMQ_LINGER, (void*) &timeo, sizeof(timeo));

    char buffer[64];
    sprintf(buffer, "tcp://localhost:%d", TCPLANE_SERVICE);
    zmq_connect(requester, buffer);

    sprintf(buffer, "%d", version);
    zmq_send(requester, buffer, strlen(buffer), 0);

    s.clear();
    size_t size = timed_read(requester, buffer, sizeof(buffer), READ_TIMEOUT);
    if(size)
        s = buffer;

    zmq_close(requester);
    zmq_ctx_destroy(context);
    return;
}
