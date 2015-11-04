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

CCUR_PRIVATE(tresult_t)
_tcLoadSysValidateArgs(
        evlog_t*                        pEvLogQ,
        evlog_desc_t*                   pLogDescSys,
        tc_ldsyscfg_conf_t*             pLdCfg)
{
    tresult_t                   _sts = ESUCCESS;
    tresult_t                   _result;
    CHAR*                       _pch;
    U16                         _i;
    tc_utils_keyvalue_t*         _pKeyValTbl = NULL;
    U16                         _nKeyValTblActive = 0;
    do
    {
        _pKeyValTbl = (tc_utils_keyvalue_t*)
                malloc(TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
        /* Feel free to add validations here... */
        _result = EFAILURE;
        if('\0' == pLdCfg->strSrcMACAddr[0])
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                "error,src mac address is not specified within sys.yaml,"
                "please check 'outgoinginterface:' within config.yaml");
            break;
        }
        if('\0' == pLdCfg->strModeOfOperation[0])
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                "error, sys.yaml, null mode of operation is not allowed");
            break;
        }
        else
        {
            if(!(!ccur_strcasecmp(pLdCfg->strModeOfOperation,"active")) &&
               !(!ccur_strcasecmp(pLdCfg->strModeOfOperation,"monitor")))
            {
                evLogTrace(
                        pEvLogQ,
                        evLogLvlFatal,
                        pLogDescSys,
                    "error,sys.yaml unknown mode of operation type specified: %s",
                    pLdCfg->strModeOfOperation);
                break;
            }
        }
        if('\0' == pLdCfg->strIntfType[0])
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                "error,sys.yaml  null interface type is not allowed");
            break;
        }
        else
        {
            _pch = strchr(pLdCfg->strIntfType,':');
            if(_pch)
            {
                ccur_memclear(_pKeyValTbl,
                        TRANSC_INTERFACE_MAX*sizeof(tc_utils_keyvalue_t));
                tcUtilsPopulateKeyValuePair(_pKeyValTbl,
                            &(_nKeyValTblActive),
                            TRANSC_INTERFACE_MAX,
                            pLdCfg->strIntfType);
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
                            "error, sys.yaml,unknown interface type specified: %s",
                            _pKeyValTbl[_i].strValue);
                        _sts = EINVAL;
                        break;
                    }
                }
            }
            else
                _sts = EINVAL;
            if(EINVAL == _sts)
                break;
        }
        /*if('\0' == pLdCfg->strDestMACAddr[0])
        {
            evLogTrace(
            pEvLogQ,
            evLogLvlFatal,
            pLogDescSys,
                "error,dest mac address is not specified within sys.yaml,"
                "please check 'outgoinginterface:' within config.yaml");
            break;
        }*/
        _result = ESUCCESS;
    }while(FALSE);

    if(_pKeyValTbl)
        free(_pKeyValTbl);

    return _result;
}

