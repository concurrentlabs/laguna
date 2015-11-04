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

#include <errno.h>
#include <strings.h>

#include <czmq.h>

//********************************************************************************
// Function: timed_read
//
// Description: Peform socket read with possible timeout in msec.
//              Return # bytes reead or 0 on timeout or error.
//********************************************************************************
size_t timed_read(void * socket, char * buffer, size_t size, int num_msec)
{
    zmq_pollitem_t items [] = { { socket, 0, ZMQ_POLLIN, 0 } };

    zmq_poll(items, 1, num_msec * ZMQ_POLL_MSEC);
    if(!(items[0].revents & ZMQ_POLLIN))
        return 0;
    int count = zmq_recv(socket, buffer, size, ZMQ_DONTWAIT);
    if(count == -1)
        return 0;
    buffer[count] = (char)0;
    return count;
}

//********************************************************************************
// Function: timed_recv
//
// Description: Peform socket read with possible timeout in msec.
//              Return # bytes reead or 0 on timeout or error.
//********************************************************************************
size_t timed_recv(int fd, char * buffer, size_t size, int num_msec)
{
    fd_set rdset;
    static struct timeval t;

    FD_ZERO(&rdset); FD_SET(fd, &rdset);
    t.tv_sec = 5; t.tv_usec = num_msec * 1000;
    int i = select(fd + 1, &rdset, 0, 0, &t);
    if(i <= 0)
        return 0;
    return recv(fd, buffer, size, MSG_DONTWAIT);
 }
