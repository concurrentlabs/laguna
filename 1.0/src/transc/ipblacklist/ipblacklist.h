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

#ifndef IPBLACKLIST_H
#define IPBLACKLIST_H 1

#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <tuple>

#include <boost/regex.hpp>

enum { IP_STR_LOWER, IP_UINT_LOWER, IP_STR_UPPER, IP_UINT_UPPER };

typedef std::tuple<std::string, uint32_t, std::string, uint32_t> blacklist_tuple_t;

//*****************************************************************************************
//*****************************************************************************************
class ipblacklist
{
    public:
    ipblacklist() { };

    bool blacklisted(const uint32_t ip);

    uint32_t IPtoUInt(const std::string & ip);

    void populate(const char * s);
    void clear(void);
    void print(void);

    private:
    std::list<blacklist_tuple_t> m_blacklist;
};

#endif //*** IPBLACKLIST_H ***
