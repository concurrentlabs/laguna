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

//**********************************************************************************************
// cache-hndl
//
// Description: Manage Apache Traffic Server cache. - Version 1.
//**********************************************************************************************

#include <ccur_remap.h>

using namespace std;

#include <inttypes.h>

#include <ts/ts.h>
#include <ts/experimental.h>

#define PLUGIN_NAME "cache-hndl"

// #define ATS_6_0_0 1

//*******************************************************************************
//*******************************************************************************
struct cache_scan_state_t
{
    TSVConn net_vc;
    TSVIO read_vio;
    TSVIO write_vio;

    TSIOBuffer req_buffer;
    TSIOBuffer resp_buffer;
    TSIOBufferReader resp_reader;

    TSCont contp;

    TSHttpTxn txnp;

    string delete_key;

    uint32_t total_items;
    int64_t  total_bytes;

    //***************************************************************************
    //***************************************************************************
    cache_scan_state_t(TSHttpTxn txnp) :
        txnp(txnp), total_items(0), total_bytes(0)
    { }

    //***************************************************************************
    //***************************************************************************
    ~cache_scan_state_t()
    {
        TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

        if(net_vc)
            TSVConnShutdown(net_vc, 1, 1);

        if(req_buffer)
            TSIOBufferDestroy(req_buffer);
        if(resp_buffer)
            TSIOBufferDestroy(resp_buffer);

        if(contp)
            TSContDestroy(contp);

        TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
        return;
    }
};

//*********** global objects ***********
static remap_table_t global_remap_table;
//**************************************

//*******************************************************************************
//*******************************************************************************
static int handle_scan(TSCont contp, TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    cache_scan_state_t *cstate = (cache_scan_state_t *) TSContDataGet(contp);

    if(event == TS_EVENT_CACHE_REMOVE)
    {
        cstate->write_vio = TSVConnWrite(cstate->net_vc, contp, cstate->resp_reader, INT64_MAX);
        string s = "<td><font color=green>Cache remove operation succeeded.</font></td></tr>\n";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer, s.c_str(), s.length());
        TSVIONBytesSet(cstate->write_vio, cstate->total_bytes);
        TSVIOReenable(cstate->write_vio);
        return TS_SUCCESS;
    }

    if(event == TS_EVENT_CACHE_REMOVE_FAILED)
    {
        cstate->write_vio = TSVConnWrite(cstate->net_vc, contp, cstate->resp_reader, INT64_MAX);
        string s = "<td><font color=red>Cache remove operation failed.</font></td></tr>\n";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer, s.c_str(), s.length());
        TSVIONBytesSet(cstate->write_vio, cstate->total_bytes);
        TSVIOReenable(cstate->write_vio);
        return TS_SUCCESS;
    }

    if(event == TS_EVENT_CACHE_SCAN)
    {
        cstate->write_vio = TSVConnWrite(cstate->net_vc, contp, cstate->resp_reader, INT64_MAX);
        string s = "<h3>Cache Contents:</h3>\n<p><pre>\n";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
            s.c_str(), s.length());
        return TS_EVENT_CONTINUE;
    }

    if(event == TS_EVENT_CACHE_SCAN_FAILED ||
       event == TS_EVENT_CACHE_SCAN_OPERATION_BLOCKED ||
       event == TS_EVENT_CACHE_SCAN_OPERATION_FAILED)
    {
        if(cstate->resp_buffer)
        {
            string s = "Cache scan operation blocked or failed.";
            cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
                s.c_str(), s.length());
        }
        if(cstate->write_vio)
        {
            TSVIONBytesSet(cstate->write_vio, cstate->total_bytes);
            TSVIOReenable(cstate->write_vio);
        }
        return TS_CACHE_SCAN_RESULT_DONE;
    }

    if(event == TS_EVENT_CACHE_SCAN_OBJECT)
    {
        TSCacheHttpInfo cache_infop = (TSCacheHttpInfo) edata;
        TSMBuffer req_bufp;
        TSMLoc req_hdr_loc, url_loc;

        TSCacheHttpInfoReqGet(cache_infop, &req_bufp, &req_hdr_loc);
        TSHttpHdrUrlGet(req_bufp, req_hdr_loc, &url_loc);
        int url_len;
        char * s = TSUrlStringGet(req_bufp, url_loc, &url_len);
        string orgin_url(s, url_len);
        TSfree(s);

        string cache_url;
        cache_url = global_remap_table.construct_remap_url(string("cache-hndl"),
		orgin_url);
        cache_url += "\n";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
            cache_url.c_str(), cache_url.length());
        TSHandleMLocRelease(req_bufp, req_hdr_loc, url_loc);
        TSHandleMLocRelease(req_bufp, TS_NULL_MLOC, req_hdr_loc);
        TSVIOReenable(cstate->write_vio);
        cstate->total_items++;
        return TS_CACHE_SCAN_RESULT_CONTINUE;
    }

    if(event == TS_EVENT_CACHE_SCAN_DONE)
    {
        ostringstream oss;
        oss << "</pre></p>\n<p>" << cstate->total_items << " total objects in cache</p>\n"
            << "<form method=\"GET\" action=\"/cache-handler\">"
            << "Enter URL to delete: <input type=\"text\" size=\"96\" name=\"remove_url\">"
            << "<input type=\"submit\"  value=\"Delete URL\">";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
            oss.str().c_str(), oss.str().length());
        TSVIONBytesSet(cstate->write_vio, cstate->total_bytes);
        TSVIOReenable(cstate->write_vio);
        return TS_CACHE_SCAN_RESULT_DONE;
    }
    TSDebug(PLUGIN_NAME, "Unknown event in handle_scan: %d.", event);
    return -1;
}

