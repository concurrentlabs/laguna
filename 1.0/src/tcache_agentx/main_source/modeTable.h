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

#ifndef MODE_TABLE_H
#define MODE_TABLE_H

bool init_modeTable(void);

#define COLUMN_MODE_IDX    1
#define COLUMN_MODE	       2
#define COLUMN_MODE_COUNT  3

//*****************************************************************
// TCACHE-MIB::modeTable of transCacheControlPlane.
// Its status is Current.
// OID: .1.3.6.1.4.1.1457.4.1.1.18, length: 11
//*****************************************************************
struct modeTable_data_s
{
    long          modeIdx;
    char          mode[64];
};

typedef struct modeTable_data_s modeTable_t;

#endif // MODE_TABLE_H
