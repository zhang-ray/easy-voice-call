project(client_essential)

set (CMAKE_CXX_STANDARD 14)

include_directories(../include/evc/)




add_definitions(-DBOOST_LOG_DYN_LINK)
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
