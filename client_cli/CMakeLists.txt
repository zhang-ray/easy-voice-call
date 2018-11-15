project(client_cli)

set (CMAKE_CXX_STANDARD 14)

include_directories(../include/evc/)

add_definitions(-DBOOST_LOG_DYN_LINK)

if(WIN32)
	include_directories(
		../../include/
	    "C:/Libraries/boost_1_67_0/"
	)
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




add_executable(${PROJECT_NAME} ${FILES_INCLUDE} ${FILES_SOURCES} )


target_link_libraries(${PROJECT_NAME} client_essential)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} client_essential)
else(WIN32)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		# macOS
	    target_link_libraries(${PROJECT_NAME} 
			client_essential
			boost_system boost_system-mt 
			boost_log_setup boost_log_setup-mt 
			boost_log boost_log-mt 
			boost_thread-mt
		)
    else()
		# Linux
        target_link_libraries(${PROJECT_NAME}
			client_essential
            boost_system
			boost_thread
			boost_log_setup
			boost_log
        )
    endif()
endif(WIN32)