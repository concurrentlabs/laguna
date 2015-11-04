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

#ifndef TCACHE_AGENTX_H
#define TCACHE_AGENTX_H 1

enum { READ_TIMEOUT = 500, HTTP_SERVICE = 80, MGT_SERVICE = 5554, TCPLANE_SERVICE };

enum { status, trafficCount, redirectCount, domainTable, domainRedirectTable,
    videoTable, videoRedirectTable, clientTable, clientRedirectTable,
    clientTopDeviceTable, rereadCplaneConfig, redirectedServiceTable,
    getMaxTableXmit, setMaxTableXmit, getEdgeProbeDuration, setEdgeProbeDuration,
    getTablePurgeDuration, setTablePurgeDuration, startTime,  version, modeTable };

#endif // TCACHE_AGENT_X
