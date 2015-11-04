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

#ifndef TCPLANE_SNMP_H
#define TCPLANE_SNMP_H 1

#include <ctime>
#include <unordered_map>

#include "lkfq.h"
#include "helper.h"
#include "tcconfig.h"
#include "tcqmsg.h"
#include "tcpktparse.h"

#define TC_SNMPPLANE_QHUNGER_MAX    1000
#define TRANSC_SNMPPLANE_MOOMSG_LEN 128

enum { STATUS, TRAFFIC_COUNT, REDIRECT_COUNT, DOMAIN_TABLE, DOMAIN_REDIRECT_TABLE,
    VIDEO_TABLE, VIDEO_REDIRECT_TABLE, CLIENT_TABLE, CLIENT_REDIRECT_TABLE,
    CLIENT_TOP_DEVICE_TABLE, READ_CPLANE_CONFIG, REDIRECTED_SERVICE_TABLE,
    GET_MAX_TABLE_XMIT, SET_MAX_TABLE_XMIT, GET_EDGE_PROBE_DURATION, SET_EDGE_PROBE_DURATION,
    GET_TABLE_PURGE_DURATION, SET_TABLE_PURGE_DURATION, START_TIME, VERSION, MODE,
    READ_CPLANE_SYSCONFIG = 500, AGENTX_SERVICE = 5555 };

enum { MONITOR = 0, ACTIVE };

enum tc_snmpplane_qmsgtype_e {tcSnmpPlaneQMsgAvail, tcSnmpPlaneQMsgNotAvail,
        tcSnmpPlaneQMsgError };

typedef std::pair<time_t, unsigned int> value_t;

typedef std::unordered_map<std::string, value_t> table_t;

struct _tc_planesnmp_moo_s
{
    CHAR                    strMsgTbl[TRANSC_INTERFACE_MAX][TRANSC_SNMPPLANE_MOOMSG_LEN];
    U16                     nstrMsgTblSz;
};
typedef struct _tc_planesnmp_moo_s
               tc_planesnmp_moo_t;

//*****************************************************************************************
//*****************************************************************************************
class snmp_processor
{
    public:
    snmp_processor() : m_traffic_count(0), m_redirect_count(0), m_max_table_xmit(50),
        m_edge_probe_duration(30), m_table_purge_duration(1800), m_start_time(time(NULL))
        { }

    tc_snmpplane_qmsgtype_e read_queue(void * pCntx);
    void handl_agentx_event(
        void * pCntx, char * raw_request, void * sock);
    void transmit_table(table_t & table, void * sock);
    void insert_or_update_table(table_t & table, std::string & key);
    void purge_tables(void);

    void set_maxTableXmit(uint32_t count)
    { m_max_table_xmit = count; return; }

    void set_edgeProbeDuration(double duration)
    { m_edge_probe_duration  = duration; return; }

    void set_tablePurgeDuration(double duration)
    { m_table_purge_duration  = duration; return; }

    void read_mode(void * context);
    void set_mode(void * context);

    double edge_probe_duration(void) { return m_edge_probe_duration; }
    double purge_duration(void)      { return m_table_purge_duration; }

    private:
    void purge_table(table_t & table);
    void print_table(table_t & table);

    table_t m_domain_table;
    table_t m_domain_redirect_table;
    table_t m_video_table;
    table_t m_video_redirect_table;
    table_t m_client_table;
    table_t m_client_redirect_table;
    table_t m_client_top_device_table;
    table_t m_redirected_service_table;

    uint32_t m_traffic_count;
    uint32_t m_redirect_count;
    uint32_t m_max_table_xmit;

    double m_edge_probe_duration;
    double m_table_purge_duration;

    tc_qmsgtbl_comptomib_t m_queue[TRANSC_MIB_QUEUE_MAX];
    uint32_t m_hunger_count; 

    time_t m_start_time;
    tc_planesnmp_moo_t m_mode_table;
};

//*****************************************************************************************
//*****************************************************************************************
namespace generic
{
    template< typename ContainerT, typename PredicateT >
    void erase_if( ContainerT& items, const PredicateT& predicate )
    {
        for( auto it = items.begin(); it != items.end(); )
        {
            if( predicate(*it) ) it = items.erase(it);
            else ++it;
        }
    };
}

#endif // TCPLANE_SNMP_H
