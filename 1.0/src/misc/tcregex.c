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

#include "helper.h"
#include "evlog.h"
#include "tcregex.h"

tresult_t tcRegexInit(void)
{return ESUCCESS;}

CCUR_PROTECTED(tresult_t)
tcRegexCompile(
        tc_regex_compile_t* rc)
{
    tresult_t               _result;
    const CHAR*             _strPerr;
    I32                     _nStrPerr;

    do
    {
        if('\0' == rc->strPattern[0])
        {
            _result = ESUCCESS;
            /* Set default all to zero */
            rc->pRegex->options     = 0;
            rc->pRegex->nstartloc   = 0;
            rc->pRegex->code        = NULL;
            rc->pRegex->extra       = NULL;
            break;
        }
        _result  = EFAILURE;
        /* Set default all to zero */
        rc->pRegex->options     = 0;
        rc->pRegex->nstartloc   = 0;
        /* Compile regex */
        rc->pRegex->code =
                pcre_compile(
                        rc->strPattern,rc->compileOptions,
                        &_strPerr,&_nStrPerr,rc->pTableptr);

        if(NULL ==
                rc->pRegex->code)
        {
#if 0
            evLogTraceSys(
                    evLogLvlError,
                    "ERROR: Could not compile '%s': %s\n",
                    rc->strPattern,
                    _strPerr);
#endif
            break;
        }
        _strPerr = NULL;
        /* Optimize the regex.
         * Returns NULL for both errors and
         * when it can not optimize regex. */
        rc->pRegex->extra =
                pcre_study(rc->pRegex->code,
                        rc->stdyOptions,
                        &_strPerr);
        if(_strPerr)
        {
#if 0
            evLogTraceSys(
                    evLogLvlError,
                    "ERROR: Could not study '%s': %s\n",
                    rc->strPattern,
                    _strPerr);
#endif
            break;
        }
        _result = ESUCCESS;
    }while(FALSE);

    return _result;
}
