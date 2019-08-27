project(client_essential)



if(WIN32)
    add_definitions(-DEVC_EXPORT)
endif(WIN32)


file(GLOB_RECURSE FILES_SOURCES *.cpp *.c)
file(GLOB_RECURSE FILES_INCLUDE *.hpp *.h ../include/*.hpp ../include/*.h)


include_directories(.)

add_library(${PROJECT_NAME} SHARED ${FILES_INCLUDE} ${FILES_SOURCES} )


if(WIN32)
    target_link_libraries(${PROJECT_NAME}
		vad aec ns indie_cng
		opus celt silk_common silk_fixed silk_float 
		portaudio_static_x86
	)
else(WIN32)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		# macOS
	    target_link_libraries(${PROJECT_NAME} 
			vad aec ns indie_cng
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
                        vad aec ns indie_cng
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
