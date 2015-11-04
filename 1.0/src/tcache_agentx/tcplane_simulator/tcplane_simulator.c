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

#include <pthread.h>

extern "C" void * tcplane_snmp(void * context);

//****************************************************************************
//****************************************************************************
int main(int argc, char ** argv)
{
    pthread_t tid1;

    pthread_create(&tid1, NULL, tcplane_snmp, NULL);
    pthread_join(tid1, NULL);
    return 0;
}
