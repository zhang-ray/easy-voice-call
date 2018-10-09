#pragma once
#include <stdio.h>

// TODO:
#if 0 // use Boost.Log
#define log(format , ...)  printf("%s:%d " format "\n", __FILE__ , __LINE__ , ##__VA_ARGS__)
#else


#define BOOST_LOG_DYN_LINK

#include <boost/log/trivial.hpp>

#define LOGV BOOST_LOG_TRIVIAL(trace)
#define LOGD BOOST_LOG_TRIVIAL(debug)
#define LOGI BOOST_LOG_TRIVIAL(info)
#define LOGW BOOST_LOG_TRIVIAL(warning)
#define LOGE BOOST_LOG_TRIVIAL(error)
#define LOGF BOOST_LOG_TRIVIAL(fatal)


#endif
