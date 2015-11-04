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

#include "pktprc.h"
#include "tcmapintf.h"

/***************************************************************************
 * function: _tcMonIntfFindIntfInMonIntfTbl
 *
 * description: Search ingress interface entry within ingress interface table
 ***************************************************************************/
CCUR_PRIVATE(tc_monintf_mon_t*)
_tcMonIntfFindIntfInMonIntfTbl(
        tc_monintf_intfd_t*          pIntfd,
        U16*                         pIdx,
        CHAR*                        strKey)
{
    U16                         _i;
    tc_monintf_mon_t*          _pIntf;

    _pIntf             = NULL;
    /* Find the interface name */
    for(_i=0;_i<pIntfd->nMonIntfTotal;_i++)
    {
        if(!strcmp(strKey,pIntfd->tMonIntfTbl[_i].tIntf.strIntfName) &&
           ('\0' != pIntfd->tMonIntfTbl[_i].tIntf.strIntfName[0]) &&
           ('\0' != pIntfd->tMonIntfTbl[_i].tIntf.strIntfVal[0]))
        {
            _pIntf = &(pIntfd->tMonIntfTbl[_i]);
            if(pIdx)
                *pIdx   = _i;
            break;
        }
    }

    return _pIntf;
}

/***************************************************************************
 * function: _tcIntfFindIntfInOutIntfTbl
 *
 * description: Search egress interface entry within egress interface table
 ***************************************************************************/
CCUR_PRIVATE(tc_monintf_out_t*)
_tcMonIntfFindIntfInOutIntfTbl(
        tc_monintf_intfd_t*          pIntfd,
        U16*                         pIdx,
        CHAR*                        strKey)
{
    U16                         _i;
    tc_monintf_out_t*         _pIntf;

    _pIntf             = NULL;

    /* Find the interface name */
    for(_i=0;_i<pIntfd->nOutIntfTotal;_i++)
    {
        if(!strcmp(strKey,pIntfd->tOutIntfTbl[_i].tLinkIntf.strIntfName) &&
           ('\0' != pIntfd->tOutIntfTbl[_i].tLinkIntf.strIntfName[0]))
        {
            _pIntf = &(pIntfd->tOutIntfTbl[_i]);
            if(pIdx)
                *pIdx   = _i;
            break;
        }
    }

    return _pIntf;
}

/***************************************************************************
 * function: _tcMonIntfInitMonIntfProperties
 *
 * description: Init outgoing interface properties such as Redirection addr.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcMonIntfInitMonIntfProperties(
        tc_monintf_intfd_t*          pIntfd,
        tc_utils_keyvalue_t*          pKeyValTbl,
        U16                          nKeyValTblActive,
        tc_intf_ptype_e       eIntfPTypes)
{
    U16                         _i;
    CHAR*                       _strKey;
    CHAR*                       _strVal;
    tc_monintf_mon_t*          _pMonIntf;

    for(_i=0;_i<nKeyValTblActive;_i++)
    {
        /* Find the interface name */
        _strKey = pKeyValTbl[_i].strKey;
        _strVal = pKeyValTbl[_i].strValue;
        _pMonIntf = _tcMonIntfFindIntfInMonIntfTbl(
                pIntfd,NULL,_strKey);
        if(_pMonIntf && _strVal)
        {
            switch(eIntfPTypes)
            {
                case tcIntfPTypeRedirAddr:
                    ccur_strlcpy(_pMonIntf->strRedirAddr,
                            _strVal,sizeof(_pMonIntf->strRedirAddr));
                    break;
                default:
                    break;
            }
        }
    }
}

/***************************************************************************
 * function: _tcMonIntfInitMonIntfPropertiesNtoOne
 *
 * description: Init many ingress properties to one egress.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcMonIntfInitMonIntfPropertiesNtoOne(
        tc_monintf_intfd_t*        pIntfd,
        CHAR*                      strCmdArg,
        tc_intf_ptype_e     eIntfPTypes)
{
    U16                         _i;
    tc_monintf_mon_t*          _pMonIntf;

    for(_i=0;_i<pIntfd->nMonIntfTotal;_i++)
    {
        _pMonIntf = &(pIntfd->tMonIntfTbl[_i]);
        /* All Monitoring interfaces
         * points to one outgoing interface */
       if('\0' != pIntfd->tMonIntfTbl[_i].tIntf.strIntfName[0])
       {
           switch(eIntfPTypes)
           {
               case tcIntfPTypeRedirAddr:
                   ccur_strlcpy(
                           _pMonIntf->strRedirAddr,strCmdArg,
                           sizeof(_pMonIntf->strRedirAddr));
                   break;
               default:
                   break;
           }

       }
    }
}

