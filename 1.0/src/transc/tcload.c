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

#include <fcntl.h>
#include "tcldcfg.h"
#include "tcutils.h"
/**************** PRIVATE Functions **********************/

/***************************************************************************
 * function: _tcLoadYamlValidateMonIntf
 *
 * description: Validates Monitoring interface configuration.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcLoadYamlValidateMonIntf(
        tc_utils_keyvalue_t*      pMonDirKeyValTbl,
        U16                      nMonDirKeyValTblActive,
        tc_utils_keyvalue_t*      pMonRuleseKeyValTbl,
        U16                      nMonRuleseKeyValTblActive)
{
    tresult_t   _result;
    U16         _i;
    BOOL        _bFnd;
    U16         _j;

    do
    {
       _result = ESUCCESS;
       /* Check if each monitor interface key has the same key on the
        * pcap-filter
        */
       for(_i=0;_i<nMonDirKeyValTblActive;_i++)
       {
           _bFnd = FALSE;
           if((ccur_strcasecmp(pMonDirKeyValTbl[_i].strValue,"rx")) &&
              (ccur_strcasecmp(pMonDirKeyValTbl[_i].strValue,"tx")) &&
              (ccur_strcasecmp(pMonDirKeyValTbl[_i].strValue,"rxtx")))
           {
               _result = EFAILURE;
               break;
           }
           for(_j=0;_j<nMonRuleseKeyValTblActive;_j++)
           {
               if(!strcmp(pMonDirKeyValTbl[_i].strKey,
                          pMonRuleseKeyValTbl[_j].strKey))
               {
                   _bFnd = TRUE;
                   break;
               }
           }
           if(FALSE == _bFnd)
           {
               _result = EFAILURE;
               break;
           }
       }
       if(ESUCCESS != _result)
           break;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: _tcLoadYamlValidateMainArgs
 *
 * description: Validates all console arguments
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcLoadYamlValidateMainArgs(
        evlog_t*                    pEvLogQ,
        evlog_desc_t*               pLogDescSys,
        tc_ldcfg_conf_t*            pLdCfg)
{
    tresult_t                   _result;
    tresult_t                   _sts = ESUCCESS;
    CHAR*                       _pch;
    U16                         _i;
    /*U16                         _nIngressTotal = 0;*/
    U16                         _nEgressTotal = 0;
    I32                         _tmpMacAddr[6];
    tc_utils_keyvalue_t*         _pKeyValTbl = NULL;
    U16                         _nKeyValTblActive = 0;
    tc_utils_keyvalue_t*         _pMonRuleseKeyValTbl = NULL;
    U16                         _nMonRuleseKeyValTblActive = 0;

    do
    {
        /* Feel free to add validations here... */
        _result = EFAILURE;
        _pKeyValTbl = (tc_utils_keyvalue_t*)
                malloc(TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
        _pMonRuleseKeyValTbl = (tc_utils_keyvalue_t*)
                malloc(TRANSC_INTERFACE_FLTR_MAX*sizeof(tc_utils_keyvalue_t));
        if(!_pKeyValTbl || !_pMonRuleseKeyValTbl)
            break;
        if('\0' == pLdCfg->strCmdArgMonIntf[0])
        {
            evLogTrace(
                pEvLogQ,
                evLogLvlWarn,
                pLogDescSys,
                "Warning, invalid -i or "
                "monitoringinterface config is set to: null");
        }
        else if('\0' == pLdCfg->strCmdArgPcapFilterRules[0])
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlWarn,
                    pLogDescSys,
                    "Warning, invalid -r or "
                    "pcap-filter config is set to: null");
        }
        else
        {
            if(strchr(pLdCfg->strCmdArgMonIntf,':')  &&
               strchr(pLdCfg->strCmdArgPcapFilterRules,':'))
            {
                ccur_memclear(_pKeyValTbl,
                        TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgMonIntf);
                ccur_memclear(_pMonRuleseKeyValTbl,
                        TRANSC_INTERFACE_FLTR_MAX*sizeof(tc_utils_keyvalue_t));
                tcUtilsPopulateKeyValuePair(_pMonRuleseKeyValTbl,
                            &(_nMonRuleseKeyValTblActive),
                            TRANSC_INTERFACE_FLTR_MAX,
                            pLdCfg->strCmdArgPcapFilterRules);
                for(_i=0;_i<_nKeyValTblActive;_i++)
                {
                    if('0' == _pKeyValTbl[_i].strValue[0])
                    {
                        evLogTrace(
                            pEvLogQ,
                            evLogLvlFatal,
                            pLogDescSys,
                            "Warning, invalid monitoring interface value(s) or filter(s): \"%s:%s\"",
                            _pKeyValTbl[_i].strKey,_pKeyValTbl[_i].strValue);
                        _sts = EINVAL;
                        break;
                    }
                }
            }
            else
            {
                evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "invalid monitoring interface value(s) or filter(s): \"%s/%s\"",
                    pLdCfg->strCmdArgMonIntf,pLdCfg->strCmdArgPcapFilterRules);
                _sts = EINVAL;
            }
            if(EINVAL == _sts)
                break;
            _sts = _tcLoadYamlValidateMonIntf(
                    _pKeyValTbl,
                    _nKeyValTblActive,
                    _pMonRuleseKeyValTbl,
                    _nMonRuleseKeyValTblActive);
            if(ESUCCESS != _sts)
            {
                evLogTrace(
                       pEvLogQ,
                       evLogLvlFatal,
                       pLogDescSys,
                       "failure to configure monitoring interface settings "
                       "possible mismatch failure found within "
                       "monitoring interface and pcap filter values");
                break;
            }
            /*_nIngressTotal = _nKeyValTblActive;*/
        }
        if('\0' != pLdCfg->strCmdArgOutIntfDestMacAddr[0])
        {
           if( 6 != sscanf(pLdCfg->strCmdArgOutIntfDestMacAddr,
                   "%x:%x:%x:%x:%x:%x",
                   &_tmpMacAddr[0],
                   &_tmpMacAddr[1],
                   &_tmpMacAddr[2],
                   &_tmpMacAddr[3],
                   &_tmpMacAddr[4],
                   &_tmpMacAddr[5]))
           {
               evLogTrace(
                      pEvLogQ,
                      evLogLvlFatal,
                      pLogDescSys,
                      "failure to load Destination MAC addr");
               break;
           }
        }
        if('\0' != pLdCfg->strCmdArgOutIntfSrcMacAddr[0])
        {
           if( 6 != sscanf(pLdCfg->strCmdArgOutIntfSrcMacAddr,
                   "%x:%x:%x:%x:%x:%x",
                   &_tmpMacAddr[0],
                   &_tmpMacAddr[1],
                   &_tmpMacAddr[2],
                   &_tmpMacAddr[3],
                   &_tmpMacAddr[4],
                   &_tmpMacAddr[5]))
           {
               evLogTrace(
                      pEvLogQ,
                      evLogLvlFatal,
                      pLogDescSys,
                      "failure to load Source MAC addr");
               break;
           }
        }
        if('\0' == pLdCfg->strCmdArgModeOfOperation[0])
        {
            evLogTrace(
                pEvLogQ,
                evLogLvlFatal,
                pLogDescSys,
                "Error, invalid -m or "
                "modeofoperation config must be specified");
            break;
        }
        else
        {
            /* mode of operation is either active or passive */
            if(!(0 == strcmp(pLdCfg->strCmdArgModeOfOperation,"active") ||
                0 == strcmp(pLdCfg->strCmdArgModeOfOperation,"monitor")))
            {
                evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "Error, invalid -m or "
                    "modeofoperation config : %s",
                    pLdCfg->strCmdArgModeOfOperation);
                break;
            }
        }
        /*
         * blacklist table is optional, it's not checked */
        if('\0' == pLdCfg->strCmdArgIpBlackList[0])
        {
            evLogTrace(
                pEvLogQ,
                evLogLvlInfo,
                pLogDescSys,
                "warning, ipblacklist config is not specified");
        }
        /*
         * bwsimulationoutgoinginterface is optional, it's not checked */
        if('\0' == pLdCfg->strCmdArgSimBwOutIntf[0])
        {
            evLogTrace(
                pEvLogQ,
                evLogLvlWarn,
                pLogDescSys,
                "warning, sim outgoing interface is not specified");
        }
        /* ---- Initialize Outgoing interface --- */
        if('\0' == pLdCfg->strCmdArgOutIntf[0])
        {
            evLogTrace(
                pEvLogQ,
                evLogLvlWarn,
                pLogDescSys,
                "Warning, invalid -o or "
                "outgoinginterface config is set to: null");
        }
        else
        {
            _pch = strchr(pLdCfg->strCmdArgOutIntf,':');
            if(_pch)
            {
                ccur_memclear(_pKeyValTbl,
                        TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgOutIntf);
                for(_i=0;_i<_nKeyValTblActive;_i++)
                {
                    if('0' == _pKeyValTbl[_i].strValue[0])
                    {
                        evLogTrace(
                            pEvLogQ,
                            evLogLvlFatal,
                            pLogDescSys,
                            "invalid output interface value: \"%s:%s\"",
                            _pKeyValTbl[_i].strKey,_pKeyValTbl[_i].strValue);
                        _sts = EINVAL;
                        break;
                    }
                    if(!(!ccur_strcasecmp(_pKeyValTbl[_i].strValue,"router")) &&
                       !(!ccur_strcasecmp(_pKeyValTbl[_i].strValue,"other")))
                    {
                        evLogTrace(
                                pEvLogQ,
                                evLogLvlFatal,
                                pLogDescSys,
                            "unknown outgoing interface value type specified: %s",
                            _pKeyValTbl[_i].strValue);
                        _sts = EINVAL;
                        break;
                    }
                }
            }
            else
            {
                evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "invalid outgoing interface specified: %s",
                    pLdCfg->strCmdArgOutIntf);
                _sts = EINVAL;
            }
            if(EINVAL == _sts)
                break;
            _nEgressTotal = _nKeyValTblActive;
        }
        if('\0' != pLdCfg->strCmdArgOutIntfSrcMacAddr[0])
        {
            _pch = strchr(pLdCfg->strCmdArgOutIntfSrcMacAddr,':');
            if(_pch)
            {
                ccur_memclear(_pKeyValTbl,
                        TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgOutIntfSrcMacAddr);
                for(_i=0;_i<_nKeyValTblActive;_i++)
                {
                    if('0' == _pKeyValTbl[_i].strValue[0])
                    {
                        evLogTrace(
                            pEvLogQ,
                            evLogLvlFatal,
                            pLogDescSys,
                            "invalid output src MAC Addr value: \"%s:%s\"",
                            _pKeyValTbl[_i].strKey,_pKeyValTbl[_i].strValue);
                        _sts = EINVAL;
                        break;
                    }
                }
            }
            else
            {
                evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "invalid output src MAC Addr: %s",
                    pLdCfg->strCmdArgOutIntfSrcMacAddr);
                _sts = EINVAL;
            }
            if(EINVAL == _sts)
                break;
        }
        if('\0' != pLdCfg->strCmdArgOutIntfDestMacAddr[0])
        {
            _pch = strchr(pLdCfg->strCmdArgOutIntfDestMacAddr,':');
            if(_pch)
            {
                ccur_memclear(_pKeyValTbl,
                        TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgOutIntfDestMacAddr);
                for(_i=0;_i<_nKeyValTblActive;_i++)
                {
                    if('0' == _pKeyValTbl[_i].strValue[0])
                    {
                        evLogTrace(
                            pEvLogQ,
                            evLogLvlFatal,
                            pLogDescSys,
                            "invalid output dst MAC Addr value: \"%s:%s\"",
                            _pKeyValTbl[_i].strKey,_pKeyValTbl[_i].strValue);
                        _sts = EINVAL;
                        break;
                    }
                }
            }
            else
            {
                evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "invalid output dst MAC Addr: %s",
                    pLdCfg->strCmdArgOutIntfDestMacAddr);
                _sts = EINVAL;
            }
            if(EINVAL == _sts)
                break;
        }
        if('\0' != pLdCfg->strCmdArgRedirTarget[0])
        {
            /* argument Allowed without ':' */
            _pch = strchr(pLdCfg->strCmdArgRedirTarget,':');
            if(_pch)
            {
                ccur_memclear(_pKeyValTbl,
                        TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgRedirTarget);
                for(_i=0;_i<_nKeyValTblActive;_i++)
                {
                    if('0' == _pKeyValTbl[_i].strValue[0])
                    {
                        evLogTrace(
                            pEvLogQ,
                            evLogLvlFatal,
                            pLogDescSys,
                            "invalid output Redir Addr label value: \"%s:%s\"",
                            _pKeyValTbl[_i].strKey,_pKeyValTbl[_i].strValue);
                        _sts = EINVAL;
                        break;
                    }
                }
            }
            else
            {
                evLogTrace(
                    pEvLogQ,
                    evLogLvlWarn,
                    pLogDescSys,
                    "Warning, Redir Addr has no ':' specified: %s."
                    "Please specify specific value if interface mapping"
                    "is being done",
                    pLdCfg->strCmdArgRedirTarget);
            }
            if(EINVAL == _sts)
                break;
#if 0
            if(_nIngressTotal != _nKeyValTblActive)
            {
                evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "invalid, must specify all redirection addresses if "
                    "monitoring interface > 1");
                break;
            }
