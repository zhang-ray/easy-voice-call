cmake_minimum_required(VERSION 2.8)

project(easy-voice-call)

#debug mode
#set(CMAKE_BUILD_TYPE "Debug")


# treat all warnings as errors
#if(CMAKE_COMPILER_IS_GNUCXX)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
#endif(CMAKE_COMPILER_IS_GNUCXX)


add_subdirectory(audio-processing-module)

add_subdirectory(client_essential)
#add_subdirectory(client_cli) # shorten building time
add_subdirectory(client_qt5)

add_subdirectory(server)

add_subdirectory(tests/unit_tests)
