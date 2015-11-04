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

#include "pktprc/pktprc.h"
#include "pktgen/pktgen.h"

/***************************************************************************
 * function: tcIntfInitIntfKeyTable
 *
 * description: populate interface table. The same interface
 * name (key) will be ignored regardless of direction string (value).
 ***************************************************************************/
CCUR_PROTECTED(U32)
tcIntfInitIntfKeyTable(
        tc_monintf_intfd_t*          pIngressIntf,
        tc_outintf_intfd_t*          pEgressIntf,
        tc_utils_keyvalue_t*          pIntfKeyValTbl,
        U16                          nIntfKeyValTbl)
{
    U16                     _i;
    U16                     _j;
    CHAR*                   _StrLdCfgKey;
    CHAR*                   _StrLdCfgValue;
    tc_intf_linkintf_t*     _pKeyIntf;
    BOOL                    _bAddToList;
    U32                     _nIntfTotal = 0;
    if(pIngressIntf || pEgressIntf)
    {
        if(pIngressIntf)
            ccur_memclear(pIngressIntf->tOutIntfTbl,
                    sizeof(pIngressIntf->tOutIntfTbl));
        else
            ccur_memclear(pEgressIntf->tMonIntfTbl,
                    sizeof(pEgressIntf->tMonIntfTbl));

        for(_i=0;_i<nIntfKeyValTbl;_i++)
        {
            /* Search through the context list to see if the same
             * value exists or not. if it is then don't add
             * then entry */
            _StrLdCfgKey    = pIntfKeyValTbl[_i].strKey;
            _StrLdCfgValue  = pIntfKeyValTbl[_i].strValue;
            _bAddToList = TRUE;
            for(_j=0;_j<TRANSC_INTERFACE_MAX;_j++)
            {
                if(pIngressIntf)
                    _pKeyIntf = &(pIngressIntf->tOutIntfTbl[_j].tLinkIntf);
                else
                    _pKeyIntf = &(pEgressIntf->tMonIntfTbl[_j].tLinkIntf);
                if(!strcmp(_StrLdCfgKey,
                        _pKeyIntf->strIntfName))
                {
                    /* Found! */
                    _bAddToList = FALSE;
                    break;
                }
            }
            /* Adding to context data structure list */
            if(_bAddToList && _StrLdCfgKey && _StrLdCfgValue)
            {
                for(_j=0;_j<TRANSC_INTERFACE_MAX;_j++)
                {
                    if(pIngressIntf)
                        _pKeyIntf = &(pIngressIntf->tOutIntfTbl[_j].tLinkIntf);
                    else
                        _pKeyIntf = &(pEgressIntf->tMonIntfTbl[_j].tLinkIntf);
                    if('\0' == _pKeyIntf->strIntfName[0])
                    {
                        ccur_strlcpy(_pKeyIntf->strIntfName,
                                _StrLdCfgKey,
                                sizeof(_pKeyIntf->strIntfName));
                        _nIntfTotal++;
                        break;
                    }
                }
            }
        }
    }

    return _nIntfTotal;
}

/***************************************************************************
 * function: _tcIntfInitIntfTable
 *
 * description: populate interface table. The same interface
 * name (key) will be ignored regardless of direction string (value).
 ***************************************************************************/
CCUR_PROTECTED(U32)
tcIntfInitIntfTable(
        tc_monintf_intfd_t*          pIngressIntf,
        tc_outintf_intfd_t*          pEgressIntf,
        tc_utils_keyvalue_t*          pIntfKeyValTbl,
        U16                          nIntfKeyValTbl)
{
    U16                     _i;
    U16                     _j;
    CHAR*                   _StrLdCfgKey;
    CHAR*                   _StrLdCfgValue;
    tc_intf_intf_t*         _pIntf;
    BOOL                    _bAddToList;
    U32                     _nIntfTotal = 0;

    if(pIngressIntf || pEgressIntf)
    {
        if(pIngressIntf)
            ccur_memclear(pIngressIntf->tMonIntfTbl,
                    sizeof(pIngressIntf->tMonIntfTbl));
        else
            ccur_memclear(pEgressIntf->tOutIntfTbl,
                    sizeof(pEgressIntf->tOutIntfTbl));

        for(_i=0;_i<nIntfKeyValTbl;_i++)
        {
            /* Search through the context list to see if the same
             * value exists or not. if it is then don't add
             * then entry */
            _StrLdCfgKey    = pIntfKeyValTbl[_i].strKey;
            _StrLdCfgValue  = pIntfKeyValTbl[_i].strValue;
            _bAddToList = TRUE;
            for(_j=0;_j<TRANSC_INTERFACE_MAX;_j++)
            {
                if(pIngressIntf)
                    _pIntf = &(pIngressIntf->tMonIntfTbl[_j].tIntf);
                else
                    _pIntf = &(pEgressIntf->tOutIntfTbl[_j].tIntf);
                if(!strcmp(_StrLdCfgKey,
                        _pIntf->strIntfName))
                {
                    /* Found! */
                    _bAddToList = FALSE;
                    break;
                }
            }
            /* Adding to context data structure list */
            if(_bAddToList && _StrLdCfgKey && _StrLdCfgValue)
            {
                for(_j=0;_j<TRANSC_INTERFACE_MAX;_j++)
                {
                    if(pIngressIntf)
                        _pIntf = &(pIngressIntf->tMonIntfTbl[_j].tIntf);
                    else
                        _pIntf = &(pEgressIntf->tOutIntfTbl[_j].tIntf);
                    if('\0' == _pIntf->strIntfName[0])
                    {
                        ccur_strlcpy(_pIntf->strIntfName,
                                _StrLdCfgKey,
                                sizeof(_pIntf->strIntfName));
                        ccur_strlcpy(_pIntf->strIntfVal,
                                _StrLdCfgValue,
                                sizeof(_pIntf->strIntfVal));
                        if(pEgressIntf)
                        {
                            if(!ccur_strcasecmp(_pIntf->strIntfVal,"router"))
                                pEgressIntf->tOutIntfTbl[_j].bIsRouter = TRUE;
                            else
                                pEgressIntf->tOutIntfTbl[_j].bIsRouter = FALSE;
                        }
                        _pIntf->nIntfIdx    = _j;
                        _pIntf->bIntfIdxSet = TRUE;
                        _nIntfTotal++;
                        break;
                    }
                }
            }
        }
    }

    return _nIntfTotal;
}

