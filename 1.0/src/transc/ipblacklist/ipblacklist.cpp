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

#include <ipblacklist.h>

using namespace std;

static ipblacklist g_ipblacklist;

//************************************************************************************
// function: populate_blacklist
//
// description: populate blacklist class with ips.
//************************************************************************************
extern "C" void populate_blacklist(const char * s)
{
    g_ipblacklist.populate(s);
    return;
}

//************************************************************************************
// function: print_blacklist
//
// description: print blacklist class ips.
//************************************************************************************
extern "C" void print_blacklist(void)
{
    g_ipblacklist.print();
    return;
}

//************************************************************************************
// function: blacklisted
//
// description: determine if ip is blacklisted.
//************************************************************************************
extern "C" int blacklisted(const uint32_t ip)
{
    return g_ipblacklist.blacklisted(ip);
}

//************************************************************************************
// function: clear_blacklist
//
// description: clear ipblacklist.
//************************************************************************************
extern "C" void clear_blacklist(void)
{
    g_ipblacklist.clear();
    return;
}

//************************************************************************************
// function: str_ip
//
// description: convert integer ip to char *.
//************************************************************************************
extern "C" const char * str_ip(const uint32_t ip)
{
    ostringstream oss;
    uint32_t i{};
    
    i = (ip & 0xFF000000); i = i >> 24;
    oss << i << '.';
    i = (ip & 0x00FF0000) >> 16;
    oss << i << '.';
    i = (ip & 0x0000FF00) >> 8;
    oss << i << '.';
    i = (ip & 0x000000FF);
    oss << i;
    return oss.str().c_str();
}

//************************************************************************************
// function: ipblacklist::clear
//
// description: clear ipblacklist.
//************************************************************************************
void ipblacklist::clear(void)
{
    m_blacklist.clear();
    return;
}

//************************************************************************************
// function: ipblacklist::populate
//
// description: split string into blacklist tokens based on coma delimiter.
//************************************************************************************
void ipblacklist::populate(const char * buffer)
{
    string s(buffer);
    if(s.length())
    {
        boost::regex re("[,]");
        boost::sregex_token_iterator p(s.cbegin(), s.cend(), re, -1);
        boost::sregex_token_iterator e;
        for( ; p != e; ++p)
        {
            blacklist_tuple_t t;
            string s2 = p->str();
            if(s2.find("-") != string::npos)
            {
                boost::regex re2("[\\-]");
                boost::sregex_token_iterator p2(s2.cbegin(), s2.cend(), re2, -1);
                boost::sregex_token_iterator e2;

                get<IP_STR_LOWER>(t) = p2->str();
                get<IP_UINT_LOWER>(t) = IPtoUInt(p2->str());

                ++p2;
                get<IP_STR_UPPER>(t) = p2->str();
                get<IP_UINT_UPPER>(t) = IPtoUInt(p2->str());
            }
            else
            {
                get<IP_STR_LOWER>(t)  = get<IP_STR_UPPER>(t) = p->str();
                get<IP_UINT_LOWER>(t) = get<IP_UINT_UPPER>(t) = IPtoUInt(p->str());
            }
            m_blacklist.push_back(t);
        }
    }
    return;
}

//************************************************************************************
// function: ipblacklist::print
//
// description: print ipblacklist contents.
//************************************************************************************
void ipblacklist::print(void)
{
    for(auto elem : m_blacklist)
    {
        cout << "blacklist token: " <<
            get<IP_STR_LOWER>(elem) << ' ' << get<IP_UINT_LOWER>(elem) << ' ' <<
            get<IP_STR_UPPER>(elem) << ' ' << get<IP_UINT_UPPER>(elem) << ".\n";
    }
    return;
}

//************************************************************************************
// function: ipblacklist::ItoUInt
//
// description: convert ip to unsigned int.
//************************************************************************************
uint32_t ipblacklist::IPtoUInt(const string & ip)
{
    int a, b, c, d;
    uint32_t addr{};

    if(sscanf(ip.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
        return 0;

    addr  = a << 24;
    addr |= b << 16;
    addr |= c << 8;
    addr |= d;
    return addr;
}

//************************************************************************************
// function: ipblacklist::blacklisted
//
// description: determine if ip is blacklisted.
//************************************************************************************
bool ipblacklist::blacklisted(const uint32_t ip)
{
    for(auto elem : m_blacklist)
    {
        if(ip >= get<IP_UINT_LOWER>(elem) && ip <= get<IP_UINT_UPPER>(elem))
            return true;
    }
    return false;
}