//*******************************************************************************
//*******************************************************************************
static int handle_accept(TSCont contp, TSEvent event, TSVConn vc)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    cache_scan_state_t *cstate = (cache_scan_state_t *) TSContDataGet(contp);

    if(event == TS_EVENT_NET_ACCEPT)
    {
        if(cstate)
        {
            cstate->net_vc = vc;
            cstate->req_buffer = TSIOBufferCreate();
            cstate->resp_buffer = TSIOBufferCreate();
            cstate->resp_reader = TSIOBufferReaderAlloc(cstate->resp_buffer);

            cstate->read_vio = TSVConnRead(cstate->net_vc, contp,
                cstate->req_buffer, INT64_MAX);
        }
    }
    else
    {
        if(cstate)
            delete cstate;
    }
    return TS_SUCCESS;
}

//*******************************************************************************
//*******************************************************************************
static void cache_delete(TSCont contp, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    cache_scan_state_t * cstate = (cache_scan_state_t *) TSContDataGet(contp);

    TSCacheKey key =  TSCacheKeyCreate();
    TSDebug(PLUGIN_NAME, "deleting url: %s.", cstate->delete_key.c_str());

    TSMBuffer urlBuf = TSMBufferCreate();
    TSMLoc urlLoc;
    TSUrlCreate(urlBuf, &urlLoc);

    const char * start = &cstate->delete_key[0];
    const char * end   = &start[cstate->delete_key.length()];
    if(TSUrlParse(urlBuf, urlLoc, &start, end) != TS_PARSE_DONE ||
        TSCacheKeyDigestFromUrlSet(key, urlLoc) != TS_SUCCESS)
    {
        TSDebug(PLUGIN_NAME, "CacheKeyDigestFromUrlSet failed.");
    }
    TSCacheRemove(contp, key);
    TSCacheKeyDestroy(key);
    TSHandleMLocRelease(urlBuf, NULL, urlLoc);
    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return;
}

