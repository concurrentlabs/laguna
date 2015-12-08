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

#include <arpa/inet.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <jansson.h>
#include <string.h>
#include <unistd.h>
#include "health.h"

#define BUFF_SIZE 4096

typedef struct
{
    char buffer[BUFF_SIZE];
    size_t size;
} UrlData_t;

// RPC Message Header Format, originally defined in cache_manager_rpc.h
typedef struct
{
    unsigned short  code;
    unsigned short  status;
    unsigned int    tid;
    unsigned int    chid;
    unsigned int    dataLen;
} rpcMessageHdr_t;

#define DEFAULT_MHCM_PORT 8040

#define RPCCMD_REQUEST  0x0100
#define RPCCMD_RESPONSE 0x0200

#define CM_GET_CMSTATS_REQ   (RPCCMD_REQUEST + 6)
#define CM_GET_CMSTATS_RESP  (RPCCMD_RESPONSE + 6)

static int16_t _cdn_status(tc_health_thread_ctxt_t * _pCntx,char *ip);

static int16_t ccur_edge_active(const char * ip);

static int16_t ccur_rr_active_sites(tc_health_thread_ctxt_t * _pCntx,const char * ip);

static int16_t process_edge(json_t * edge);

static size_t UrlCallback(void * contents, size_t size, size_t nmemb, void * userp);

static int32_t tcp_connect(const char * host, uint16_t port);

static size_t read_async(int32_t fd, uint8_t buf[], size_t length);


//**********************************************************************************
// function: _cdn_status
//
// description: First query redirect ip address to see if it is request router.
//              If active_sites are returned, edge is active.
//
//              If redirect ip address isn't request router, query as edge.
//              If response is returned, edge is active.
//
// return: 1 for edge active, 0 for edge inactive.
//**********************************************************************************
int16_t _cdn_status(tc_health_thread_ctxt_t * _pCntx,char *ip)
{
    if(_pCntx->bNoRRPolling)
    {
        return 1;
    }
    if((strcmp(ip, "127.0.0.1") == 0) ||
       ('\0' == ip[0]))
        return 0;
    if(ccur_rr_active_sites(_pCntx, ip))
        return 1;
    if(ccur_edge_active(ip))
        return 1;
    return 0;
}

void cdn_status(tc_health_thread_ctxt_t * pCntx)
{
    /* update map interface link status */
    U16                             _i;
    tc_health_monactv_t*         _pPing;

    for(_i=0;_i<pCntx->nMonActvTbl;_i++)
    {
        _pPing = &(pCntx->tMonActvTbl[_i]);
        _pPing->bIsRedirUp =
                _cdn_status(pCntx,_pPing->strRedirAddr);
        /* Print to log only if state change from up to down,
         * vice versa. */
        if(_pPing->bOldRedirIsUp)
        {
            if( FALSE ==
                    _pPing->bIsRedirUp)
            {
                if('\0' != _pPing->strRedirAddr[0])
                    evLogTrace(
                            pCntx->pQHealthToBkgrnd,
                            evLogLvlWarn,
                            &(pCntx->tLogDescSys),
                           "cache node(s): %s/down",
                           _pPing->strRedirAddr);
                else
                    evLogTrace(
                            pCntx->pQHealthToBkgrnd,
                            evLogLvlWarn,
                            &(pCntx->tLogDescSys),
                           "cache node(s): null/down");
            }
        }
        else
        {
            if( TRUE ==
                    _pPing->bIsRedirUp)
            {
                if('\0' != _pPing->strRedirAddr[0])
                    evLogTrace(
                            pCntx->pQHealthToBkgrnd,
                            evLogLvlWarn,
                            &(pCntx->tLogDescSys),
                           "cache node(s): %s/up",
                           _pPing->strRedirAddr);
                else
                    evLogTrace(
                            pCntx->pQHealthToBkgrnd,
                            evLogLvlWarn,
                            &(pCntx->tLogDescSys),
                           "cache node(s): null/down");
            }
        }
        _pPing->bOldRedirIsUp = _pPing->bIsRedirUp;
    }
}

