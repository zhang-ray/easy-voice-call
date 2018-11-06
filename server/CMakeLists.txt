project(server)

set (CMAKE_CXX_STANDARD 14)

include_directories(../include/)

if (WIN32)
	include_directories(C:/Libraries/boost_1_67_0)
	link_directories(C:/Libraries/boost_1_67_0/lib32-msvc-14.0/)
	add_definitions(-D_WIN32_WINNT=0x0A00)
endif(WIN32)

file(GLOB_RECURSE FILES_SOURCES *.cpp *.c)


add_executable(${PROJECT_NAME} ${FILES_INCLUDE} ${FILES_SOURCES} )

if(NOT WIN32)
	add_definitions(-DBOOST_LOG_DYN_LINK)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		# macOS
		link_directories(/usr/local/lib/)
		target_link_libraries(${PROJECT_NAME} boost_system boost_system-mt boost_log_setup boost_log_setup-mt boost_log boost_log-mt pthread )
	else()
		# Linux
		target_link_libraries(${PROJECT_NAME} boost_system boost_log pthread )
	endif()
endif(NOT WIN32)