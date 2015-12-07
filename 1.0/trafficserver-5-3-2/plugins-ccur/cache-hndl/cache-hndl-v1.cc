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

// #define __ATS_6_0_0__ 1

static int CacheHandler(TSCont contp, TSEvent event, void * edata);

static int handleCacheScan(TSEvent event, void * edata);

//*******************************************************************************
// struct: cache_scan_state_t
//
// desctiption: encapsulate cache scan functionality.
//*******************************************************************************
struct cache_scan_state_t
{
    TSCont contp;

    TSVConn net_vc;

    TSIOBuffer req_buffer;
    TSIOBuffer resp_buffer;
    TSIOBufferReader resp_reader;

    TSVIO read_vio;
    TSVIO write_vio;

    int64_t total_bytes;

    //***************************************************************************
    // function: constructor
    //
    // description: setup ats objects.
    //***************************************************************************
    cache_scan_state_t() :
        contp(TSContCreate(CacheHandler, TSMutexCreate())),
        req_buffer(TSIOBufferCreate()),
        resp_buffer(TSIOBufferCreate()),
        resp_reader(TSIOBufferReaderAlloc(resp_buffer)),
        total_bytes(0)
    { }

    //***************************************************************************
    // function: destructor
    //
    // description: close and free ats objects.
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
        if(resp_reader)
            TSIOBufferReaderFree(resp_reader);

        TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
        return;
    }

    //***************************************************************************
    // function: accept
    //
    // description: perform http accept.
    //***************************************************************************
    void accept(void * v)
    {
        TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

        net_vc = (TSVConn) v;
        read_vio  = TSVConnRead(net_vc, contp, req_buffer, INT64_MAX);
        write_vio = TSVConnWrite(net_vc, contp, resp_reader, INT64_MAX);

        TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
        return;
    }
};

//*******************************************************************************
// class: delete_keys_t
//
// desctiption: encapsutate cache delete functionality.
//*******************************************************************************
class delete_keys_t
{
    public:
    //***************************************************************************
    // function: constructor
    //
    // description: split string into list of keys.
    //***************************************************************************
    delete_keys_t(string & s)
    {
        if(s.length())
        {
            // *** strip '&' and '=on' and remove escape sequences ***
            boost::regex re("[&]");
            boost::sregex_token_iterator p(s.cbegin(), s.cend(), re, -1);
            boost::sregex_token_iterator e;
            for( ; p != e; ++p)
            {
                string key(p->str().substr(0, p->length() - 3));
                unescapeStr(key);
                m_keys.push_back(move(key));
            }
        }
    }

    //***************************************************************************
    //***************************************************************************
    ~delete_keys_t() { }

    //***************************************************************************
    //***************************************************************************
    void unescapeStr(string & s)
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

    //***************************************************************************
    // function: perform_deletions
    //
    // description: delete from cache using list of keys.
    //***************************************************************************
    void perform_deletions(void)
    {
        TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

        TSCont contp = TSContCreate(CacheHandler, TSMutexCreate());

        for(auto elem : m_keys)
        {
            TSCacheKey key =  TSCacheKeyCreate();
            TSMBuffer urlBuf = TSMBufferCreate();
            TSMLoc urlLoc;
            TSUrlCreate(urlBuf, &urlLoc);
            TSDebug(PLUGIN_NAME, "deleting url: %s.", elem.c_str());
            const char * start = &elem[0];
            const char * end   = &start[elem.length()];
            TSUrlParse(urlBuf, urlLoc, &start, end);
            TSCacheKeyDigestFromUrlSet(key, urlLoc);
            TSCacheRemove(contp, key);
            TSCacheKeyDestroy(key);
            TSHandleMLocRelease(urlBuf, NULL, urlLoc);
        }
        TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
        return;
    }

    private:
    list<string> m_keys;
};

//*********** global objects ***********
static remap_table_t global_remap_table;
static list<string> global_cache_table;
static cache_scan_state_t * cstate{};
//**************************************

