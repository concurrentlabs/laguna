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

#include <stdio.h>
#include <string.h>

#include "dummy_code.h"

//**********************************************************************
//**********************************************************************
char * tcBkgrndGetRedirAddr(tc_gd_thread_ctxt_t * pCntx, char * ip,
    size_t size)
{
    strcpy(ip, "127.0.0.1");
    return ip;
}

//**********************************************************************
//**********************************************************************
int tcBkgrndReLoadCfg(tc_gd_thread_ctxt_t * pCntx)
{
    return 1;
}

//**********************************************************************
//**********************************************************************
int tcBkgrndGetTrSts(tc_gd_thread_ctxt_t * pCntx)
{
    return 1;
}

//**********************************************************************
//**********************************************************************
void evLogTraceSys(int level, const char * fmt, const char * msg)
{
    char buffer[96];
    sprintf(buffer, fmt, msg);
    printf(buffer); printf("\n");
    return;
}

//**********************************************************************
//**********************************************************************
void tcBkgrndSetRedirNode(tc_gd_thread_ctxt_t * _pCntx, int state)
{
    return;
}
