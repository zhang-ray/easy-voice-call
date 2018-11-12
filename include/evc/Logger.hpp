#pragma once

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <exception>

inline void dumpException(const std::exception &e) { BOOST_LOG_TRIVIAL(error) << " [" << __FUNCTION__ << "] [" << __FILE__ << ":" << __LINE__ << "] " << e.what(); }
#define LOGV BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << ":\t"
#define LOGI BOOST_LOG_TRIVIAL(info)  << " [" << __FUNCTION__ << "] [" << __FILE__ << ":" << __LINE__ << "] "
#define LOGD BOOST_LOG_TRIVIAL(debug)  << " [" << __FUNCTION__ << "] [" << __FILE__ << ":" << __LINE__ << "] "
#define LOGE BOOST_LOG_TRIVIAL(error) << " [" << __FUNCTION__ << "] [" << __FILE__ << ":" << __LINE__ << "] "