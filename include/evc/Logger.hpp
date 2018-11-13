#pragma once

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <exception>
#include <cstdio>
#include <string>

#define LOGV BOOST_LOG_TRIVIAL(trace)  << "\t[" << __FILE__ << ":" << __LINE__ << "]\t"
#define LOGI BOOST_LOG_TRIVIAL(info)   << "\t[" << __FILE__ << ":" << __LINE__ << "]\t"
#define LOGD BOOST_LOG_TRIVIAL(debug)  << "\t[" << __FILE__ << ":" << __LINE__ << "]\t"
#define LOGE BOOST_LOG_TRIVIAL(error)  << "\t[" << __FILE__ << ":" << __LINE__ << "]\t"

inline void dumpException(const std::exception &e) { LOGE << e.what(); }




class TrickyBoostLog {
/*
HINT: 
  - USE BOOST_LOG_DYN_LINK
    - https://www.boost.org/doc/libs/1_67_0/libs/log/doc/html/log/detailed/sink_frontends.html say that:
        If asynchronous boost::log is used in a multi-module application, one should decide carefully when to unload dynamically loaded modules that write logs.
  - redirecting stdout instead of boost::log::add_file_log
    - when I use boost::log::add_file_log, the formatter is missing... 
    - relevant ticket: https://svn.boost.org/trac10/ticket/8840
*/
public:

    TrickyBoostLog(const boost::log::trivial::severity_level minLevel) {
        boost::log::core::get()->set_filter(boost::log::trivial::severity >= minLevel);
    }

    TrickyBoostLog(const std::string &filePath, const boost::log::trivial::severity_level minLevel= boost::log::trivial::severity_level::debug)
        :TrickyBoostLog(minLevel)
    {
        freopen(filePath.c_str(), "w", stdout);
    }

    ~TrickyBoostLog(){
        fclose(stdout);
    }
};
