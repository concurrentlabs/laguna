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

#define STATUS                    0
#define TRAFFIC_COUNT             1
#define REDIRECT_COUNT            2
#define DOMAIN_TABLE              3
#define DOMAIN_REDIRECT_TABLE     4
#define VIDEO_TABLE               5
#define VIDEO_REDIRECT_TABLE      6
#define CLIENT_TABLE              7
#define CLIENT_REDIRECT_TABLE     8
#define CLIENT_TOP_DEVICE_TABLE   9
#define READ_CPLANE_CONFIG       10
#define REDIRECTED_SERVICE_TABLE 11

#define AGENTX_SERVICE  5555

void * tcplane_snmp(void * context);

void increment_traffic_count(void);
void increment_redirect_count(void);

void update_traffic_count(unsigned long value);
void update_redirect_count(unsigned long value);

void increment_domainTable(char * key);
void increment_domainRedirectTable(char * key);
void increment_videoTable(char * key);
void increment_videoRedirectTable(char * key);
void increment_clientTable(char * key);
void increment_clientRedirectTable(char * key);
void increment_clientTopDeviceTable(char * key);
void increment_clientdomainTable(char* keyclient, char * keydomain);

void update_domainTable(char * key, unsigned long value);
void update_domainRedirectTable(char * key, unsigned long value);
void update_videoTable(char * key, unsigned long value);
void update_videoRedirectTable(char * key, unsigned long value);
void update_clientTable(char * key, unsigned long value);
void update_clientRedirectTable(char * key, unsigned long value);
void update_clientTopDeviceTable(char * key, unsigned long value);

void destroy_snmp_tables(void);

int svcredirect_init_table(void);
void svcredirect_set_svc(char * strSvcName);
void increment_redirectedTable(char* srcAddr,char* strHostName,char * strSvcName);
void increment_redirectedServiceTable(char * strSvcName);

#endif // TCPLANE_SNMP_H