//*******************************************************************************
//*******************************************************************************
static int handle_io(TSCont contp, TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);
    TSDebug(PLUGIN_NAME, "%s: Event: %d.", __FUNCTION__, event);

    cache_scan_state_t * cstate = (cache_scan_state_t *) TSContDataGet(contp);

    string s;
    switch(event)
    {
        case TS_EVENT_VCONN_READ_READY:
        case TS_EVENT_VCONN_READ_COMPLETE:
        TSVConnShutdown(cstate->net_vc, 1, 0);
        s =  "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
            s.c_str(), s.length());

        if(cstate->delete_key.length())
            cache_delete(contp, edata);
        else
            TSCacheScan(contp, 0, 512000);
        break;

        case TS_EVENT_VCONN_WRITE_READY:
        TSDebug(PLUGIN_NAME,
            "Event: TS_EVENT_VCONN_WRITE_READY, ndone: %ld, total_bytes: %ld.",
            TSVIONDoneGet(cstate->write_vio), cstate->total_bytes);
        break;

        case TS_EVENT_VCONN_WRITE_COMPLETE:
        case TS_EVENT_VCONN_EOS:
        TSDebug(PLUGIN_NAME, "%s: Event: %d.", __FUNCTION__, event);
        delete cstate;
        break;

        default:
        TSDebug(PLUGIN_NAME, "%s: Unexpected event: %d", __FUNCTION__, event);
        break;
    }
    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TS_SUCCESS;
}

//*******************************************************************************
//*******************************************************************************
static int cache_intercept(TSCont contp, TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);
    // TSDebug(PLUGIN_NAME, "%s: event: %d", __FUNCTION__, event);

    cache_scan_state_t * cstate = (cache_scan_state_t *) TSContDataGet(contp);

    switch(event)
    {
        case TS_EVENT_NET_ACCEPT:
        case TS_EVENT_NET_ACCEPT_FAILED:
        return handle_accept(contp, event, (TSVConn) edata);

        case TS_EVENT_VCONN_READ_READY:
        case TS_EVENT_VCONN_READ_COMPLETE:
        case TS_EVENT_VCONN_WRITE_READY:
        case TS_EVENT_VCONN_WRITE_COMPLETE:
        case TS_EVENT_VCONN_EOS:
        return handle_io(contp, event, edata);

        case TS_EVENT_CACHE_SCAN:
        case TS_EVENT_CACHE_SCAN_FAILED:
        case TS_EVENT_CACHE_SCAN_OBJECT:
        case TS_EVENT_CACHE_SCAN_OPERATION_BLOCKED:
        case TS_EVENT_CACHE_SCAN_OPERATION_FAILED:
        case TS_EVENT_CACHE_SCAN_DONE:
        case TS_EVENT_CACHE_REMOVE:
        case TS_EVENT_CACHE_REMOVE_FAILED:
        return handle_scan(contp, event, edata);

        case TS_EVENT_ERROR:
        if(cstate)
            delete cstate;
        TSDebug(PLUGIN_NAME, "%s: TS_EVENT_ERROR.", __FUNCTION__);
        break;

        default:
        TSDebug(PLUGIN_NAME, "%s: Unexpected event: %d", __FUNCTION__, event);
        if(cstate)
            delete cstate;
        break;
    }
    return TS_SUCCESS;
}

//*******************************************************************************
//*******************************************************************************
static void unescapifyStr(string & s)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    ostringstream oss;
    string::size_type i{};

    while(i < s.length())
    {
        if(s[i] == '%' && s[i + 1] != '\0' && s[i + 2] != '\0')
        {
            ++i;
            string t;
            t += s[i]; 
            ++i;
            t += s[i];
            ++i;
            oss << (char) strtol(t.c_str(), (char**) NULL, 16);
            continue;
        }
        else if(s[i] == '+')
            oss << ' ';
        else
            oss << s[i];
        ++i;
    }
    s = oss.str();
    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return;
}

