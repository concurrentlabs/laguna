
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

#ifndef CCUR_REMAP_H
#define CCUR_REMAP_H 1

#include <fstream>
#include <iostream>
#include <sstream>
#include <list>
#include <string>
#include <tuple>

#include <boost/regex.hpp>

#include <inttypes.h>

#include <ts/ts.h>
#include <ts/experimental.h>
#include <remap.h>

//*** from_pattern, to_pattern, from_re ****
typedef std::tuple<std::string, std::string, boost::regex> remap_tuple_t;

//*******************************************************************************
//*******************************************************************************
struct remap_elem_t
{
    //***************************************************************************
    //***************************************************************************
    remap_elem_t(remap_tuple_t & t) : m_tuple(t) { }

    //***************************************************************************
    //***************************************************************************
    void print(void) const
    {
        TSDebug("ccur_remap", "remap pattern: %s -> %s",
            std::get<0>(m_tuple).c_str(), std::get<1>(m_tuple).c_str());
        return;
    }

    //***************************************************************************
    //***************************************************************************
    const std::string  from_pattern(void) { return std::get<0>(m_tuple); }
    const std::string  to_pattern(void)   { return std::get<1>(m_tuple); }
    const boost::regex from_re(void)      { return std::get<2>(m_tuple); }

    //***************************************************************************
    //***************************************************************************
    private:
    remap_tuple_t m_tuple;
};

//*******************************************************************************
//*******************************************************************************
class remap_table_t
{
    public:
    remap_table_t() { }

    std::string construct_remap_url(const std::string & debug_tag,
	const std::string & from_url);

    void load_config_file(std::string & path);

    void clear() { m_list.clear(); }

    size_t size() { return m_list.size(); }

    //***************************************************************************
    //***************************************************************************
    void print(void)
    {
        for(std::list<remap_elem_t>::const_iterator it = m_list.cbegin();
            it != m_list.cend(); ++it)
        {
            (*it).print();
        }
        return;
    }

    private:
    remap_tuple_t split(const std::string & s);

    std::list<remap_elem_t> m_list;
};

//*******************************************************************************
//*******************************************************************************
std::string remap_table_t::construct_remap_url(const std::string & debug_tag,
	const std::string & from_url)
{
    boost::smatch matches;

    std::string to_url = from_url;

    for(std::list<remap_elem_t>::iterator it = m_list.begin();
        it != m_list.end(); ++it)
    {
        if(boost::regex_match(from_url, matches, (*it).from_re()))
        {
            to_url = boost::regex_replace(from_url, (*it).from_re(), (*it).to_pattern());

            TSDebug(debug_tag.c_str(), "====================================================");
            TSDebug(debug_tag.c_str(), "Old URL: [%s].", from_url.c_str());
            TSDebug(debug_tag.c_str(), "New URL: [%s].", to_url.c_str());
            TSDebug(debug_tag.c_str(), "Matched Rule: [%s].", (*it).from_pattern().c_str());
            TSDebug(debug_tag.c_str(), "====================================================");
            break;
        }
    }
    return std::move(to_url);
}

//************************************************************************************
//************************************************************************************
void remap_table_t::load_config_file(std::string & fname)
{
    std::ifstream stream(fname);
    if(!stream.good())
    {
        std::cout << "\nERROR: open " << fname << " failed.\n\n";
        return;
    }

    while(stream.good() && !stream.eof())
    {
        std::string s;
        std::getline(stream, s);
        if(s.c_str()[0] == '#')
            continue;
        remap_tuple_t t = split(s);
        if(!std::get<0>(t).length())
            continue;
        remap_elem_t remap_elem(t);
        m_list.push_back(remap_elem);
    }
    stream.close();
    return;
}

//************************************************************************************
// function: remap_table_t::split
//
// description: split string into remap_tuple_t based in SPACES or TABS.
//************************************************************************************
remap_tuple_t remap_table_t::split(const std::string & s)
{
    std::string::size_type i{};
    remap_tuple_t t;

    if((i = s.find(' ')) == std::string::npos)
        i = s.find('\t');
    if(i == std::string::npos)
        return std::move(t);
    std::get<0>(t) = s.substr(0, i);
    std::get<2>(t) = boost::regex(s.substr(0, i));
    while(s[i] == ' ' || s[i] == '\t')
        ++i;
    std::get<1>(t) = s.substr(i, std::string::npos);
    return std::move(t);
}

#endif //*** CCUR_REMAP_H ***
