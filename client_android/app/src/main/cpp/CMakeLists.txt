cmake_minimum_required(VERSION 3.4.1)


############   fetch git tag   ############
set (git_cmd "git")
set (git_arg "describe" "--tags")
message(STATUS "git cmd: ${git_cmd}")
execute_process(COMMAND ${git_cmd} ${git_arg}  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}  RESULT_VARIABLE RESULT_VAR  OUTPUT_VARIABLE GIT_TAG)
message(STATUS "git_result: ${git_result}")
message(STATUS "OUTPUT_VARIABLE: ${GIT_TAG}")
string(REGEX REPLACE "\n$" "" GIT_TAG "${GIT_TAG}") # strip line ending
set (GIT_INFO_HEADER_FILE "./git_info.hpp")
file(WRITE ${GIT_INFO_HEADER_FILE} "#pragma once\n")
if (RESULT_VAR STREQUAL 0)
    message(STATUS "RESULT_VAR STREQUAL 0")
    #add_compile_definitions(GIT_TAG=${GIT_TAG}) # need higher cmake
    file(APPEND ${GIT_INFO_HEADER_FILE} "#define GIT_TAG \""  ${GIT_TAG} "\"")
endif()
############   fetch git tag   ############

include_directories(.)

add_library( client_essential SHARED Entry.cpp )
find_library( log-lib log )

target_link_libraries(client_essential log )