//*******************************************************************************
//*******************************************************************************
static int handleCacheScan(TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    string s{};
    if(event == TS_EVENT_CACHE_SCAN)
    {
        global_cache_table.clear();
        return TS_EVENT_CONTINUE;
    }

    if(event == TS_EVENT_CACHE_SCAN_OBJECT)
    {
        TSCacheHttpInfo cache_infop = (TSCacheHttpInfo) edata;
        TSMBuffer req_bufp;
        TSMLoc req_hdr_loc, url_loc;

        TSCacheHttpInfoReqGet(cache_infop, &req_bufp, &req_hdr_loc);
        TSHttpHdrUrlGet(req_bufp, req_hdr_loc, &url_loc);
        int url_len;
        char * cstr = TSUrlStringGet(req_bufp, url_loc, &url_len);
        string orgin_url(cstr, url_len);
        TSfree(cstr);
        string cache_url = global_remap_table.construct_remap_url(string("cache-hndl"),
		orgin_url);
        global_cache_table.push_back(cache_url);

        TSHandleMLocRelease(req_bufp, req_hdr_loc, url_loc);
        TSHandleMLocRelease(req_bufp, TS_NULL_MLOC, req_hdr_loc);
        return TS_CACHE_SCAN_RESULT_CONTINUE;
    }

    if(event == TS_EVENT_CACHE_SCAN_DONE)
    {
        s = "<H3>Cache Contents:</H3><PRE>"
            "<TABLE>"
            "<STYLE>"
            "TABLE, TD { border: 1px solid black; }"
            "TABLE { border-spacing: 2px; }"
            "TD { background-color: #F2EFFB; paddding: 2px; }"
            "</STYLE>"
            "<FORM NAME=\"f\" ID=\"f\" METHOD=\"GET\"";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
            s.c_str(), s.length());

        ostringstream oss;
        for(auto elem : global_cache_table)
        {
            ostringstream oss;
            oss << "<TR><TD><INPUT TYPE=CHECKBOX NAME=\"" << elem << "\"" << "\"></TD>";
            oss << "<TD>" << elem << "</TD></TR>";
            cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
                oss.str().c_str(), oss.str().length());
        }
        s =  "</TABLE><BR>";
        s += "<INPUT TYPE=SUBMIT VALUE=\"Delete\"> </PRE></FORM>";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
            s.c_str(), s.length());
        TSVIONBytesSet(cstate->write_vio, cstate->total_bytes);
        TSVIOReenable(cstate->write_vio);
        return TS_CACHE_SCAN_RESULT_DONE;
    }

    if(event == TS_EVENT_CACHE_SCAN_FAILED ||
       event == TS_EVENT_CACHE_SCAN_OPERATION_BLOCKED ||
       event == TS_EVENT_CACHE_SCAN_OPERATION_FAILED)
    {
        if(cstate->resp_buffer)
        {
            s = "<H3>Cache scan operation blocked or failed.</H3>\n";
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

    if(event == TS_EVENT_CACHE_REMOVE_FAILED)
    {
        TSDebug(PLUGIN_NAME, "%s, %s.", __FUNCTION__,
		"Event: TS_EVENT_CACHE_REMOVE_FAILED");
    }

    if(event == TS_EVENT_CACHE_REMOVE)
    {
        TSDebug(PLUGIN_NAME, "%s, %s.", __FUNCTION__,
		"Event: TS_EVENT_CACHE_REMOVE");
    }
    TSDebug(PLUGIN_NAME, "Unknown event in %s: %d.", __FUNCTION__, event);
    return -1;
}

//*******************************************************************************
//*******************************************************************************
static int handleIO(TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);
    TSDebug(PLUGIN_NAME, "%s: Event: %d.", __FUNCTION__, event);

    string s;
    switch(event)
    {
        case TS_EVENT_VCONN_READ_READY:
        case TS_EVENT_VCONN_READ_COMPLETE:
        TSVConnShutdown(cstate->net_vc, 1, 0);
        s =  "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
            s.c_str(), s.length());

        TSCacheScan(cstate->contp, 0, 512000);
        break;

        case TS_EVENT_VCONN_WRITE_READY:
        TSDebug(PLUGIN_NAME,
            "Event: TS_EVENT_VCONN_WRITE_READY, ndone: %ld, total_bytes: %ld.",
            TSVIONDoneGet(cstate->write_vio), cstate->total_bytes);
        break;

        case TS_EVENT_VCONN_WRITE_COMPLETE:
        case TS_EVENT_VCONN_EOS:
        TSDebug(PLUGIN_NAME, "%s: Event: %d.", __FUNCTION__, event);
	if(cstate)
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
static int CacheHandler(TSCont contp, TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    switch(event)
    {
        case TS_EVENT_NET_ACCEPT:
        cstate->accept(edata);
        return TS_SUCCESS;

        case TS_EVENT_NET_ACCEPT_FAILED:
        delete cstate;
        return TS_SUCCESS;

        case TS_EVENT_VCONN_READ_READY:
        case TS_EVENT_VCONN_READ_COMPLETE:
        case TS_EVENT_VCONN_WRITE_READY:
        case TS_EVENT_VCONN_WRITE_COMPLETE:
        case TS_EVENT_VCONN_EOS:
        return handleIO(event, edata);

        case TS_EVENT_CACHE_SCAN:
        case TS_EVENT_CACHE_SCAN_FAILED:
        case TS_EVENT_CACHE_SCAN_OBJECT:
        case TS_EVENT_CACHE_SCAN_OPERATION_BLOCKED:
        case TS_EVENT_CACHE_SCAN_OPERATION_FAILED:
        case TS_EVENT_CACHE_SCAN_DONE:
        case TS_EVENT_CACHE_REMOVE:
        case TS_EVENT_CACHE_REMOVE_FAILED:
        return handleCacheScan(event, edata);

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
static int SetupRequest(TSCont contp, TSHttpTxn txnp)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    TSMBuffer bufp;
    TSMLoc hdr_loc;
    TSMLoc url_loc;
    int len;

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

    const char * s = TSUrlPathGet(bufp, url_loc, &len);
    if(!s)
    {
        TSDebug(PLUGIN_NAME, "Couldn't retrieve request path.");
        TSHandleMLocRelease(bufp, hdr_loc, url_loc);
        TSHandleMLocRelease(bufp, TS_NULL_MLOC, hdr_loc);
        TSHttpTxnReenable(txnp, TS_EVENT_HTTP_CONTINUE);
        return TS_SUCCESS;
    }
    string path(s, len);

    if(path == "cache-handler")
    {
        cstate = new cache_scan_state_t();
        TSHttpTxnIntercept(cstate->contp, txnp);
    }

    s = TSUrlHttpQueryGet(bufp, url_loc, &len);
    string query(s, len);
    if(query.length())
    {
        delete_keys_t d(query);
        d.perform_deletions();
    }
    TSHandleMLocRelease(bufp, hdr_loc, url_loc);
    TSHandleMLocRelease(bufp, TS_NULL_MLOC, hdr_loc);
    TSHttpTxnReenable(txnp, TS_EVENT_HTTP_CONTINUE);

    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TS_SUCCESS;
}

//************************************************************************************
//************************************************************************************
static int CacheHndlPlugin(TSCont contp, TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);
    TSDebug(PLUGIN_NAME, "%s: event == %d.", __FUNCTION__, event);

    switch(event)
    {
        case TS_EVENT_HTTP_READ_REQUEST_HDR:
        return SetupRequest(contp, (TSHttpTxn) edata);

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

#ifdef __ATS_6_0_0__
        TSPluginRegistrationInfo info;
        info.plugin_name = (char*) "cache-hndl-v1";
        info.vendor_name = (char*) "Concurrent Computer Corporation";
        info.support_email = (char *) "";

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

    TSCont contp = TSContCreate(CacheHndlPlugin, TSMutexCreate());
    TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, contp);

    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return;
}