/***************************************************************************
 * function: _tcPktProcInitMonIntfFilterTable
 *
 * description: Adding interface filter as an entry into interface name table.
 * an interface name can have multiple interface filters.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcPktProcInitMonIntfFilterTable(
        tc_monintf_intfd_t*          pIntfd,
        tc_utils_keyvalue_t*          pMonRuleseKeyValTbl,
        U16                          nMonRuleseKeyValTblActive)
{
    U16                         _i;
    U16                         _j;
    CHAR*                       _StrLdCfgMonIntf;
    CHAR*                       _StrLdCfgMonIntfltr;
    tc_monintf_mon_t*           _pMonIntf;
    tc_monintf_fltr_t*          _pMonIntffltr;

    for(_i=0;_i<nMonRuleseKeyValTblActive;_i++)
    {
        /* Find the interface name */
        _StrLdCfgMonIntf    = pMonRuleseKeyValTbl[_i].strKey;
        _StrLdCfgMonIntfltr = pMonRuleseKeyValTbl[_i].strValue;
        _pMonIntf = _tcMonIntfFindIntfInMonIntfTbl(
                pIntfd,NULL,_StrLdCfgMonIntf);
        if(_pMonIntf &&
           _StrLdCfgMonIntfltr)
        {
            /* Find empty entry and add the filter */
            for(_j=0;_j<TRANSC_INTERFACE_FLTR_MAX;_j++)
            {
                _pMonIntffltr = &(_pMonIntf->tFilterTbl[_j]);
                if('\0' == _pMonIntffltr->strRuleset[0])
                {
                    ccur_strlcpy(_pMonIntffltr->strRuleset,
                            _StrLdCfgMonIntfltr,
                            sizeof(_pMonIntffltr->strRuleset));
                    _pMonIntf->nFilterTotal++;
                    break;
                }
            }
        }
    }
}

/***************************************************************************
 * function: _tcMonIntfInitMapOneInjIntf
 *
 * description: Map many ingress interface to one egress interface
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcMonIntfInitMapOneInjIntf(
        tc_monintf_intfd_t*      pIntfd)
{
    U16                         _idx;
    tc_monintf_out_t*           _pOutIntf;
    tc_monintf_mon_t*           _pMonIntf;

    _pOutIntf = NULL;
    ccur_memclear(pIntfd->tIntfMapTbl,
            sizeof(pIntfd->tIntfMapTbl));
    pIntfd->nIntfMapTblTotal = 0;
    for(_idx=0;_idx<pIntfd->nOutIntfTotal;_idx++)
    {
       if('\0' != pIntfd->tOutIntfTbl[_idx].tLinkIntf.strIntfName[0])
       {
           _pOutIntf = &(pIntfd->tOutIntfTbl[_idx]);
           _pOutIntf->tLinkIntf.nRefCnt = 0;
           break;
       }
    }
    for(_idx=0;_idx<pIntfd->nMonIntfTotal;_idx++)
    {
        _pMonIntf = &(pIntfd->tMonIntfTbl[_idx]);
        /* All Monitoring interfaces
         * points to one outgoing interface */
       if('\0' != pIntfd->tMonIntfTbl[_idx].tIntf.strIntfName[0])
       {
           pIntfd->tIntfMapTbl[_idx].pMonIntf    = _pMonIntf;
           pIntfd->tIntfMapTbl[_idx].pOutIntfH   = _pOutIntf;
           pIntfd->tIntfMapTbl[_idx].bLinked     = TRUE;
           pIntfd->nIntfMapTblTotal++;
       }
    }
}