#endif
        }
        else
        {
            evLogTrace(
                pEvLogQ,
                evLogLvlFatal,
                pLogDescSys,
                "Error, invalid -t or "
                "redirectaddress config must be specified");
            _sts = EINVAL;
            break;
        }
        if('\0' == pLdCfg->strCmdArgMapIntf[0])
        {
            /* no mapping with n-egress */
            if( _nEgressTotal > 1)
            {
                evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "invalid map interface "
                    "values must be specified if "
                    "nth outgoing interfaces are "
                    "specified.");
                break;
            }
        }
        else
        {
            /* argument Allowed without ':' */
            _pch = strchr(pLdCfg->strCmdArgMapIntf,':');
            if(_pch)
            {
                ccur_memclear(_pKeyValTbl,
                        TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgMapIntf);
                for(_i=0;_i<_nKeyValTblActive;_i++)
                {
                    if('0' == _pKeyValTbl[_i].strValue[0])
                    {
                        evLogTrace(
                            pEvLogQ,
                            evLogLvlFatal,
                            pLogDescSys,
                            "invalid Map interface value: \"%s:%s\"",
                            _pKeyValTbl[_i].strKey,_pKeyValTbl[_i].strValue);
                        _sts = EINVAL;
                        break;
                    }
                }
            }
            else
            {
                evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "invalid Map interface value: %s",
                    pLdCfg->strCmdArgMapIntf);
                _sts = EINVAL;
                break;
            }
            if(EINVAL == _sts)
                break;
        }
        _result = ESUCCESS;
    }while(FALSE);

    if(_pKeyValTbl)
        free(_pKeyValTbl);
    if(_pMonRuleseKeyValTbl)
        free(_pMonRuleseKeyValTbl);

    return _result;
}

