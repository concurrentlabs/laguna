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
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>

//*******************************************************************
// Function: socket_setup
//
// Description: Sets up tcp socket.
//*******************************************************************
int32_t socket_setup(uint16_t port)
{
    struct sockaddr_in serv_addr;;
    int32_t s;

    if((s = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    int32_t one = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    bzero(&serv_addr, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if(bind(s, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in)) == -1)
        return -1;

    if(listen(s, 1024) == -1)
        return -1;
    return s;
}

// ****************************************************************************
// Function: tcp_accept
//
// Description: Accept connection on tcp socket.
// ****************************************************************************
int32_t tcp_accept(int32_t listen_fd)
{
    struct sockaddr_in cli_addr;

    socklen_t len = sizeof(cli_addr);
    return accept(listen_fd, (struct sockaddr *) &cli_addr, &len);
}

// ****************************************************************************
// Function: tcp_connect
//
// Description: Connect to tcp server using port #.
// ****************************************************************************
int32_t tcp_connect(const char * host, uint16_t port)
{
    int32_t fd;
    struct sockaddr_in serv_addr;
    struct hostent *hp;

    if(!(hp = gethostbyname(host)))
        return -1;

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family  = AF_INET;
    serv_addr.sin_port    = htons(port);
    memcpy(&serv_addr.sin_addr, hp->h_addr, hp->h_length);

    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if(connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        close(fd);
        return -1;
    }
    return fd;
}
