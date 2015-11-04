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

#include "pktgen.h"
#include "tcmapintf.h"

/***************************************************************************
 * function: _tcOutIntfFindIntfInOutIntfTbl
 *
 * description: Search egress interface entry within egress interface table
 ***************************************************************************/
CCUR_PRIVATE(tc_outintf_out_t*)
_tcOutIntfFindIntfInOutIntfTbl(
        tc_outintf_intfd_t*          pIntfd,
        U16*                         pIdx,
        CHAR*                        strKey)
{
    U16                         _i;
    tc_outintf_out_t*          _pIntf;

    _pIntf             = NULL;
    /* Find the interface name */
    for(_i=0;_i<pIntfd->nOutIntfTotal;_i++)
    {
        if(!strcmp(strKey,pIntfd->tOutIntfTbl[_i].tIntf.strIntfName) &&
           ('\0' != pIntfd->tOutIntfTbl[_i].tIntf.strIntfName[0]) &&
           ('\0' != pIntfd->tOutIntfTbl[_i].tIntf.strIntfVal[0]))
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
 * function: _tcMonIntfFindIntfInOutIntfTbl
 *
 * description: Search ingress interface entry within ingress interface table
 ***************************************************************************/
CCUR_PRIVATE(tc_outintf_mon_t*)
_tcOutIntfFindIntfInMonIntfTbl(
        tc_outintf_intfd_t*          pIntfd,
        U16*                         pIdx,
        CHAR*                        strKey)
{
    U16                         _i;
    tc_outintf_mon_t*         _pIntf;

    _pIntf             = NULL;

    /* Find the interface name */
    for(_i=0;_i<pIntfd->nMonIntfTotal;_i++)
    {
        if(!strcmp(strKey,pIntfd->tMonIntfTbl[_i].tLinkIntf.strIntfName) &&
           ('\0' != pIntfd->tMonIntfTbl[_i].tLinkIntf.strIntfName[0]))
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
 * function: _tcMonIntfInitMonIntfProperties
 *
 * description: Init outgoing interface properties such as Redirection addr.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcOutIntfInitMonIntfProperties(
        tc_outintf_intfd_t*          pIntfd,
        tc_utils_keyvalue_t*         pKeyValTbl,
        U16                          nKeyValTblActive,
        tc_intf_ptype_e              eIntfPTypes,
        BOOL                         bConfigYaml)
{
    U16                         _i;
    CHAR*                       _strKey;
    CHAR*                       _strVal;
    tc_outintf_mon_t*          _pMonIntf;

    for(_i=0;_i<nKeyValTblActive;_i++)
    {
        /* Find the interface name */
        _strKey = pKeyValTbl[_i].strKey;
        _strVal = pKeyValTbl[_i].strValue;
        _pMonIntf = _tcOutIntfFindIntfInMonIntfTbl(
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
_tcOutIntfInitMonIntfPropertiesNtoOne(
        tc_outintf_intfd_t*         pIntfd,
        CHAR*                       strCmdArg,
        tc_intf_ptype_e             eIntfPTypes,
        BOOL                        bConfigYaml)
{
    U16                         _i;
    tc_outintf_mon_t*          _pMonIntf;

    for(_i=0;_i<pIntfd->nMonIntfTotal;_i++)
    {
        _pMonIntf = &(pIntfd->tMonIntfTbl[_i]);
        /* All Monitoring interfaces
         * points to one outgoing interface */
       if('\0' != pIntfd->tMonIntfTbl[_i].tLinkIntf.strIntfName[0])
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
 * function: _tcOutIntfInitOutIntfProperties
 *
 * description: Init outgoing interface properties such as
 * source and dest MAC addresses.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcOutIntfInitOutIntfProperties(
        tc_outintf_intfd_t*             pIntfd,
        tc_intf_config_t*               pIntfdCfg,
        tc_utils_keyvalue_t*            pKeyValTbl,
        U16                             nKeyValTblActive,
        tc_outintf_ptype_e              eIntfPTypes,
        BOOL                            bConfigYaml)
{
    U16                         _i;
    U16                         _j;
    CHAR*                       _strKey;
    CHAR*                       _strVal;
    tc_outintf_out_t*          _pOutIntf;
    I32                         _tmpMacAddr[6];
    BOOL                        _bLoadMacAddr;

    for(_i=0;_i<nKeyValTblActive;_i++)
    {
        /* Find the interface name */
        _strKey = pKeyValTbl[_i].strKey;
        _strVal = pKeyValTbl[_i].strValue;
        _pOutIntf = _tcOutIntfFindIntfInOutIntfTbl(
                pIntfd,NULL,_strKey);
        if(_pOutIntf &&
           _strVal)
        {
            switch(eIntfPTypes)
            {
                case tcOutIntfPTypeSMAC:
                    _bLoadMacAddr = FALSE;
                    if(bConfigYaml)
                    {
                        _bLoadMacAddr = TRUE;
                        _pOutIntf->bSrcMACNotAutomatic = TRUE;
                    }
                    else
                    {
                        if(FALSE == _pOutIntf->bSrcMACNotAutomatic)
                            _bLoadMacAddr = TRUE;
                    }
                    if(_bLoadMacAddr)
                    {
                        sscanf(_strVal,
                                "%x:%x:%x:%x:%x:%x",
                                &_tmpMacAddr[0],
                                &_tmpMacAddr[1],
                                &_tmpMacAddr[2],
                                &_tmpMacAddr[3],
                                &_tmpMacAddr[4],
                                &_tmpMacAddr[5]);
                        for( _j = 0; _j < 6; ++_j )
                            _pOutIntf->pCmdArgSmacAddr[_j] =
                                    (U8)_tmpMacAddr[_j];
                        _pOutIntf->tIntf.bIntfRdy = TRUE;
                        if(!memcmp(_pOutIntf->pCmdArgSmacAddr,
                                "\0\0\0\0\0\0",sizeof(_pOutIntf->pCmdArgSmacAddr)))
                        {
                            _pOutIntf->tIntf.bIntfRdy = FALSE;
                            evLogTrace(
                                    pIntfdCfg->pEvLog,
                                  evLogLvlWarn,
                                  pIntfdCfg->pLogDescSysX,
                                  "Warning, outgoing interface source mac addr invalid input:"
                                  "%x:%x:%x:%x:%x:%x, forcing mode of operation to monitoring!",
                                  _pOutIntf->pCmdArgSmacAddr[0],
                                  _pOutIntf->pCmdArgSmacAddr[1],
                                  _pOutIntf->pCmdArgSmacAddr[2],
                                  _pOutIntf->pCmdArgSmacAddr[3],
                                  _pOutIntf->pCmdArgSmacAddr[4],
                                  _pOutIntf->pCmdArgSmacAddr[5]);
                        }
                    }
                    break;
                case tcOutIntfPTypeDMAC:
                    _bLoadMacAddr = FALSE;
                    if(bConfigYaml)
                    {
                        _bLoadMacAddr = TRUE;
                        _pOutIntf->bDstMACNotAutomatic = TRUE;
                    }
                    else
                    {
                        if(FALSE == _pOutIntf->bDstMACNotAutomatic)
                            _bLoadMacAddr = TRUE;
                    }
                    if(_bLoadMacAddr)
                    {
                        sscanf(_strVal,
                                "%x:%x:%x:%x:%x:%x",
                                &_tmpMacAddr[0],
                                &_tmpMacAddr[1],
                                &_tmpMacAddr[2],
                                &_tmpMacAddr[3],
                                &_tmpMacAddr[4],
                                &_tmpMacAddr[5]);
                        for( _j = 0; _j < 6; ++_j )
                            _pOutIntf->pCmdArgDmacAddr[_j] =
                                    (U8)_tmpMacAddr[_j];
                        _pOutIntf->tIntf.bIntfRdy = TRUE;
                        if(_pOutIntf->bIsRouter)
                        {
                            if(!memcmp(_pOutIntf->pCmdArgDmacAddr,
                                    "\0\0\0\0\0\0",sizeof(_pOutIntf->pCmdArgDmacAddr)))
                            {
                                _pOutIntf->tIntf.bIntfRdy = FALSE;
                                evLogTrace(
                                        pIntfdCfg->pEvLog,
                                      evLogLvlWarn,
                                      pIntfdCfg->pLogDescSysX,
                                      "Warning, outgoing interface dest mac addr invalid input:"
                                      "%x:%x:%x:%x:%x:%x, forcing mode of operation to monitoring!",
                                      _pOutIntf->pCmdArgDmacAddr[0],
                                      _pOutIntf->pCmdArgDmacAddr[1],
                                      _pOutIntf->pCmdArgDmacAddr[2],
                                      _pOutIntf->pCmdArgDmacAddr[3],
                                      _pOutIntf->pCmdArgDmacAddr[4],
                                      _pOutIntf->pCmdArgDmacAddr[5]);
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

/***************************************************************************
 * function: _tcOutIntfInitMapOneInjIntf
 *
 * description: Map many ingress interface to one egress interface
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcOutIntfInitMapOneInjIntf(
        tc_outintf_intfd_t*           pIntfd)
{
    U16                         _idx;
    tc_outintf_out_t*           _pOutIntf;
    tc_outintf_mon_t*           _pMonIntf;

    _pOutIntf = NULL;
    ccur_memclear(pIntfd->tIntfMapTbl,
            sizeof(pIntfd->tIntfMapTbl));
    pIntfd->nIntfMapTblTotal = 0;
    for(_idx=0;_idx<pIntfd->nOutIntfTotal;_idx++)
    {
       if('\0' != pIntfd->tOutIntfTbl[_idx].tIntf.strIntfName[0])
       {
           _pOutIntf = &(pIntfd->tOutIntfTbl[_idx]);
           _pOutIntf->nRefCnt = 0;
           break;
       }
    }
    for(_idx=0;_idx<pIntfd->nMonIntfTotal;_idx++)
    {
        _pMonIntf = &(pIntfd->tMonIntfTbl[_idx]);
        /* All Monitoring interfaces
         * points to one outgoing interface */
       if('\0' != pIntfd->tMonIntfTbl[_idx].tLinkIntf.strIntfName[0])
       {
           pIntfd->tIntfMapTbl[_idx].pMonIntfH   = _pMonIntf;
           pIntfd->tIntfMapTbl[_idx].pOutIntf    = _pOutIntf;
           pIntfd->tIntfMapTbl[_idx].bIsModeActv = FALSE;
           pIntfd->tIntfMapTbl[_idx].bLinked     = TRUE;
           pIntfd->nIntfMapTblTotal++;
       }
    }
}

/***************************************************************************
 * function: _tcOutIntfInitMapInjIntf
 *
 * description: Map ingress with egress, find the first available entry.
 * It is possible for a map entry to be null on either ingress or egress.
 ***************************************************************************/
CCUR_PRIVATE(void)
_tcOutIntfInitMapInjIntf(
        tc_outintf_intfd_t*          pIntfd,
        tc_intf_config_t*            pIntfdCfg,
        tc_utils_keyvalue_t*         pKeyValTbl,
        U16                          nKeyValTblActive)
{
    U16                         _i;
    tc_outintf_mon_t*       _pMonIntf;
    tc_outintf_out_t*        _pOutIntf;

    ccur_memclear(pIntfd->tIntfMapTbl,
            sizeof(pIntfd->tIntfMapTbl));
    pIntfd->nIntfMapTblTotal = 0;
    for(_i=0;_i<nKeyValTblActive;_i++)
    {
        pIntfd->nIntfMapTblTotal++;
        pIntfd->tIntfMapTbl[_i].pMonIntfH      = NULL;
        pIntfd->tIntfMapTbl[_i].pOutIntf       = NULL;
        pIntfd->tIntfMapTbl[_i].bIsModeActv    = FALSE;
        pIntfd->tIntfMapTbl[_i].bLinked        = FALSE;
        _pMonIntf =
                _tcOutIntfFindIntfInMonIntfTbl(
                        pIntfd,NULL,pKeyValTbl[_i].strKey);
        if(_pMonIntf)
        {
            pIntfd->tIntfMapTbl[_i].pMonIntfH       = _pMonIntf;
            _pOutIntf =
                    _tcOutIntfFindIntfInOutIntfTbl(
                            pIntfd,NULL,pKeyValTbl[_i].strValue);
            if(_pOutIntf)
            {
                _pMonIntf->tLinkIntf.nRefCnt++;
                _pOutIntf->nRefCnt=0;
                pIntfd->tIntfMapTbl[_i].pOutIntf   = _pOutIntf;
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
        _pMonIntf = pIntfd->tIntfMapTbl[_i].pMonIntfH;
        if(_pMonIntf && _pMonIntf->tLinkIntf.nRefCnt > 1)
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
 * function: tcOutIntfConfigYamlInitMACAddresses
 *
 * description:  reload data structure from config.yaml configuration
 * file.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcOutIntfConfigYamlInitMACAddresses(
        tc_outintf_intfd_t*          pIntfd,
        tc_intf_config_t*            pIntfdCfg,
        tc_ldcfg_conf_t*             pLdCfg)
{

    tc_utils_keyvalue_t*         _pKeyValTbl = NULL;
    U16                         _nKeyValTblActive = 0;

    CCURASSERT(pIntfd);
    CCURASSERT(pLdCfg);

    _pKeyValTbl = (tc_utils_keyvalue_t*)
            malloc(TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
    if(!_pKeyValTbl)
    {
        evLogTrace(
                pIntfdCfg->pEvLog,
              evLogLvlError,
              pIntfdCfg->pLogDescSysX,
              "failure, config.yaml loading"
              " memory allocation error");
    }
    else
    {
        if('\0' != pLdCfg->strCmdArgOutIntfDestMacAddr[0])
        {
            /*--- Init DMAC */
            ccur_memclear(_pKeyValTbl,
                    sizeof(tc_utils_keyvalue_t)*
                    TRANSC_INTERFACE_MAX);
            tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                        &(_nKeyValTblActive),
                        TRANSC_INTERFACE_MAX,
                        pLdCfg->strCmdArgOutIntfDestMacAddr);
            _tcOutIntfInitOutIntfProperties(
                    pIntfd,
                    pIntfdCfg,
                    _pKeyValTbl,
                    _nKeyValTblActive,
                    tcOutIntfPTypeDMAC,
                    TRUE);
            pIntfd->bIntfCfgChanged = TRUE;
        }
        if('\0' != pLdCfg->strCmdArgOutIntfSrcMacAddr[0])
        {
            /*--- Init SMAC */
            ccur_memclear(_pKeyValTbl,
                    sizeof(tc_utils_keyvalue_t)*
                    TRANSC_INTERFACE_MAX);
            tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                        &(_nKeyValTblActive),
                        TRANSC_INTERFACE_MAX,
                        pLdCfg->strCmdArgOutIntfSrcMacAddr);
            _tcOutIntfInitOutIntfProperties(
                    pIntfd,
                    pIntfdCfg,
                    _pKeyValTbl,
                    _nKeyValTblActive,
                    tcOutIntfPTypeSMAC,
                    TRUE);
            pIntfd->bIntfCfgChanged = TRUE;
        }
    }

    if(_pKeyValTbl)
        free(_pKeyValTbl);
}

/***************************************************************************
 * function: tcOutIntfSysYamlInitMACAddresses
 *
 * description: reload data structure from sys.yaml configuration
 * file.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcOutIntfSysYamlInitMACAddresses(
        tc_outintf_intfd_t*          pIntfd,
        tc_intf_config_t*            pIntfdCfg,
        tc_ldsyscfg_conf_t*          pLdCfg)
{
    tc_utils_keyvalue_t*         _pKeyValTbl = NULL;
    U16                         _nKeyValTblActive = 0;

    if(!ccur_strcasecmp(pLdCfg->strModeOfOperation,"active"))
        pIntfdCfg->bActiveMode = TRUE;
    else
        pIntfdCfg->bActiveMode = FALSE;
    _pKeyValTbl = (tc_utils_keyvalue_t*)
                       calloc(TRANSC_INTERFACE_MAX,
                               sizeof(tc_utils_keyvalue_t));
    if(_pKeyValTbl)
    {
        /*--- Init DMAC */
        if('\0' != pLdCfg->strDestMACAddr[0])
        {
           ccur_memclear(_pKeyValTbl,
                   sizeof(tc_utils_keyvalue_t)*
                   TRANSC_INTERFACE_MAX);
           tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                       &(_nKeyValTblActive),
                       TRANSC_INTERFACE_MAX,
                       pLdCfg->strDestMACAddr);
           _tcOutIntfInitOutIntfProperties(
                   pIntfd,
                   pIntfdCfg,
                   _pKeyValTbl,
                   _nKeyValTblActive,
                   tcOutIntfPTypeDMAC,
                   FALSE);
           pIntfd->bIntfCfgChanged = TRUE;
        }
        /*--- Init SMAC */
        if('\0' != pLdCfg->strSrcMACAddr[0])
        {
           ccur_memclear(_pKeyValTbl,
                   sizeof(tc_utils_keyvalue_t)*
                   TRANSC_INTERFACE_MAX);
           tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                       &(_nKeyValTblActive),
                       TRANSC_INTERFACE_MAX,
                       pLdCfg->strSrcMACAddr);
           _tcOutIntfInitOutIntfProperties(
                   pIntfd,
                   pIntfdCfg,
                   _pKeyValTbl,
                   _nKeyValTblActive,
                   tcOutIntfPTypeSMAC,
                   FALSE);
           pIntfd->bIntfCfgChanged = TRUE;
        }
    }
    else
    {
        evLogTrace(
              pIntfdCfg->pEvLog,
              evLogLvlError,
              pIntfdCfg->pLogDescSysX,
              "failure, config.yaml loading"
              " memory allocation error");
    }
    if(_pKeyValTbl)
       free(_pKeyValTbl);
    _pKeyValTbl       = NULL;
}

/***************************************************************************
 * function: tcOutIntfInitMapCheck
 *
 * description: validate interface mapping between ingress and egress.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcOutIntfInitMapCheck(
        tc_outintf_intfd_t*        pIntfd,
        tc_intf_config_t*          pIntfdCfg)
{
    tresult_t               _result;
    tresult_t               _sts = ESUCCESS;
    U16                     _i;
    tc_outintf_mon_t*       _pMonIntf;

    do
    {
        _result = EFAILURE;
        if(0 ==
                pIntfd->nIntfMapTblTotal)
            break;
        for(_i=0;_i<pIntfd->nIntfMapTblTotal;_i++)
        {
            if(NULL == pIntfd->tIntfMapTbl[_i].pMonIntfH ||
               NULL == pIntfd->tIntfMapTbl[_i].pOutIntf ||
               FALSE == pIntfd->tIntfMapTbl[_i].bLinked)
            {
                _sts = EFAILURE;
                break;
            }
            /* do not allow 1 the same monitoring intf being repeated or
             * mapped to n-th outgoing interface. */
            _pMonIntf = pIntfd->tIntfMapTbl[_i].pMonIntfH;
            if(_pMonIntf && _pMonIntf->tLinkIntf.nRefCnt > 1)
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
 * function: tcOutIntfConfigYamlInitAllInterfaceTbls
 *
 * description: Init ingress, egress, map and redirection interface tables.
 ***************************************************************************/
CCUR_PROTECTED(tresult_t)
tcOutIntfConfigYamlInitAllInterfaceTbls(
        tc_outintf_intfd_t*          pIntfd,
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
                pIntfd->nOutIntfTotal =
                        tcIntfInitIntfKeyTable(
                            NULL,
                            pIntfd,
                            _pKeyValTbl,
                            _nKeyValTblActive);
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
                pIntfd->nMonIntfTotal =
                        tcIntfInitIntfTable(
                        NULL,
                        pIntfd,
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
                    _tcOutIntfInitMonIntfProperties(
                            pIntfd,
                            _pKeyValTbl,
                            _nKeyValTblActive,
                            tcIntfPTypeRedirAddr,
                            TRUE);
                }
                else
                {
                    _tcOutIntfInitMonIntfPropertiesNtoOne(
                            pIntfd,
                            pLdCfg->strCmdArgRedirTarget,
                            tcIntfPTypeRedirAddr,
                            TRUE);
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
                    _tcOutIntfInitMapOneInjIntf(pIntfd);
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
                _tcOutIntfInitMapInjIntf(
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