CCUR_PRIVATE(tresult_t)
_tcLoadSysYamlConfInit(FILE*                   f,
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

CCUR_PRIVATE(tresult_t)
_tcLoadSysYamlConfRead(
        evlog_t*                        pEvLogQ,
        evlog_desc_t*                   pLogDescSys,
        yaml_parser_t*                  prs,
        FILE*                           file,
        tc_ldsyscfg_conf_t*             pLdCfg,
        BOOL                            bLock)
{
    tresult_t       _result;
    yaml_event_t    _event;
    U8              _nStkDepth  = 0;
    S32             _nSitesIdx  = 0;
    S32             _nUrlIdx    = -1;
    CHAR*           _pStrBuff   = NULL;
    U32*            _pnStrSig   = NULL;
    U32             _nStrSz     = 0;
    S32             _done       = 0;
    BOOL            _bValSkip   = FALSE;
    /* Debug */
    U8              _i;
    CHAR            _pTab[256];
    CHAR            _pStrScalar[256];
    struct flock    _lock;

    CCURASSERT(pLdCfg);
    CCURASSERT(prs);

    ccur_memclear(_pTab,sizeof(_pTab));
    /* Read the event sequence. */
    _result = ESUCCESS;

    if(bLock)
    {
        ccur_memclear(&_lock, sizeof(_lock));
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
                 *pEvLogQ,
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
                                        (CHAR*)_event.data.scalar.value,"services",sizeof("services")-1))
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
                                            (CHAR*)_event.data.scalar.value,"VERSION",sizeof("VERSION")-1))
                                    {
                                        _pStrBuff   = &(pLdCfg->strVer[0]);
                                        _nStrSz     = sizeof(pLdCfg->strVer);
                                        _bValSkip   = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"MODEOFOPERATION",sizeof("MODEOFOPERATION")-1))
                                    {
                                        _pStrBuff       = &(pLdCfg->strModeOfOperation[0]);
                                        _nStrSz         = sizeof(pLdCfg->strModeOfOperation);
                                        _bValSkip       = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"INTERFACE_TYPE",sizeof("INTERFACE_TYPE")-1))
                                    {
                                        _pStrBuff       = &(pLdCfg->strIntfType[0]);
                                        _nStrSz         = sizeof(pLdCfg->strIntfType);
                                        _bValSkip       = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"INTERFACE_MACADDR",sizeof("INTERFACE_MACADDR")-1))
                                    {
                                        _pStrBuff       = &(pLdCfg->strSrcMACAddr[0]);
                                        _nStrSz         = sizeof(pLdCfg->strSrcMACAddr);
                                        _bValSkip       = FALSE;
                                    }
                                    else if(!strncasecmp(
                                            (CHAR*)_event.data.scalar.value,"GATEWAY_MACADDR",sizeof("GATEWAY_MACADDR")-1))
                                    {
                                        _pStrBuff       = &(pLdCfg->strDestMACAddr[0]);
                                        _nStrSz         = sizeof(pLdCfg->strDestMACAddr);
                                        _bValSkip       = FALSE;
                                    }
                                    else
                                        _bValSkip = TRUE;
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
                 *pEvLogQ,
                        evLogLvlDebug,
                        pLogDescSys,
                 "YAML_SEQUENCE_START_EVENT");*/
                /* Push! */
                _nStkDepth++;
                /* debug print statements */
                _pTab[0] = '\0';
                for(_i=0;_i<_nStkDepth;_i++)
                    ccur_strlcat(_pTab,"    ",sizeof(_pTab));
                break;
            case YAML_SEQUENCE_END_EVENT:
                /*evLogTrace(
                 *pEvLogQ,
                        evLogLvlDebug,
                        pLogDescSys,
                  "YAML_SEQUENCE_END_EVENT");*/
                /* Pop! */
                _nStkDepth--;
                /* debug print statements */
                _pTab[0] = '\0';
                for(_i=0;_i<_nStkDepth;_i++)
                    ccur_strlcat(_pTab,"    ",sizeof(_pTab));
                break;
            case YAML_MAPPING_START_EVENT:
                /*evLogTrace(
                 *pEvLogQ,
                        evLogLvlDebug,
                        pLogDescSys,
                 "YAML_MAPPING_START_EVENT");*/
                break;
            case YAML_MAPPING_END_EVENT:
                /*evLogTrace(
                 *pEvLogQ,
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

CCUR_PROTECTED(tresult_t)
tcLoadUnmarshallSysYaml(
        evlog_t*                        pEvLogQ,
        evlog_desc_t*                   pLogDescSys,
        CHAR*                           strRdConfigLoc,
        tc_ldsyscfg_conf_t*             pLdCfg,
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
        if('\0' == strRdConfigLoc[0])
        {
            _result = ESUCCESS;
            break;
        }
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
        _result = _tcLoadSysYamlConfInit(_f,pYmlParser);
        if(ESUCCESS != _result)
        {
            evLogTrace(
                    pEvLogQ,
                    evLogLvlFatal,
                    pLogDescSys,
                   "Error, sys.yaml read init %s, file doesn't exist",
                   strRdConfigLoc);
            break;
        }
        _bYmlPrsInit = TRUE;
        _result = _tcLoadSysYamlConfRead(
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
                   "Error, sys.yaml read %s",
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
        _result = _tcLoadSysValidateArgs(
                pEvLogQ,pLogDescSys,pLdCfg);

    return _result;
}
