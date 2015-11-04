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

#ifndef  TCREGEX_H_
#define  TCREGEX_H_

struct _tc_regex_s
{
    pcre*       code;
    pcre_extra* extra;
    I32         options;
    I32         nstartloc;
};
typedef struct _tc_regex_s
               tc_regex_t;

struct _tc_regex_compile_s
{
    CHAR*         strPattern;
    I32           compileOptions;
    I32           stdyOptions;
    tc_regex_t*   pRegex;
    U8*           pTableptr;
};
typedef struct _tc_regex_compile_s
               tc_regex_compile_t;

CCUR_PROTECTED(tresult_t)
tcRegexInit(void);

CCUR_PROTECTED(tresult_t)
tcRegexCompile(
        tc_regex_compile_t* rc);

#define tcRegexCopySubstring  \
        pcre_copy_substring

#define tcRegexExec(re,sdata,sndata, ovector, ovectorsize)  \
    pcre_exec(re->code, re->extra,             \
              sdata, sndata, \
              re->nstartloc, re->options,      \
              ovector, ovectorsize)

#endif
