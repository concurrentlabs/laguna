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

#ifndef REDIRECTED_SERVICE_TABLE_H
#define REDIRECT_SERVICE_TABLE_H

bool init_redirectedServiceTable(void);

#define COLUMN_REDIRECTED_SERVICE_IDX    1
#define COLUMN_REDIRECTED_SERVICE        2
#define COLUMN_REDIRECTED_SERVICE_COUNT  3

//*************************************************************************
// TCACHE-MIB::redirectedServiceTable of transCacheControlPlane.
// Its status is Current.
// OID: .1.3.6.1.4.1.1457.4.1.1.12, length: 11
//*************************************************************************
struct redirectedServiceTable_data_s
{
    long          redirectedServiceIdx;
    char          redirectedService[256];
    unsigned long redirectedServiceCount;
};

typedef struct redirectedServiceTable_data_s redirectedServiceTable_t;

#endif // REDIRECTED_SERVICE_TABLE_H