//*******************************************************************************
//*******************************************************************************
static int setup_request(TSCont contp, TSHttpTxn txnp)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    TSMBuffer bufp;
    TSMLoc hdr_loc;
    TSMLoc url_loc;
    int path_len, query_len;

    if(TSHttpTxnClientReqGet(txnp, &bufp, &hdr_loc) != TS_SUCCESS)
    {
        TSDebug(PLUGIN_NAME, "Couldn't retrieve client request header.");
        TSHttpTxnReenable(txnp, TS_EVENT_HTTP_CONTINUE);
        return TS_SUCCESS;
    }

    if(TSHttpHdrUrlGet(bufp, hdr_loc, &url_loc) != TS_SUCCESS)
    {
        TSDebug(PLUGIN_NAME, "Couldn't retrieve request url.");
        TSHandleMLocRelease(bufp, TS_NULL_MLOC, hdr_loc);
        TSHttpTxnReenable(txnp, TS_EVENT_HTTP_CONTINUE);
        return TS_SUCCESS;
    }

    const char * s = TSUrlPathGet(bufp, url_loc, &path_len);
    if(!s)
    {
        TSDebug(PLUGIN_NAME, "Couldn't retrieve request path.");
        TSHandleMLocRelease(bufp, hdr_loc, url_loc);
        TSHandleMLocRelease(bufp, TS_NULL_MLOC, hdr_loc);
        TSHttpTxnReenable(txnp, TS_EVENT_HTTP_CONTINUE);
        return TS_SUCCESS;
    }
    string path(s, path_len);

    if(path == "cache-handler")
    {
        cache_scan_state_t * cstate = new cache_scan_state_t(txnp);
        cstate->contp = TSContCreate(cache_intercept, TSMutexCreate());
        TSHttpTxnIntercept(cstate->contp, txnp);

        s = TSUrlHttpQueryGet(bufp, url_loc, &query_len);
        string query(s, query_len);
        if(query.length())
        {
            unescapifyStr(query);
            boost::smatch matches;
            if(boost::regex_match(query, matches, boost::regex("remove_url=(.*)$")))
            {
                TSDebug(PLUGIN_NAME, "Setting delete key: %s.",
                    matches[1].str().c_str());
                cstate->delete_key = matches[1].str();
            }
        }
        TSContDataSet(cstate->contp, cstate);
        TSDebug(PLUGIN_NAME, "setup cache intercept.");
    }
    TSHandleMLocRelease(bufp, hdr_loc, url_loc);
    TSHandleMLocRelease(bufp, TS_NULL_MLOC, hdr_loc);
    TSHttpTxnReenable(txnp, TS_EVENT_HTTP_CONTINUE);

    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TS_SUCCESS;
}

//************************************************************************************
//************************************************************************************
static int cache_hndl_plugin(TSCont contp, TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);
    TSDebug(PLUGIN_NAME, "%s: event == %d.", __FUNCTION__, event);

    switch(event)
    {
        case TS_EVENT_HTTP_READ_REQUEST_HDR:
        return setup_request(contp, (TSHttpTxn) edata);

        default:
        TSDebug(PLUGIN_NAME, "%s: Unexpected event: %d", __FUNCTION__, event);
        break;
    }
    TSHttpTxnReenable((TSHttpTxn) edata, TS_EVENT_HTTP_CONTINUE);
    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TS_SUCCESS;
}

//************************************************************************************
//************************************************************************************
void TSPluginInit(int argc, const char *argv[])
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

#ifdef ATS_6_0_0
        TSPluginRegistrationInfo info;
        info.plugin_name = (char*) "cache-hndl-v1";
        info.vendor_name = (char*) "Concurrent Computer Corporation";
        info.support_email = (char *) " ";

        if(!TSPluginRegister(&info))
            TSError ("[plugin_name] Plugin registration failed.");
#endif

    string path = TSConfigDirGet();
    path += "/";
    if(argc == 2)
        path += argv[1];
    else
        path += "cacheurl.config";
    global_remap_table.load_config_file(path);

    TSCont contp = TSContCreate(cache_hndl_plugin, TSMutexCreate());
    TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, contp);

    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return;
}