/***************************************************************************
 * function: _tcMonIntfInitMapInjIntf
 *
 * description: Map ingress with egress, find the first available entry.
 * It is possible for a map entry to be null on either ingress or egress.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcMonIntfInitMapInjIntf(
        tc_monintf_intfd_t*          pIntfd,
        tc_intf_config_t*            pIntfdCfg,
        tc_utils_keyvalue_t*          pKeyValTbl,
        U16                          nKeyValTblActive)
{
    U16                         _i;
    tc_monintf_mon_t*           _pMonIntf;
    tc_monintf_out_t*           _pOutIntf;

    ccur_memclear(pIntfd->tIntfMapTbl,
            sizeof(pIntfd->tIntfMapTbl));
    pIntfd->nIntfMapTblTotal = 0;
    for(_i=0;_i<nKeyValTblActive;_i++)
    {
        pIntfd->nIntfMapTblTotal++;
        pIntfd->tIntfMapTbl[_i].pMonIntf       = NULL;
        pIntfd->tIntfMapTbl[_i].pOutIntfH      = NULL;
        pIntfd->tIntfMapTbl[_i].bLinked        = FALSE;
        _pMonIntf =
                _tcMonIntfFindIntfInMonIntfTbl(
                        pIntfd,NULL,pKeyValTbl[_i].strKey);
        if(_pMonIntf)
        {
            pIntfd->tIntfMapTbl[_i].pMonIntf       = _pMonIntf;
            _pOutIntf =
                    _tcMonIntfFindIntfInOutIntfTbl(
                            pIntfd,NULL,pKeyValTbl[_i].strValue);
            if(_pOutIntf)
            {
                _pMonIntf->nRefCnt++;
                _pOutIntf->tLinkIntf.nRefCnt=0;
                pIntfd->tIntfMapTbl[_i].pOutIntfH  = _pOutIntf;
                pIntfd->tIntfMapTbl[_i].bLinked    = TRUE;
            }
            else
                pIntfd->tIntfMapTbl[_i].bLinked    = FALSE;
        }
    }
    /* do not allow 1 the same monitoring intf being repeated or
     * mapped to n-th outgoing interface. */
    for(_i=0;_i<pIntfd->nIntfMapTblTotal;_i++)
    {
        _pMonIntf = pIntfd->tIntfMapTbl[_i].pMonIntf;
        if(_pMonIntf && _pMonIntf->nRefCnt > 1)
        {
            pIntfd->tIntfMapTbl[_i].bLinked = FALSE;
            evLogTrace(pIntfdCfg->pEvLog,
                    evLogLvlError,
                    pIntfdCfg->pLogDescSysX,
                   "invalid map interface settings. Unable to map "
                   "the same incoming interface into one outgoing"
                   "interface.");
        }
    }
}

/***************************************************************************
 * function: tcMonIntfInitMapCheck
 *
 * description: validate interface mapping between ingress and egress.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcMonIntfInitMapCheck(
        tc_monintf_intfd_t*        pIntfd,
        tc_intf_config_t*          pIntfdCfg)
{
    tresult_t               _result;
    tresult_t               _sts = ESUCCESS;
    U16                     _i;
    tc_monintf_mon_t*      _pMonIntf;

    do
    {
        _result = EFAILURE;
        if(0 ==
                pIntfd->nIntfMapTblTotal)
            break;
        for(_i=0;_i<pIntfd->nIntfMapTblTotal;_i++)
        {
            if(NULL == pIntfd->tIntfMapTbl[_i].pMonIntf ||
               NULL == pIntfd->tIntfMapTbl[_i].pOutIntfH ||
               FALSE == pIntfd->tIntfMapTbl[_i].bLinked)
            {
                _sts = EFAILURE;
                break;
            }
            /* do not allow 1 the same monitoring intf being repeated or
             * mapped to n-th outgoing interface. */
            _pMonIntf = pIntfd->tIntfMapTbl[_i].pMonIntf;
            if(_pMonIntf && _pMonIntf->nRefCnt > 1)
            {
                _sts = EFAILURE;
                break;
            }
        }
        if(EFAILURE == _sts)
            break;
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}