//**********************************************************************************
// function: ccur_rr_active_sites
//
// description: Determines if Concurrent request_router has acvive edge sites.
//              Returns 0 for no sites, # of active edge sites.
//**********************************************************************************
static int16_t ccur_rr_active_sites(tc_health_thread_ctxt_t * pCntx,
                               const char * ip)
{
    CURL * curl;
    CURLcode res;
    UrlData_t url_data;
    json_error_t error;
    int16_t ret = 0;
    const char * url_fmt = "http://%s/api/servers/";
    char url[64];

    bzero(&url_data, sizeof(url_data));
    snprintf(url, sizeof(url),url_fmt, ip);
    url[sizeof(url)-1] = '\0';
 
    curl = curl_easy_init();
    if(NULL == curl)
        return 0;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, UrlCallback);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)& url_data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        return 0;
    }
    curl_easy_cleanup(curl);

    json_t * root = json_loads(url_data.buffer, 0, &error);
    if(!root)
        return 0;
    void *iter = json_object_iter(root);
    if(!iter)
    {
        json_decref(root);
        return 0;
    }
    const char * key = json_object_iter_key(iter);
    json_t * j = json_object_iter_value(iter);

    json_object_del(j, "route_expression");
    json_object_del(j, "route_expression_suffix");
    json_object_del(j, "date");
    json_object_del(j, "active_sites");
    json_object_del(j, "supported_sites");

    json_t * edge_cluster;
    json_object_foreach(j, key, edge_cluster)
    {
        evLogTrace(
                pCntx->pQHealthToBkgrnd,
                evLogLvlDebug,
                &(pCntx->tLogDescSys),
                "Processing edge_cluster %s.", key);
        void *iter = json_object_iter(edge_cluster);
        if(process_edge(json_object_iter_value(iter)))
        {
            ret = 1;
            break;
        }
    }
    json_decref(root);
    return ret;
}

//**********************************************************************************
// function: process_edge
//
// description: compare edge server_group against servers_bad.
//              return 1 on success, o on fail.
//**********************************************************************************
static int16_t process_edge(json_t * edge)
{
    json_t * server_group = json_object_get(edge, "server_group");
    json_t * servers_bad  = json_object_get(edge, "servers_bad");
    if(json_array_size(server_group) > json_array_size(servers_bad))
        return 1;
    return 0;
}

//**********************************************************************************
// function: UrlCallback
//
// description: curl callback to recieve json data.
//**********************************************************************************
static size_t UrlCallback(void * contents, size_t size, size_t nmemb, void * userp)
{
    size_t realsize = size * nmemb;
    UrlData_t *mem = (UrlData_t *) userp;

    if((mem->size + realsize) >= BUFF_SIZE -1)
        return 0;
    memcpy(&(mem->buffer[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->buffer[mem->size] = 0;
    return realsize;
}

//*************************************************************************************
// function: ccur_edge_active
//
// description: If redirectaddress is Concurrent edge server, determines if edge is up.
//*************************************************************************************
static int16_t ccur_edge_active(const char * ip)
{
    rpcMessageHdr_t hdr;
    size_t msg_len;
    int32_t fd;

    bzero(&hdr, sizeof(hdr));
    hdr.code = CM_GET_CMSTATS_REQ;
    hdr.tid = 1;

    fd = tcp_connect(ip, 8040);
    if(fd < 0)
        return 0;

    if(write(fd, &hdr, sizeof(rpcMessageHdr_t)) < 0)
    {
        close(fd);
        return 0;
    }
    bzero(&hdr, sizeof(hdr));
    msg_len = read_async(fd, (uint8_t*) &hdr, sizeof(hdr));
    if(msg_len <= 0)
    {
        close(fd);
        return 0;
    }
    if(hdr.code != CM_GET_CMSTATS_RESP)
    {
        close(fd);
        return 0;
    }
    close(fd);
    return 1;
}

// ****************************************************************************
// Function: tcp_connect
//
// Description: Connect to tcp server using port #.
// ****************************************************************************
static int32_t tcp_connect(const char * ip, uint16_t port)
{
    int32_t fd;
    struct sockaddr_in serv_addr;
    struct timeval t;
    fd_set set;

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family  = AF_INET;
    serv_addr.sin_port    = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    int arg = fcntl(fd, F_GETFL, NULL);
    arg |= O_NONBLOCK;
    fcntl(fd, F_SETFL, arg);
    int ret = connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if(ret == -1)
    {
        bzero(&set, sizeof(set));
        t.tv_sec = 0;
        t.tv_usec = 5000000;
        FD_ZERO(&set);
        FD_SET(fd, &set);
        ret = select(fd + 1, NULL, &set, NULL, &t);
    }
    arg = fcntl(fd, F_GETFL, NULL);
    arg &= (~O_NONBLOCK);
    fcntl(fd, F_SETFL, arg);
    return (ret > 0) ? fd : -1;
}

// ****************************************************************************
// function:    readn_async
//
// description: Read bytes from file descriptor.  Only blocks for 5 seconds.
//              returns # bytes recieved.
// ****************************************************************************
static size_t read_async(int32_t fd, uint8_t buf[], size_t length)
{
    fd_set rdset;
    static struct timeval t;

    FD_ZERO(&rdset); FD_SET(fd, &rdset);
    bzero(&t, sizeof(struct timeval));
    t.tv_usec = 5000000;
    if(select(fd + 1, &rdset, 0, 0, &t) <= 0)
        return 0;
    size_t nread = read(fd, buf, length);
    return nread;
}