/***************************************************************************
 * function: _tcLoadYamlValidateRoutingArgs
 *
 * description: Validates the arguments other than console from config.yaml.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcLoadYamlValidateRoutingArgs(
        evlog_t*                        pEvLogQ,
        evlog_desc_t*                   pLogDescSys,
        tc_ldcfg_conf_t*                pLdCfg)
{
    tresult_t   _result;
    int        _nSitesIdx;
    U32         _nIdx;

    do
    {
        _result = ESUCCESS;
        /* Feel free to add validations here... */
        if('\0' == pLdCfg->strVer[0] ||
           !tcUtilsValidConfigVersion(pLdCfg->strVer))
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "Config file load failure, incompatible version!"
                    " executable is ver %s while config.yaml is"
                    " ver %s\n",TRANSC_MAIN_CONF_VER,pLdCfg->strVer);
            _result = EINVAL;
            break;
        }
        if(0 == pLdCfg->nSites)
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "Error, invalid sites not specified\n");
            _result = EINVAL;
            break;
        }
        if(pLdCfg->nSites <= 0 ||
                pLdCfg->nSites >= TRANSC_LDCFG_SITE_MAXSITES_LST)
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                    "Error, internal sites array out of bound\n");
            _result = EINVAL;
            break;
        }
        /* Validate the rest of the config */
        for(_nSitesIdx=1;_nSitesIdx<pLdCfg->nSites;_nSitesIdx++)
        {
            if('\0' == pLdCfg->tSites[_nSitesIdx].strCfgArgSites[0])
            {
                evLogTrace(
                        pEvLogQ,
                        evLogLvlFatal,
                        pLogDescSys,
                        "Error, invalid sites string not specified\n");
                _result = EINVAL;
                break;
            }
            if('\0' == pLdCfg->tSites[_nSitesIdx].strCfgArgOpt[0])
            {
                evLogTrace(
                        pEvLogQ,
                        evLogLvlFatal,
                        pLogDescSys,
                        "Error, invalid option string not specified\n");
                _result = EINVAL;
                break;
            }
            /*
             * Host is optional, it's not checked
            if('\0' == pLdCfg->tSites[_nSitesIdx].strCfgArgHost[0])
            {
                evLogTrace(
                    pEvLogQ,
                evLogLvlWarn,
                            pLogDescSys,
                        "Error, invalid host string not specified\n");
                _result = EINVAL;
                break;
            }*/
            /*
             * Referrer is optional, it's not checked
            if('\0' == pLdCfg->tSites[_nSitesIdx].strCfgArgReferrer[0])
            {
                evLogTrace(
                    pEvLogQ,
                evLogLvlWarn,
                            pLogDescSys,
                        "Error, invalid referrer string not specified\n");
                _result = EINVAL;
                break;
            }*/
            /*
             * httprangefield is optional, it's not checked
            if('\0' == pLdCfg->tSites[_nSitesIdx].strCfgArgHttpRangeField[0])
            {
                evLogTrace(
                    pEvLogQ,
                evLogLvlWarn,
                            pLogDescSys,
                        "Error, invalid Http Range string not specified\n");
                _result = EINVAL;
                break;
            }*/
            /*
             * httpsessionfield is optional, it's not checked
            if('\0' == pLdCfg->tSites[_nSitesIdx].strCfgArgHttpSessField[0])
            {
                evLogTrace(
                    pEvLogQ,
                evLogLvlWarn,
                            pLogDescSys,
                        "Error, invalid Http Session string not specified\n");
                _result = EINVAL;
                break;
            }*/
            if('\0' ==
                    pLdCfg->tSites[_nSitesIdx].strCfgArgType[0])
            {
                evLogTrace(
                        pEvLogQ,
                        evLogLvlFatal,
                        pLogDescSys,
                        "Error, invalid sites type\n");
                _result = EINVAL;
                break;
            }
            /* Uri */
            if(pLdCfg->tSites[_nSitesIdx].nCfgArgURI <= 0 ||
                    pLdCfg->tSites[_nSitesIdx].nCfgArgURI >=
                    TRANSC_LDCFG_SITE_MAXSITES_LST)
            {
                evLogTrace(
                        pEvLogQ,
                        evLogLvlFatal,
                        pLogDescSys,
                        "Error, internal sites array out of bound\n");
                _result = EINVAL;
                break;
            }
            for(_nIdx=0;_nIdx < pLdCfg->tSites[_nSitesIdx].nCfgArgURI;_nIdx++)
            {
                if('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nIdx].strCfgArgUriSig[0])
                {
                   evLogTrace(
                           pEvLogQ,
                           evLogLvlFatal,
                           pLogDescSys,
                       "Error, invalid URI sig string not specified\n");
                   _result = EINVAL;
                   break;
                }
                if('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nIdx].strCfgArgMatchSz[0])
                {
                   evLogTrace(
                           pEvLogQ,
                           evLogLvlFatal,
                           pLogDescSys,
                           "Error, invalid matchsize string not specified\n");
                   _result = EINVAL;
                   break;
                }
                if(('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nIdx].strCfgArgCKeyId[0]) &&
                   ('\0' ==
                           pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nIdx].strCfgArgCtxId[0]))
                {
                   evLogTrace(
                           pEvLogQ,
                           evLogLvlFatal,
                           pLogDescSys,
                           "Error, invalid Cache key string not specified\n");
                   _result = EINVAL;
                   break;
                }
                /*
                 * cache key cachekeyrange is optional, it's not checked
                if('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nIdx].strCfgArgCKeyRange[0])
                {
                   evLogTrace(
                    pEvLogQ,
                   evLogLvlWarn,
                               pLogDescSys,
                           "Error, invalid Cache key Range string is not specified\n");
                   _result = EINVAL;
                   break;
                }*/
                /*
                 * cache key misc is optional, it's not checked
                if('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nIdx].strCfgArgCKeyMisc[0])
                {
                   evLogTrace(
                    pEvLogQ,
                   evLogLvlWarn,
                               pLogDescSys,
                           "Error, invalid Cache key misc string is not specified\n");
                   _result = EINVAL;
                   break;
                }*/
            }
            if(ESUCCESS != _result )
                break;
            /* Session is Optional */
            if(pLdCfg->tSites[_nSitesIdx].nCfgArgSess >=
                                TRANSC_LDCFG_SITE_MAXSITES_LST)
            {
                evLogTrace(
                        pEvLogQ,
                        evLogLvlFatal,
                        pLogDescSys,
                        "Error, internal sites array out of bound\n");
                _result = EINVAL;
                break;
            }
            for(_nIdx=0;_nIdx < pLdCfg->tSites[_nSitesIdx].nCfgArgSess;_nIdx++)
            {
                if('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nIdx].strCfgArgSessSig[0])
                {
                   evLogTrace(
                           pEvLogQ,
                           evLogLvlFatal,
                           pLogDescSys,
                       "Error, invalid Sess sig string not specified\n");
                   _result = EINVAL;
                   break;
                }
                if('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nIdx].strCfgArgMatchSz[0])
                {
                   evLogTrace(
                           pEvLogQ,
                           evLogLvlFatal,
                           pLogDescSys,
                           "Error, invalid matchsize string not specified\n");
                   _result = EINVAL;
                   break;
                }
                if(('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nIdx].strCfgArgCKeyId[0]) &&
                   ('\0' ==
                           pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nIdx].strCfgArgCtxId[0]))
                {
                   evLogTrace(
                           pEvLogQ,
                           evLogLvlFatal,
                           pLogDescSys,
                           "Error, invalid Cache key string not specified\n");
                   _result = EINVAL;
                   break;
                }
                if('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nIdx].strCfgArgCtxId[0])
                {
                   evLogTrace(
                           pEvLogQ,
                           evLogLvlFatal,
                           pLogDescSys,
                           "Error, invalid Ctx id string not specified\n");
                   _result = EINVAL;
                   break;
                }
                /*
                 * cache key cachekeyrange is optional, it's not checked
                if('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nIdx].strCfgArgCKeyRange[0])
                {
                   evLogTrace(
                    pEvLogQ,
                   evLogLvlWarn,
                               pLogDescSys,
                           "Error, invalid Cache key Range string is not specified\n");
                   _result = EINVAL;
                   break;
                }*/
                /*
                 * cache key misc is optional, it's not checked
                if('\0' ==
                        pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nIdx].strCfgArgCKeyMisc[0])
                {
                   evLogTrace(
                    pEvLogQ,
                   evLogLvlWarn,
                   pLogDescSys,
                           "Error, invalid Cache key misc string is not specified\n");
                   _result = EINVAL;
                   break;
                }*/
            }
            if(ESUCCESS != _result )
                break;
        }
        if(ESUCCESS != _result )
            break;
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

CCUR_PRIVATE(tresult_t)
_tcLoadYamlInit(FILE*             f,
        yaml_parser_t*           prs)
{
    tresult_t       _result;

    do
    {
        /* Create the Parser object. */
        _result = (tresult_t)yaml_parser_initialize(prs);
        if(1 != _result)
            break;
        yaml_parser_set_input_file(prs, f);
    }while(FALSE);

    if(1 == _result)
        _result = ESUCCESS;
    else
        _result = EFAILURE;

    return _result;
}

/***************************************************************************
 * function: tcLoadYamlRead
 *
 * description: unmarshall config.yaml file.
 ***************************************************************************/
CCUR_PRIVATE(tresult_t)
_tcLoadYamlRead(
        evlog_t*                        pEvLogQ,
        evlog_desc_t*                   pLogDescSys,
        yaml_parser_t*                  prs,
        FILE*                           file,
        tc_ldcfg_conf_t*                pLdCfg,
        BOOL                            bLock)
{
    tresult_t       _result;
    yaml_event_t    _event;
    U8              _nStkDepth  = 0;
    S32             _nSitesIdx  = 0;
    S32             _nUrlIdx    = -1;
    S32             _nSessIdx   = -1;
    CHAR*           _pStrBuff   = NULL;
    U32*            _pnStrSig   = NULL;
    U32             _nStrSz     = 0;
    S32             _done       = 0;
    BOOL            _bValSkip   = FALSE;
    /* Debug */
    U8              _i;
    CHAR            _pTab[256];
    CHAR            _pStrScalar[256];
    CHAR            _pStrServiceType[256];
    struct flock    _lock;

    CCURASSERT(pLdCfg);
    CCURASSERT(prs);

    ccur_memclear(_pTab,sizeof(_pTab));
    /* Read the event sequence. */
    _result = ESUCCESS;
    if(bLock)
    {
        ccur_memclear(&_lock,sizeof(_lock));
        _lock.l_type = F_RDLCK;
         /* Place a write lock on the file. */
         fcntl(fileno(file), F_SETLKW, &_lock);
    }
    while (!_done)
    {
        /* Get the next event. */
        if (!yaml_parser_parse(prs, &_event))
        {
            _result = EFAILURE;
            evLogTrace(
                    pEvLogQ,
                    evLogLvlError,
                    pLogDescSys,
                    "error loading yaml config file, %s",
                    (CHAR*)prs->problem);
            break;
        }
        switch(_event.type)
        {
            /** An empty event. */
            case  YAML_NO_EVENT:
                break;
            case YAML_STREAM_START_EVENT:
                evLogTrace(
                        pEvLogQ,
                        evLogLvlDebug,
                        pLogDescSys,
                        "YAML_STREAM_START_EVENT");
                _done = 0;
                break;
            case YAML_STREAM_END_EVENT:
                evLogTrace(
                        pEvLogQ,
                        evLogLvlDebug,
                        pLogDescSys,
                        "YAML_STREAM_END_EVENT");
                _done = _event.type;
                break;
            case YAML_DOCUMENT_START_EVENT:
                evLogTrace(
                        pEvLogQ,
                        evLogLvlDebug,
                        pLogDescSys,
                        "YAML_DOCUMENT_START_EVENT");
                break;
            case YAML_DOCUMENT_END_EVENT:
                evLogTrace(
                        pEvLogQ,
                        evLogLvlDebug,
                        pLogDescSys,
                        "YAML_DOCUMENT_END_EVENT");
                break;
            case YAML_ALIAS_EVENT:
                /*evLogTrace(
                        evLogLvlDebug,
                        pLogDescSys,
                 * "YAML_ALIAS_EVENT");*/
                break;
            case YAML_SCALAR_EVENT:
                if(_bValSkip)
                {
                    _bValSkip   = FALSE;
                    _pnStrSig   = NULL;
                    _pStrBuff   = NULL;
                    _nStrSz     = 0;
                    break;
                }
                else if(_event.data.scalar.value)
                {
                    if(_pStrBuff)
                    {
                        if(_event.data.scalar.value)
                        {
                            if(strlen((CHAR*)(_event.data.scalar.value)) < _nStrSz )
                            {
                                ccur_strlcpy(_pStrBuff,
                                        (CHAR*)(_event.data.scalar.value),_nStrSz);
                                if(_pnStrSig)
                                    *_pnStrSig = strlen((CHAR*)_event.data.scalar.value);
                                evLogTrace(
                                        pEvLogQ,
                                        evLogLvlDebug,
                                        pLogDescSys,
                                        "%s%s:%s    - sIdx:%i|urlIdx:%i",
                                        _pTab,_pStrScalar,_pStrBuff,_nSitesIdx,_nUrlIdx);
                                if('\0' != _pStrBuff[0])
                                {
                                    if(!strncasecmp(_pStrBuff,"null",sizeof("null")))
                                        ccur_memclear(_pStrBuff,sizeof("null"));
                                }
                            }
                            else
                            {
                                _result = EINVAL;
                                _done = TRUE;
                            }
                        }
                        else
                        {
                            _result = EINVAL;
                            _done = TRUE;
                        }
                        _pnStrSig   = NULL;
                        _pStrBuff   = NULL;
                        _nStrSz     = 0;

                    }
                    else
                    {
                        switch(_nStkDepth)
                        {
                            case 0:
                                ccur_strlcpy(_pStrScalar,
                                        (CHAR*)_event.data.scalar.value,sizeof(_pStrScalar));
                                if(!strncasecmp(
                                        (CHAR*)_event.data.scalar.value,"services",sizeof("services")))
                                {
                                    evLogTrace(
                                            pEvLogQ,
                                            evLogLvlDebug,
                                            pLogDescSys,
                                            "%s%s:",_pTab,_pStrScalar);
                                    ccur_strlcat(_pTab,"    ",sizeof(_pTab));
                                }
                                else
                                {
                                    if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"version",sizeof("version")))
                                    {
                                        _pStrBuff   = &(pLdCfg->strVer[0]);
                                        _nStrSz     = sizeof(pLdCfg->strVer);
                                        _bValSkip   = pLdCfg->bStrVerSkip;
                                        pLdCfg->bStrVerSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"monitoringinterface",
                                            sizeof("monitoringinterface")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgMonIntf[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgMonIntf);
                                        _bValSkip       = pLdCfg->bStrCmdArgMonIntfSkip;
                                        pLdCfg->bStrCmdArgMonIntfSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"outgoinginterfaceaddmplstag",
                                            sizeof("outgoinginterfaceaddmplstag")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgOutIntfMplsLabel[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgOutIntfMplsLabel);
                                        _bValSkip       = pLdCfg->bstrCmdArgOutIntfMplsLabelSkip;
                                        pLdCfg->bstrCmdArgOutIntfMplsLabelSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"processproxyrequest",
                                            sizeof("processproxyrequest")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgProcessProxyReq[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgProcessProxyReq);
                                        _bValSkip       = pLdCfg->bstrCmdArgProcessProxyReqSkip;
                                        pLdCfg->bstrCmdArgProcessProxyReqSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"outgoinginterfaceignorecors",
                                            sizeof("outgoinginterfaceignorecors")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgIgnoreCORSReq[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgIgnoreCORSReq);
                                        _bValSkip       = pLdCfg->bstrCmdArgIgnoreCORSReqSkip;
                                        pLdCfg->bstrCmdArgIgnoreCORSReqSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"outgoingredirreqratemax",
                                            sizeof("outgoingredirreqratemax")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgOutRedirReqRateMax[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgOutRedirReqRateMax);
                                        _bValSkip       = pLdCfg->bstrCmdArgOutRedirReqRateMaxSkip;
                                        pLdCfg->bstrCmdArgOutRedirReqRateMaxSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"outgoinginterfacedestmac",
                                            sizeof("outgoinginterfacedestmac")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgOutIntfDestMacAddr[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgOutIntfDestMacAddr);
                                        _bValSkip       = pLdCfg->bstrCmdArgOutIntfDestMacAddrSkip;
                                        pLdCfg->bstrCmdArgOutIntfDestMacAddrSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                             (CHAR*)_event.data.scalar.value,"outgoinginterfacesrcmac",
                                             sizeof("outgoinginterfacesrcmac")))
                                    {
                                         _pStrBuff       = &(pLdCfg->strCmdArgOutIntfSrcMacAddr[0]);
                                         _nStrSz         = sizeof(pLdCfg->strCmdArgOutIntfSrcMacAddr);
                                         _bValSkip       = pLdCfg->bstrCmdArgOutIntfSrcMacAddrSkip;
                                         pLdCfg->bstrCmdArgOutIntfSrcMacAddrSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"outgoinginterface",sizeof("outgoinginterface")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgOutIntf[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgOutIntf);
                                        _bValSkip       = pLdCfg->bstrCmdArgOutIntfSkip;
                                        pLdCfg->bstrCmdArgOutIntfSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"pcap-filter",sizeof("pcap-filter")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgPcapFilterRules[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgPcapFilterRules);
                                        _bValSkip       = pLdCfg->bstrCmdArgPcapFilterRulesSkip;
                                        pLdCfg->bstrCmdArgPcapFilterRulesSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"redirectaddress",sizeof("redirectaddress")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgRedirTarget[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgRedirTarget);
                                        _bValSkip       = pLdCfg->bStrCmdArgRedirTargetSkip;
                                        pLdCfg->bStrCmdArgRedirTargetSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"modeofoperation",sizeof("modeofoperation")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgModeOfOperation[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgModeOfOperation);
                                        _bValSkip       = pLdCfg->bstrCmdArgModeOfOperationSkip;
                                        pLdCfg->bstrCmdArgModeOfOperationSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"ipblacklist",sizeof("ipblacklist")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgIpBlackList[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgIpBlackList);
                                        _bValSkip       = pLdCfg->bstrCmdArgIpBlackListSkip;
                                        pLdCfg->bstrCmdArgIpBlackListSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"bwsimulationmode",sizeof("bwsimulationmode")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgSimBwSimMode[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgSimBwSimMode);
                                        _bValSkip       = pLdCfg->bstrCmdArgSimBwSimModeSkip;
                                        pLdCfg->bstrCmdArgSimBwSimModeSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"bwsimulationworkers",sizeof("bwsimulationworkers")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgSimBwThreadsNum[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgSimBwThreadsNum);
                                        _bValSkip       = pLdCfg->bstrCmdArgSimBwThreadsNumSkip;
                                        pLdCfg->bstrCmdArgSimBwThreadsNumSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"bwsimulationoutgoinginterface",
                                            sizeof("bwsimulationoutgoinginterface")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgSimBwOutIntf[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgSimBwOutIntf);
                                        _bValSkip       = pLdCfg->bstrCmdArgSimBwOutIntfSkip;
                                        pLdCfg->bstrCmdArgSimBwOutIntfSkip = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"mapinterface",
                                            sizeof("mapinterface")))
                                    {
                                        _pStrBuff       = &(pLdCfg->strCmdArgMapIntf[0]);
                                        _nStrSz         = sizeof(pLdCfg->strCmdArgMapIntf);
                                        _bValSkip       = pLdCfg->bstrCmdArgMapIntfSkip;
                                        pLdCfg->bstrCmdArgMapIntfSkip = FALSE;
                                    }
                                    else
                                        _bValSkip = TRUE;
                                }
                                break;
                            case 1:
                                if(_nSitesIdx > 0 && _nSitesIdx < TRANSC_LDCFG_SITE_MAXSITES_LST)
                                {
                                    ccur_strlcpy(_pStrScalar,
                                            (CHAR*)_event.data.scalar.value,sizeof(_pStrScalar));
                                    if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"url",sizeof("url")))
                                    {
                                        evLogTrace(
                                                pEvLogQ,
                                                evLogLvlDebug,
                                                pLogDescSys,
                                                "%s%s:",_pTab,_pStrScalar);
                                        ccur_strlcat(_pTab,"    ",sizeof(_pTab));
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"session",sizeof("session")))
                                    {
                                        evLogTrace(
                                                pEvLogQ,
                                                evLogLvlDebug,
                                                pLogDescSys,
                                                "%s%s:",_pTab,_pStrScalar);
                                        ccur_strlcat(_pTab,"    ",sizeof(_pTab));
                                    }
                                    else
                                    {
                                        if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"type",sizeof("type")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].strCfgArgType[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].strCfgArgType);
                                            _bValSkip       = pLdCfg->tSites[_nSitesIdx].bstrCfgArgTypeSkip;
                                            pLdCfg->tSites[_nSitesIdx].bstrCfgArgTypeSkip = FALSE;
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"target",sizeof("target")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].strCfgArgSites[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].strCfgArgSites);
                                            _bValSkip       = pLdCfg->tSites[_nSitesIdx].bstrCfgArgSiteskip;
                                            pLdCfg->tSites[_nSitesIdx].bstrCfgArgSiteskip = FALSE;
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"options",sizeof("options")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].strCfgArgOpt[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].strCfgArgOpt);
                                            _bValSkip       = pLdCfg->tSites[_nSitesIdx].bstrCfgArgOptSkip;
                                            pLdCfg->tSites[_nSitesIdx].bstrCfgArgOptSkip = FALSE;
                                        }
                                        else if(!strncasecmp(
                                                   (CHAR*)_event.data.scalar.value,"httphdrmatchsignature",
                                                   sizeof("httphdrmatchsignature")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].strCfgArgHttpHdrMatch[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].strCfgArgHttpHdrMatch);
                                            _bValSkip       = pLdCfg->tSites[_nSitesIdx].bstrCfgArgHttpHdrMatchSkip;
                                            pLdCfg->tSites[_nSitesIdx].bstrCfgArgHttpHdrMatchSkip = FALSE;
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"hostsignature",sizeof("hostsignature")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].strCfgArgHost[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].strCfgArgHost);
                                            _bValSkip       = pLdCfg->tSites[_nSitesIdx].bstrCfgArgHostSkip;
                                            pLdCfg->tSites[_nSitesIdx].bstrCfgArgHostSkip = FALSE;
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"referersignature",sizeof("referersignature")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].strCfgArgReferrer[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].strCfgArgReferrer);
                                            _bValSkip       = pLdCfg->tSites[_nSitesIdx].bstrCfgArgReferrerSkip;
                                            pLdCfg->tSites[_nSitesIdx].bstrCfgArgReferrerSkip = FALSE;
                                        }
                                        else if(!strncasecmp(
                                              (CHAR*)_event.data.scalar.value,"httprangefield",sizeof("httprangefield")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].strCfgArgHttpRangeField[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].strCfgArgHttpRangeField);
                                            _bValSkip       = pLdCfg->tSites[_nSitesIdx].bstrCfgArgHttpRangeFieldSkip;
                                            pLdCfg->tSites[_nSitesIdx].bstrCfgArgHttpRangeFieldSkip = FALSE;
                                        }
                                        else
                                            _bValSkip = TRUE;
                                    }
                                }
                                else
                                {
                                    _result = ENOMEM;
                                    _done = TRUE;
                                }
                                break;
                            case 2:
                                if(_nSitesIdx > 0 && _nSitesIdx < TRANSC_LDCFG_SITE_MAXSITES_LST)
                                {
                                    if(!strcmp(_pStrServiceType,"session") &&
                                            (_nSessIdx > -1 && _nSessIdx < TRANSC_LDCFG_SITE_MAXSESSTYPE_LST))
                                    {
                                        ccur_strlcpy(_pStrScalar,
                                                (CHAR*)_event.data.scalar.value,sizeof(_pStrScalar));
                                        _bValSkip       = pLdCfg->tSites[_nSitesIdx].btCfgArgSessSkip;
                                        pLdCfg->tSites[_nSitesIdx].btCfgArgSessSkip = FALSE;
                                        if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"signature",sizeof("signature")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgSessSig[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgSessSig);
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"maxmatchsize",sizeof("maxmatchsize")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgMatchSz[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgMatchSz);
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"contextid",sizeof("contextid")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgCtxId[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgCtxId);
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"cachekeyid",sizeof("cachekeyid")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgCKeyId[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgCKeyId);
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"cachekeyrange",sizeof("cachekeyrange")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgCKeyRange[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgCKeyRange);
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"cachekeymisc",sizeof("cachekeymisc")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgCKeyMisc[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgSess[_nSessIdx].strCfgArgCKeyMisc);
                                        }
                                        else
                                            _bValSkip = TRUE;
                                    }
                                    else if(!strcmp(_pStrServiceType,"url") &&
                                            (_nUrlIdx > -1 && _nUrlIdx < TRANSC_LDCFG_SITE_MAXURITYPE_LST))
                                    {
                                        ccur_strlcpy(_pStrScalar,
                                                (CHAR*)_event.data.scalar.value,sizeof(_pStrScalar));
                                        _bValSkip       = pLdCfg->tSites[_nSitesIdx].bCfgArgURISkip;
                                        pLdCfg->tSites[_nSitesIdx].bCfgArgURISkip = FALSE;
                                        if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"signature",sizeof("signature")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgUriSig[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgUriSig);
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"maxmatchsize",sizeof("maxmatchsize")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgMatchSz[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgMatchSz);
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"contextid",sizeof("contextid")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgCtxId[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgCtxId);
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"cachekeyid",sizeof("cachekeyid")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgCKeyId[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgCKeyId);
                                        }
                                        else if(!strncasecmp(
                                             (CHAR*)_event.data.scalar.value,"cachekeyrange",sizeof("cachekeyrange")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgCKeyRange[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgCKeyRange);
                                        }
                                        else if(!strncasecmp(
                                                (CHAR*)_event.data.scalar.value,"cachekeymisc",sizeof("cachekeymisc")))
                                        {
                                            _pStrBuff       = &(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgCKeyMisc[0]);
                                            _nStrSz         = sizeof(pLdCfg->tSites[_nSitesIdx].tCfgArgURI[_nUrlIdx].strCfgArgCKeyMisc);
                                        }
                                        else
                                            _bValSkip = TRUE;
                                    }
                                    else
                                    {
                                        _result = ENOMEM;
                                        _done = TRUE;
                                    }
                                }
                                else
                                {
                                    _result = ENOMEM;
                                    _done = TRUE;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                }
                break;
            case YAML_SEQUENCE_START_EVENT:
                /*evLogTrace(
                 *      pEvLogQ,
                        evLogLvlDebug,
                                    pLogDescSys,
                 "YAML_SEQUENCE_START_EVENT");*/
                /* Push! */
                _nStkDepth++;
                ccur_strlcpy(_pStrServiceType,
                        _pStrScalar,sizeof(_pStrServiceType));
                /* debug print statements */
                _pTab[0] = '\0';
                for(_i=0;_i<_nStkDepth;_i++)
                    ccur_strlcat(_pTab,"    ",sizeof(_pTab));
                break;
            case YAML_SEQUENCE_END_EVENT:
                /*evLogTrace(
                 *      pEvLogQ,
                        evLogLvlDebug,
                                    pLogDescSys,
                  "YAML_SEQUENCE_END_EVENT");*/
                /* Pop! */
                _nStkDepth--;
                ccur_strlcpy(_pStrServiceType,
                        _pStrScalar,sizeof(_pStrServiceType));
                /* debug print statements */
                _pTab[0] = '\0';
                for(_i=0;_i<_nStkDepth;_i++)
                    ccur_strlcat(_pTab,"    ",sizeof(_pTab));
                break;
            case YAML_MAPPING_START_EVENT:
                /* Manage sites and url counters */
                if(1 == _nStkDepth)
                {
                    _nSitesIdx++;
                    pLdCfg->nSites = _nSitesIdx+1;
                    _nUrlIdx = -1;
                    _nSessIdx = -1;
                }
                if(2 == _nStkDepth)
                {
                    if(!strcmp(_pStrServiceType,"url"))
                    {
                        _nUrlIdx++;
                        if(_nSitesIdx > 0 && _nSitesIdx < TRANSC_LDCFG_SITE_MAXSITES_LST )
                            pLdCfg->tSites[_nSitesIdx].nCfgArgURI = _nUrlIdx+1;
                    }
                    else if(!strcmp(_pStrServiceType,"session"))
                    {
                        _nSessIdx++;
                        if(_nSitesIdx > 0 && _nSitesIdx < TRANSC_LDCFG_SITE_MAXSITES_LST )
                            pLdCfg->tSites[_nSitesIdx].nCfgArgSess = _nSessIdx+1;
                    }
                    else
                    {
                        _result = EFAILURE;
                        _done = TRUE;
                    }
                }
                /*evLogTrace(
                 *      pEvLogQ,
                        evLogLvlDebug,
                                    pLogDescSys,
                 "YAML_MAPPING_START_EVENT");*/
                break;
            case YAML_MAPPING_END_EVENT:
                /*evLogTrace(
                 *      pEvLogQ,
                        evLogLvlDebug,
                                    pLogDescSys,
                 "YAML_MAPPING_END_EVENT");*/
                break;
            default:
                break;
        }

        /* The application is responsible for destroying the event object. */
        yaml_event_delete(&_event);
    }

    if(bLock)
    {
        /* Release the lock. */
        _lock.l_type = F_UNLCK;
        fcntl (fileno(file), F_SETLKW, &_lock);
    }

    return _result;
}

