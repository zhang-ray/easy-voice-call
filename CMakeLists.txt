cmake_minimum_required(VERSION 2.8)
project(EVC)

#debug mode
set(CMAKE_BUILD_TYPE "Debug")


# treat all warnings as errors
#if(CMAKE_COMPILER_IS_GNUCXX)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
#endif(CMAKE_COMPILER_IS_GNUCXX)

add_subdirectory(src)

if(WIN32)
else(WIN32)
add_subdirectory(src_server)
add_subdirectory(tests/unit_tests)
endif (WIN32)
