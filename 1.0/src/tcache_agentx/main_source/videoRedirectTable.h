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

#ifndef VIDEO_REDIRECT_TABLE_H
#define VIDEO_REDIRECT_TABLE_H

bool init_videoRedirectTable(void);

#define COLUMN_VIDEO_REDIRECT_IDX     1
#define COLUMN_VIDEO_REDIRECT	      2
#define COLUMN_VIDEO_REDIRECT_COUNT   3

//***********************************************************************
// TCACHE-MIB::videoRedirectTable of transCacheControlPlane.
// Its status is Current.
// OID: .1.3.6.1.4.1.1457.4.1.1.8, length: 11
//***********************************************************************
struct videoRedirectTable_data_s
{
    long          videoRedirectIdx;
    char          videoRedirect[64];
    unsigned long videoRedirectCount;
};

typedef struct videoRedirectTable_data_s videoRedirectTable_t;

#endif // VIDEO_REDIRECT_TABLE_H