/***************************************************************************
 * function: tcIntfCkIntf
 *
 * description: If any changes occur on the config file, then we
 * need to check to see if we need to reload monitoring interface or
 * outgoing interface.
 ***************************************************************************/
CCUR_PROTECTED(void)
tcIntfCkIntf(
        tc_intf_config_t*            pIntfCfg,
        tc_ldcfg_conf_t*             pLdCfg,
        BOOL*                        bRxLoad,
        BOOL*                        bTxLoad,
        BOOL*                        bMapLoad,
        BOOL*                        bRedirLoad)
{
    BOOL                        _bMonIntfChange;
    BOOL                        _bPPRulesChange;

    CCURASSERT(pIntfCfg);
    CCURASSERT(pLdCfg);
    CCURASSERT(bRxLoad);
    CCURASSERT(bTxLoad);
    CCURASSERT(pLdCfg);
    CCURASSERT(bMapLoad);
    CCURASSERT(bRedirLoad);

    _bMonIntfChange = FALSE;
    _bPPRulesChange = FALSE;
    *bRxLoad        = FALSE;
    *bTxLoad        = FALSE;
    *bMapLoad       = FALSE;
    *bRedirLoad     = FALSE;
    /* Check to see if any changes in monitoring,
     * output or pcap ruleset.
     */
    /* check if we need to reload RX */
    if(strncmp(pIntfCfg->strPktPrcArgMonIntf,
            pLdCfg->strCmdArgMonIntf,
            sizeof(pIntfCfg->strPktPrcArgMonIntf)-1))
        _bMonIntfChange = TRUE;
    if(strncmp(pIntfCfg->strPktPrcArgRuleset,
            pLdCfg->strCmdArgPcapFilterRules,
            sizeof(pIntfCfg->strPktPrcArgRuleset)-1))
        _bPPRulesChange = TRUE;
    if(_bMonIntfChange || _bPPRulesChange)
    {
        if('\0' != pLdCfg->strCmdArgMonIntf[0] ||
           '\0' != pLdCfg->strCmdArgPcapFilterRules[0])
            *bRxLoad = TRUE;
        else
            *bRxLoad = FALSE;
    }
    else
        *bRxLoad = FALSE;
    /* check if we need to reload TX */
    if(strncmp(pIntfCfg->strPktPrcArgOutIntf,
            pLdCfg->strCmdArgOutIntf,
            sizeof(pIntfCfg->strPktPrcArgOutIntf)-1))
        *bTxLoad = TRUE;
    else
        *bTxLoad = FALSE;
    if(strncmp(pIntfCfg->strPktPrcArgMapIntf,
            pLdCfg->strCmdArgMapIntf,
            sizeof(pIntfCfg->strPktPrcArgMapIntf)-1))
        *bMapLoad = TRUE;
    else
        *bMapLoad = FALSE;
    if(strncmp(pIntfCfg->strPktPrcArgRedirTarget,
            pLdCfg->strCmdArgRedirTarget,
            sizeof(pIntfCfg->strPktPrcArgRedirTarget)-1))
        *bRedirLoad = TRUE;
    else
        *bRedirLoad = FALSE;
    if(*bTxLoad || *bRxLoad)
    {
        *bMapLoad = TRUE;
        *bRedirLoad = TRUE;
    }
    ccur_strlcpy(
            pIntfCfg->strPktPrcArgMonIntf,
            pLdCfg->strCmdArgMonIntf,
            sizeof(pIntfCfg->strPktPrcArgMonIntf));
    ccur_strlcpy(pIntfCfg->strPktPrcArgOutIntf,
            pLdCfg->strCmdArgOutIntf,
            sizeof(pIntfCfg->strPktPrcArgOutIntf));
    ccur_strlcpy(
            pIntfCfg->strPktPrcArgRuleset,
            pLdCfg->strCmdArgPcapFilterRules,
            sizeof(pIntfCfg->strPktPrcArgRuleset));
    ccur_strlcpy(
            pIntfCfg->strPktPrcArgMapIntf,
            pLdCfg->strCmdArgMapIntf,
            sizeof(pIntfCfg->strPktPrcArgMapIntf));
    ccur_strlcpy(
            pIntfCfg->strPktPrcArgRedirTarget,
            pLdCfg->strCmdArgRedirTarget,
            sizeof(pIntfCfg->strPktPrcArgRedirTarget));
}

