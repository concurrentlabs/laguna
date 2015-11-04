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

#ifndef DOMAIN_REDIRECT_TABLE_H
#define DOMAIN_REDIRECT_TABLE_H

bool init_domainRedirectTable(void);

#define COLUMN_DOMAIN_REDIRECT_IDX    1
#define COLUMN_DOMAIN_REDIRECT	      2
#define COLUMN_DOMAIN_REDIRECT_COUNT  3

//***********************************************************************
// TCACHE-MIB::domainRedirectTable of transCacheControlPlane.
// Its status is Current.
// OID: .1.3.6.1.4.1.1457.4.1.1.6, length: 11
//***********************************************************************
struct domainRedirectTable_data_s
{
    long          domainRedirectIdx;
    char          domainRedirect[64];
    unsigned long domainRedirectCount;
};

typedef struct domainRedirectTable_data_s domainRedirectTable_t;

#endif // DOMAIN_REDIRECT_TABLE_H