/**************** PROTECTED Functions **********************/

/***************************************************************************
 * function: tcLoadUnmarshallConfigYaml
 *
 * description: unmarshall config.yaml file.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcLoadUnmarshallConfigYaml(
        evlog_t*                        pEvLogQ,
        evlog_desc_t*                   pLogDescSys,
        CHAR*                           strRdConfigLoc,
        tc_ldcfg_conf_t*                pLdCfg,
        yaml_parser_t*                  pYmlParser)
{
    tresult_t   _result;
    BOOL        _bYmlPrsInit;
    FILE*       _f;

    do
    {
        _f              = NULL;
        _bYmlPrsInit    = FALSE;
        _result         = EFAILURE;
        _f = fopen(strRdConfigLoc, "rb");
        if(NULL == _f)
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                   "Error, unable to open file:%s",
                   strRdConfigLoc);
            break;
        }
        _result = _tcLoadYamlInit(_f,pYmlParser);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                   "Error, trconfig read init %s\n",
                   strRdConfigLoc);
            break;
        }
        _bYmlPrsInit = TRUE;
        _result = _tcLoadYamlRead(
                pEvLogQ,
                pLogDescSys,
                pYmlParser,
                _f,
                pLdCfg,
                TRUE);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                   "Error, config read %s\n",
                   strRdConfigLoc);
            break;
        }
        _result = ESUCCESS;
    }while(FALSE);

    if(_bYmlPrsInit)
        yaml_parser_delete(pYmlParser);
    if(_f)
        fclose(_f);
    if(ESUCCESS == _result)
    {
        _result = _tcLoadYamlValidateMainArgs(
                pEvLogQ,pLogDescSys,pLdCfg);
        if(ESUCCESS == _result)
            _result = _tcLoadYamlValidateRoutingArgs(
                    pEvLogQ,pLogDescSys,pLdCfg);
    }
    if(ESUCCESS == _result)
    {
        evLogTrace(
                pEvLogQ,
                evLogLvlInfo,
                pLogDescSys,
                "Reading config from: %s success!",
                strRdConfigLoc);
    }
    else
    {
        evLogTrace(
                pEvLogQ,
                evLogLvlError,
                pLogDescSys,
                "Reading config from: %s failure!",
                strRdConfigLoc);
    }

    return _result;
}
