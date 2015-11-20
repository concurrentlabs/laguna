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
// Description: Manage Apache Traffic Server cache.
//**********************************************************************************************
#include <ccur_remap.h>

#define PLUGIN_NAME "cache-hndl"

using namespace std;

static int cache_handler(TSCont contp, TSEvent event, void * edata);

//**********************************************************************************************
// struct: cache_scan_state_t
//
// desctiption: encapsutate cache scan functionality.
//**********************************************************************************************
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
        contp(TSContCreate(cache_handler, TSMutexCreate())),
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

//**********************************************************************************************
// class: delete_keys_t
//
// desctiption: encapsutate cache delete functionality.
//**********************************************************************************************
class delete_keys_t
{
    public:
    //***************************************************************************
    // function: constructor
    //
    // description: split string into list of keys based on semi-colon delimiter.
    //***************************************************************************
    delete_keys_t(const string & s)
    {
        if(s.length())
        {
            boost::regex re("[;]");
            boost::sregex_token_iterator p(s.cbegin(), s.cend(), re, -1);
            boost::sregex_token_iterator e;
            for( ; p != e; ++p)
                m_keys.push_back(p->str());
        }
    }

    //***************************************************************************
    //***************************************************************************
    ~delete_keys_t() { }

    //***************************************************************************
    // function: perform_deletions
    //
    // description: delete from cache using list of keys.
    //***************************************************************************
    void perform_deletions(void)
    {
        TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

        TSCont contp = TSContCreate(cache_handler, TSMutexCreate());

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

//**********************************************************************************************
// function: scan_javascript
//
// descripion: javascript for browser interaction.
//**********************************************************************************************
void scan_javascript(void)
{
    string s = "<SCRIPT>\n"
        "var urllist = [];\n"
        "index = 0;\n"
        "function addCacheDeleteKey(input) {\n"
        "	for(c = 0; c < index; c++) {\n"
        "		if (urllist[c] == input.name) {\n"
        "			urllist.splice(c, 1);\n"
        "			index--;\n"
        "			return true;\n"
        "		}\n"
        "	}\n"
        "   urllist[index++] = input.name;\n"
        "	return true;\n"
        "}\n";

    s += "function deleteCache(form) {\n"
        "   form.elements[0].value=\"\";\n"
        "   if(index == 0) {\n"
        "       alert(\"Please select at least one url for deletion.\");\n"
        "       return false;\n"
        "   }\n"
        "   else if (index > 5) {\n"
        "       alert(\"Can't choose more than 5 urls for deletion.\");\n"
        "       return false;\n"
        "   }\n"
        "   for(c = 0; c < index; c++)\n"
        "       form.elements[0].value += urllist[c] + \";\";\n"
        "   srcfile=\"/delete-cache?urls=\" + form.elements[0].value;\n"
        "   document.location=srcfile;\n "
        "   return true;\n"
        "}\n"
        "</SCRIPT>";
    cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer, s.c_str(), s.length());
    return;
}

//**********************************************************************************************
// function: handle_cache_scan
//
// descripion: handle cache scan events.
//**********************************************************************************************
static int handle_cache_scan(TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);
    TSDebug(PLUGIN_NAME, "%s: Event: %d.", __FUNCTION__, event);

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
        scan_javascript();

        s = "<H3>Cache Contents:</H3><PRE>"
            "<TABLE>"
            "<style>"
            "table, td { border: 1px solid black; }"
            "table { border-spacing: 2px; }"
            "td { background-color: #F2EFFB; paddding: 2px; }"
            "</style>";
        cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
            s.c_str(), s.length());

        ostringstream oss;
        for(auto elem : global_cache_table)
        {
            ostringstream oss;
            oss << "<TR><TD><INPUT TYPE=CHECKBOX NAME=\"" << elem << "\"" <<
                "onClick=\"addCacheDeleteKey(this)\"></TD>";
            oss << "<TD>" << elem << "</TD></TR>";
            cstate->total_bytes += TSIOBufferWrite(cstate->resp_buffer,
                oss.str().c_str(), oss.str().length());
        }
        s = "</TABLE><BR>"
            "<FORM NAME=\"f\" ACTION=\"\" METHOD=GET>"
            "<INPUT TYPE=HIDDEN NAME=\"url\">"
            "<INPUT TYPE=button value=\"Delete\""
            "onClick=\"deleteCache(document.f)\"></PRE> </FORM>";
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
    TSDebug(PLUGIN_NAME, "Unknown event in %s: %d.", __FUNCTION__, event);
    return -1;
}

