cmake_minimum_required(VERSION 2.8)

project(easy-voice-call)

#debug mode
#set(CMAKE_BUILD_TYPE "Debug")


# treat all warnings as errors
#if(CMAKE_COMPILER_IS_GNUCXX)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
#endif(CMAKE_COMPILER_IS_GNUCXX)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Ubuntu 14.04 is to old...
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_subdirectory(audio-processing-module)
    add_subdirectory(client_essential)
    add_subdirectory(client_cli)
    add_subdirectory(client_qt5)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        add_subdirectory(tests/unit_tests)
    endif()
endif()

add_subdirectory(server)