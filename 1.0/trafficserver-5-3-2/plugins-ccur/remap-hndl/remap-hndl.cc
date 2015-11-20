
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

#include <ccur_remap.h>

using namespace std;

#include <inttypes.h>

#include <ts/ts.h>
#include <remap.h>

//************** global objects **************
static remap_table_t global_origin_remap_table;
static remap_table_t global_cache_remap_table;
//********************************************

#define PLUGIN_NAME "remap-hndl"

//************************************************************************************
//************************************************************************************
TSReturnCode TSRemapInit(TSRemapInterface * api_info, char *errbuf, int errbuf_size)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    if(!api_info)
    {
        strncpy(errbuf, "[tsremap_init] Invalid TSRemapInterface argument.",
            errbuf_size - 1);
        return TS_ERROR;
    }

    if(api_info->size < sizeof(TSRemapInterface))
    {
        strncpy(errbuf, "[tsremap_init] Incorrect size of TSRemapInterface structure.",
            errbuf_size - 1);
        return TS_ERROR;
    }

    if(api_info->tsremap_version < TSREMAP_VERSION)
    {
        snprintf(errbuf, errbuf_size - 1, "[tsremap_init] Incorrect API version %ld.%ld.",
            api_info->tsremap_version >> 16, (api_info->tsremap_version & 0xffff));
        return TS_ERROR;
    }
    TSDebug(PLUGIN_NAME, "Plugin is successfully initialized.");
    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TS_SUCCESS;
}

//************************************************************************************
//************************************************************************************
TSReturnCode TSRemapNewInstance(int argc, char *argv[], void **ih,
    char *errbuf, int errbuf_size)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    string path = TSConfigDirGet();
    path += "/";
    path += argv[2];
    global_origin_remap_table.load_config_file(path);

    path = TSConfigDirGet();
    path += "/";
    path += argv[3];
    global_cache_remap_table.load_config_file(path);

    // global_origin_remap_table.print();
    // cout << "================================================================\n";
    // global_cache_remap_table.print();

    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TS_SUCCESS;
}

//************************************************************************************
//************************************************************************************
void TSRemapDeleteInstance(void *ih)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);
    global_origin_remap_table.clear();
    global_cache_remap_table.clear();
    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return;
}

//************************************************************************************
//************************************************************************************
TSRemapStatus TSRemapDoRemap(void *ih, TSHttpTxn rh, TSRemapRequestInfo * rri)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    char *s{};
    int len{};

    // s = (char*) TSUrlSchemeGet(rri->requestBufp, rri->requestUrl, &len);
    // string scheme(s, len);
    // s = (char*) TSUrlHostGet(rri->requestBufp, rri->requestUrl, &len);
    // string host(s, len);
    // s = (char*) TSUrlPathGet(rri->requestBufp, rri->requestUrl, &len);
    // string path(s, len);
    // s = (char*) TSUrlHttpQueryGet(rri->requestBufp, rri->requestUrl, &len);
    // string query(s, len);
    // s = (char*) TSUrlHttpParamsGet(rri->requestBufp, rri->requestUrl, &len);
    // string matrix(s, len);
    // int port = TSUrlPortGet(rri->requestBufp, rri->requestUrl);

    // TSMBuffer buf;
    // TSMLoc offset;
    // TSHttpTxnClientReqGet(static_cast<TSHttpTxn>(rh), &buf, &offset);
    // s = (char*) TSHttpHdrMethodGet(buf, offset, &len);
    // string method(s, len);

    s = TSHttpTxnEffectiveUrlStringGet(rh, &len);
    if(!s)
        TSError("[%s] couldn't retrieve request url.\n", PLUGIN_NAME);
    string url(s, len);
    TSfree(s);

    //*** set cache url ***
    string cache_url = global_cache_remap_table.construct_remap_url(string(PLUGIN_NAME), url);
    if(TSCacheUrlSet(rh, cache_url.c_str(), cache_url.length()) != TS_SUCCESS)
    {
        TSError("[%s] Unable to modify cache url from " "%s to %s.\n",
            PLUGIN_NAME, url.c_str(), cache_url.c_str());

    }

    //*** set remap url ***
    string edge_url = global_origin_remap_table.construct_remap_url(string(PLUGIN_NAME), url);
    const char * start = &edge_url[0];
    const char * end   = &start[edge_url.length()];
    if(TSUrlParse(rri->requestBufp, rri->requestUrl, &start, end) == TS_PARSE_ERROR)
    {
        TSHttpTxnSetHttpRetStatus(rh, TS_HTTP_STATUS_INTERNAL_SERVER_ERROR);
        TSError("Can't parse substituted URL string.");
        return TSREMAP_NO_REMAP;
    }
    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TSREMAP_DID_REMAP;
}
