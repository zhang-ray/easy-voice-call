cmake_minimum_required(VERSION 2.8)

project(easy-voice-call)

#debug mode
#set(CMAKE_BUILD_TYPE "Debug")


# treat all warnings as errors
#if(CMAKE_COMPILER_IS_GNUCXX)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
#endif(CMAKE_COMPILER_IS_GNUCXX)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)



############   fetch git tag   ############
set (git_cmd "git")
set (git_arg "describe" "--tags")
message(STATUS "git cmd: ${git_cmd}")
execute_process(COMMAND ${git_cmd} ${git_arg}  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}  RESULT_VARIABLE RESULT_VAR  OUTPUT_VARIABLE GIT_TAG)
message(STATUS "git_result: ${git_result}")
message(STATUS "OUTPUT_VARIABLE: ${GIT_TAG}")
string(REGEX REPLACE "\n$" "" GIT_TAG "${GIT_TAG}") # strip line ending
set (GIT_INFO_HEADER_FILE "./include/evc/git_info.hpp")
file(WRITE ${GIT_INFO_HEADER_FILE} "#pragma once\n")
if (RESULT_VAR STREQUAL 0)
    message(STATUS "RESULT_VAR STREQUAL 0")
    #add_compile_definitions(GIT_TAG=${GIT_TAG}) # need higher cmake
    file(APPEND ${GIT_INFO_HEADER_FILE} "#define GIT_TAG \""  ${GIT_TAG} "\"")
endif()



############  Speed up compiling ###########
if(WIN32)
    add_compile_options(/MP)
endif()



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