//**********************************************************************************************
// function: handle_io
//
// descripion: handle io events.
//**********************************************************************************************
static int handle_io(TSEvent event, void * edata)
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

//**********************************************************************************************
// function: cache_handler
//
// descripion: dispatch events to accept, io and cache handlers.
//**********************************************************************************************
static int cache_handler(TSCont contp, TSEvent event, void * edata)
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
        return handle_io(event, edata);

        case TS_EVENT_CACHE_SCAN:
        case TS_EVENT_CACHE_SCAN_FAILED:
        case TS_EVENT_CACHE_SCAN_OBJECT:
        case TS_EVENT_CACHE_SCAN_OPERATION_BLOCKED:
        case TS_EVENT_CACHE_SCAN_OPERATION_FAILED:
        case TS_EVENT_CACHE_SCAN_DONE:
        return handle_cache_scan(event, edata);

        case TS_EVENT_CACHE_REMOVE:
        case TS_EVENT_CACHE_REMOVE_FAILED:
        return TS_SUCCESS;

        case TS_EVENT_ERROR:
        TSDebug(PLUGIN_NAME, "%s: TS_EVENT_ERROR.", __FUNCTION__);
        break;

        default:
        TSDebug(PLUGIN_NAME, "%s: Unexpected event: %d", __FUNCTION__, event);
        break;
    }
    return TS_SUCCESS;
}

//**********************************************************************************************
// function: setup_request
//
// descripion: setup either cache scan of cache delete requests.
//**********************************************************************************************
static int setup_request(TSHttpTxn txnp)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    TSMBuffer bufp;
    TSMLoc hdr_loc, url_loc;
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

    if(path == "cache-handler" || path == "delete-cache")
    {
        cstate = new cache_scan_state_t();
        TSHttpTxnIntercept(cstate->contp, txnp);
    }

    if(path == "delete-cache")
    {
        s = TSUrlHttpQueryGet(bufp, url_loc, &len);
        string query(s, len);
        if(query.length())
        {
            boost::smatch matches;
            if(boost::regex_match(query, matches, boost::regex("urls=(.*)$")))
            {
                delete_keys_t d(matches[1].str());
                d.perform_deletions();
            }
        }
    }
    TSHandleMLocRelease(bufp, hdr_loc, url_loc);
    TSHandleMLocRelease(bufp, TS_NULL_MLOC, hdr_loc);
    TSHttpTxnReenable(txnp, TS_EVENT_HTTP_CONTINUE);

    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TS_SUCCESS;
}

//**********************************************************************************************
//**********************************************************************************************
static int CacheHndlPlugin(TSCont contp, TSEvent event, void * edata)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);
    TSDebug(PLUGIN_NAME, "%s: event == %d.", __FUNCTION__, event);

    switch(event)
    {
        case TS_EVENT_HTTP_READ_REQUEST_HDR:
        return setup_request((TSHttpTxn) edata);

        default:
        TSDebug(PLUGIN_NAME, "%s: Unexpected event: %d.", __FUNCTION__, event);
        break;
    }
    TSHttpTxnReenable((TSHttpTxn) edata, TS_EVENT_HTTP_CONTINUE);
    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TS_SUCCESS;
}

//**********************************************************************************************
// function: TSRemapInit
//
// descripion: required for remap plugin.
//**********************************************************************************************
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

//**********************************************************************************************
// function: TSRemapNewInstance
//
// descripion: required for remap plugin.
//**********************************************************************************************
TSReturnCode TSRemapNewInstance(int argc, char *argv[], void **ih,
    char *errbuf, int errbuf_size)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    string path = TSConfigDirGet();
    path += "/";
    if(argc >= 3)
        path += argv[2];
    else
        path += "cacheurl.config";
    global_remap_table.load_config_file(path);

    TSCont contp = TSContCreate(CacheHndlPlugin, NULL);
    TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, contp);

    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TS_SUCCESS;
}

//**********************************************************************************************
// function: TSRemapDoRemap
//
// descripion: required for remap plugin.
//**********************************************************************************************
TSRemapStatus TSRemapDoRemap(void *ih, TSHttpTxn rh, TSRemapRequestInfo * rri)
{
    TSDebug(PLUGIN_NAME, "Enter: %s.", __FUNCTION__);

    TSDebug(PLUGIN_NAME, "Exit: %s.", __FUNCTION__);
    return TSREMAP_NO_REMAP;
}
