project(client_essential)

set (CMAKE_CXX_STANDARD 14)

include_directories(../include/)


# fetch git tag
set (git_cmd "git")
set (git_arg "describe" "--tags")
message(STATUS "git cmd: ${git_cmd}")
execute_process(COMMAND ${git_cmd} ${git_arg}  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}  RESULT_VARIABLE RESULT_VAR  OUTPUT_VARIABLE GIT_TAG)
message(STATUS "git_result: ${git_result}")
message(STATUS "OUTPUT_VARIABLE: ${GIT_TAG}")
string(REGEX REPLACE "\n$" "" GIT_TAG "${GIT_TAG}") # strip line ending
set (GIT_INFO_HEADER_FILE "../include/git_info.hpp")
file(WRITE ${GIT_INFO_HEADER_FILE} "#pragma once\n")
if (RESULT_VAR STREQUAL 0)
    message(STATUS "RESULT_VAR STREQUAL 0")
    #add_compile_definitions(GIT_TAG=${GIT_TAG}) # need higher cmake
    file(APPEND ${GIT_INFO_HEADER_FILE} "#define GIT_TAG \""  ${GIT_TAG} "\"")
endif()




if(WIN32)
	include_directories(
        ../../include/
        "C:/Libraries/boost_1_67_0/"
        )
	add_definitions(-D_WIN32_WINNT=0x0A00)
    add_definitions(-DEVC_API=EVC_EXPORT)
    link_directories(
        "C:/projects/VS2010/Win32/Release/"
        "C:/projects/build/Release/"
        "C:/Libraries/boost_1_67_0/lib32-msvc-14.0/"
        )
else(WIN32)
	add_definitions(-DBOOST_LOG_DYN_LINK)
	link_directories(/usr/local/lib/)
endif(WIN32)


file(GLOB_RECURSE FILES_SOURCES *.cpp *.c)
file(GLOB_RECURSE FILES_INCLUDE *.hpp *.h ../include/*.hpp ../include/*.h)




add_library(${PROJECT_NAME} SHARED ${FILES_INCLUDE} ${FILES_SOURCES} )


if(WIN32)
    target_link_libraries(${PROJECT_NAME}
		vad aec ns
		opus celt silk_common silk_fixed silk_float 
		portaudio_static_x86
	)
else(WIN32)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		# macOS
	    target_link_libraries(${PROJECT_NAME} 
			vad aec ns
			boost_system boost_system-mt 
			boost_log_setup boost_log_setup-mt 
			boost_log boost_log-mt 
			boost_thread-mt
			portaudio opus
			pthread 
		)
    else()
		# Linux
        target_link_libraries(${PROJECT_NAME}
			vad aec ns
			portaudio opus
			opus pthread
            boost_system
			boost_thread
			boost_log_setup
			boost_log
            pthread
            asound
        )
    endif()
endif(WIN32)
