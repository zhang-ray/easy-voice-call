project(server)


file(GLOB_RECURSE FILES_SOURCES *.cpp *.c ../3rd/kcp/*.c)
file(GLOB_RECURSE FILES_INCLUDE *.hpp *.h ../3rd/kcp/*.h)

include_directories(../3rd/kcp/)

add_executable(${PROJECT_NAME} ${FILES_INCLUDE} ${FILES_SOURCES})

if(NOT WIN32)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		# macOS
		link_directories(/usr/local/lib/)
		target_link_libraries(${PROJECT_NAME} boost_system boost_system-mt boost_log_setup boost_log_setup-mt boost_log boost_log-mt pthread )
	else()
		# Linux
		target_link_libraries(${PROJECT_NAME} boost_system boost_log pthread )
	endif()
endif(NOT WIN32)