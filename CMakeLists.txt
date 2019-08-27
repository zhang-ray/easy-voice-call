cmake_minimum_required(VERSION 2.8)

project(easy-voice-call)

#debug mode
#set(CMAKE_BUILD_TYPE "Debug")


# treat all warnings as errors
#if(CMAKE_COMPILER_IS_GNUCXX)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
#endif(CMAKE_COMPILER_IS_GNUCXX)


set (CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_definitions(-DBOOST_LOG_DYN_LINK)
include_directories(./include/)
if(WIN32)
    add_compile_options(/MP)  # Speed up compiling
	include_directories(
        ../include/
        "C:/Libraries/boost_1_67_0/"
        )
	add_definitions(-D_WIN32_WINNT=0x0A00)
    link_directories(
        "C:/projects/VS2010/Win32/Release/"
        "C:/projects/build/Release/"
        "C:/Libraries/boost_1_67_0/lib32-msvc-14.0/"
        )
else(WIN32)
    if (ANDROID_NDK)
        include_directories("./opus_android/opuslib/src/main/jni/include" "./Boost-for-Android/boost_1_67_0")
        link_directories("./Boost-for-Android/build/out/arm64-v8a/lib/" "./opus_android/opuslib/src/main/libs/arm64-v8a/")
    else() # Linux
        link_directories(/usr/local/lib/)
    endif()
endif(WIN32)



############   fetch git tag   ############
set (git_cmd "git")
set (git_arg "describe" "--tags")
message(STATUS "git cmd: ${git_cmd}")
execute_process(COMMAND ${git_cmd} ${git_arg}  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}  RESULT_VARIABLE RESULT_VAR  OUTPUT_VARIABLE GIT_TAG)
message(STATUS "git_result: ${git_result}")
message(STATUS "OUTPUT_VARIABLE: ${GIT_TAG}")
string(REGEX REPLACE "\n$" "" GIT_TAG "${GIT_TAG}") # strip line ending
set (GIT_INFO_HEADER_FILE "./include/git_info.hpp")
file(WRITE ${GIT_INFO_HEADER_FILE} "#pragma once\n")
if (RESULT_VAR STREQUAL 0)
    message(STATUS "RESULT_VAR STREQUAL 0")
    #add_compile_definitions(GIT_TAG=${GIT_TAG}) # need higher cmake
    file(APPEND ${GIT_INFO_HEADER_FILE} "#define GIT_TAG \""  ${GIT_TAG} "\"")
endif()


add_subdirectory(audio-processing-module)
if (ANDROID_NDK)
    add_subdirectory(client_essential_android)
else(ANDROID_NDK)
    add_subdirectory(client_essential)
    add_subdirectory(client_cli)
    add_subdirectory(client_qt5)
    add_subdirectory(tests)
    add_subdirectory(server)
endif(ANDROID_NDK)