/***************************************************************************
 * function: tcIntfConfigYamlInitAllInterfaceTbls
 *
 * description: Init ingress, egress, map and redirection interface tables.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcMonIntfConfigYamlInitAllInterfaceTbls(
        tc_monintf_intfd_t*          pIntfd,
        tc_intf_config_t*            pIntfdCfg,
        tc_ldcfg_conf_t*             pLdCfg,
        BOOL                         bRxLoad,
        BOOL                         bTxLoad,
        BOOL                         bMapLoad,
        BOOL                         bRedirLoad)
{
    tresult_t                   _result;
    CHAR*                       _pch;
    tc_utils_keyvalue_t*         _pKeyValTbl = NULL;
    U16                         _nKeyValTblActive = 0;
    tc_utils_keyvalue_t*         _pMonRuleseKeyValTbl = NULL;
    U16                         _nMonRuleseKeyValTblActive = 0;

    CCURASSERT(pIntfd);
    CCURASSERT(pLdCfg);

    do
    {
        _result = EFAILURE;
        if(!strcasecmp("active",
                pLdCfg->strCmdArgModeOfOperation))
            pIntfdCfg->bActiveMode = TRUE;
        else
            pIntfdCfg->bActiveMode = FALSE;
        _pKeyValTbl = (tc_utils_keyvalue_t*)
                malloc(TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
        _pMonRuleseKeyValTbl = (tc_utils_keyvalue_t*)
                malloc(TRANSC_INTERFACE_FLTR_MAX*sizeof(tc_utils_keyvalue_t));
        if(!_pKeyValTbl || !_pMonRuleseKeyValTbl)
        {
            evLogTrace(
                    pIntfdCfg->pEvLog,
                  evLogLvlError,
                  pIntfdCfg->pLogDescSysX,
                  "failure, config.yaml loading"
                  " memory allocation error");
            break;
        }
        if(bRxLoad)
        {
            if('\0' != pLdCfg->strCmdArgPcapFilterRules[0])
            {
                /*--- Init Monitoring interface */
                ccur_memclear(_pKeyValTbl,
                        sizeof(tc_utils_keyvalue_t)*
                        TRANSC_INTERFACE_MAX);
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgMonIntf);
                ccur_memclear(_pMonRuleseKeyValTbl,
                        sizeof(tc_utils_keyvalue_t)*
                        TRANSC_INTERFACE_FLTR_MAX);
                tcUtilsPopulateKeyValuePair(_pMonRuleseKeyValTbl,
                            &(_nMonRuleseKeyValTblActive),
                            TRANSC_INTERFACE_FLTR_MAX,
                            pLdCfg->strCmdArgPcapFilterRules);
                pIntfd->nMonIntfTotal =
                        tcIntfInitIntfTable(
                        pIntfd,
                        NULL,
                        _pKeyValTbl,
                        _nKeyValTblActive);
                _tcPktProcInitMonIntfFilterTable(
                        pIntfd,
                        _pMonRuleseKeyValTbl,
                        _nMonRuleseKeyValTblActive);
                pIntfd->bIntfCfgChanged = TRUE;
            }
        }
        if(bTxLoad)
        {
            if('\0' != pLdCfg->strCmdArgOutIntf[0])
            {
                /*--- Init outgoing interface */
                ccur_memclear(_pKeyValTbl,
                        sizeof(tc_utils_keyvalue_t)*
                        TRANSC_INTERFACE_MAX);
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgOutIntf);
                pIntfd->nOutIntfTotal =
                    tcIntfInitIntfKeyTable(
                            pIntfd,
                            NULL,
                            _pKeyValTbl,
                            _nKeyValTblActive);
                pIntfd->bIntfCfgChanged = TRUE;
            }
        }
        if(bRedirLoad)
        {
            if('\0' != pLdCfg->strCmdArgRedirTarget[0])
            {
                /* init redirection address argument */
                ccur_memclear(_pKeyValTbl,
                        sizeof(tc_utils_keyvalue_t)*
                        TRANSC_INTERFACE_MAX);
                _pch = strchr(pLdCfg->strCmdArgRedirTarget,':');
                if(_pch)
                {
                    tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                                &(_nKeyValTblActive),
                                TRANSC_INTERFACE_MAX,
                                pLdCfg->strCmdArgRedirTarget);
                    _tcMonIntfInitMonIntfProperties(
                            pIntfd,
                            _pKeyValTbl,
                            _nKeyValTblActive,
                            tcIntfPTypeRedirAddr);
                }
                else
                {
                    _tcMonIntfInitMonIntfPropertiesNtoOne(
                            pIntfd,
                            pLdCfg->strCmdArgRedirTarget,
                            tcIntfPTypeRedirAddr);
                }
                pIntfd->bIntfCfgChanged = TRUE;
            }
        }
        if(bMapLoad)
        {
            /* init map interface */
            if('\0' == pLdCfg->strCmdArgMapIntf[0])
            {
                ccur_memclear(_pKeyValTbl,
                        sizeof(tc_utils_keyvalue_t)*
                        TRANSC_INTERFACE_MAX);
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgMapIntf);
                if((1 == pIntfd->nOutIntfTotal) &&
                   (pIntfd->nMonIntfTotal > 0))
                {
                    _tcMonIntfInitMapOneInjIntf(pIntfd);
                    pIntfd->bIntfCfgChanged = TRUE;
                }
                else
                {
                    evLogTrace(
                            pIntfdCfg->pEvLog,
                          evLogLvlError,
                          pIntfdCfg->pLogDescSysX,
                          "failure, Monitoring to Outgoing mapping must be "
                          "specified for n-number of outgoing interfaces");
                    break;
                }
            }
            else
            {
                ccur_memclear(_pKeyValTbl,
                        sizeof(tc_utils_keyvalue_t)*
                        TRANSC_INTERFACE_MAX);
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strCmdArgMapIntf);
                _tcMonIntfInitMapInjIntf(
                        pIntfd,
                        pIntfdCfg,
                        _pKeyValTbl,
                        _nKeyValTblActive);
                pIntfd->bIntfCfgChanged = TRUE;
            }
        }
        _result = ESUCCESS;
    }while(FALSE);

    if(_pKeyValTbl)
        free(_pKeyValTbl);
    if(_pMonRuleseKeyValTbl)
        free(_pMonRuleseKeyValTbl);

    return(_result);
}
