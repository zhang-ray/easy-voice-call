#pragma once

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <exception>

inline void dumpException(const std::exception &e) { BOOST_LOG_TRIVIAL(error) << " [" << __FUNCTION__ << "] [" << __FILE__ << ":" << __LINE__ << "] " << e.what(); }