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

#ifndef TCP_RTNS_H
#define TCP_RTNS_H 1

#include <stdint.h>

int32_t socket_setup(uint16_t port);
int32_t tcp_accept(int32_t listen_fd);
int32_t tcp_connect(const char * host, uint16_t port);

#endif
