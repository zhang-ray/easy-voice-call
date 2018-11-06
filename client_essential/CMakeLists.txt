project(client_essential)

set (CMAKE_CXX_STANDARD 14)

include_directories(../include/)

if(WIN32)
	include_directories(
        ../../include/
        "C:/Libraries/boost_1_67_0/"
        )
	add_definitions(-D_WIN32_WINNT=0x0A00)
endif(WIN32)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # macOS
	add_definitions(-DBOOST_LOG_DYN_LINK)
endif()


file(GLOB_RECURSE FILES_SOURCES *.cpp *.c)
file(GLOB_RECURSE FILES_INCLUDE *.hpp *.h ../include/*.hpp ../include/*.h)


if(WIN32)
    link_directories(
        "C:/projects/VS2010/Win32/Release/"
        "C:/projects/build/Release/"
        "C:/Libraries/boost_1_67_0/lib32-msvc-14.0/"
        )
endif(WIN32)



add_library(${PROJECT_NAME} ${FILES_INCLUDE} ${FILES_SOURCES} )

if(WIN32)
    target_link_libraries(${PROJECT_NAME} opus celt silk_common silk_fixed silk_float portaudio_static_x86)
else(WIN32)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries(${PROJECT_NAME} opus pthread
            boost_system
            pthread
            asound
        )
    else(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        # macOS
        target_link_libraries(${PROJECT_NAME} portaudio opus pthread
            boost_system boost_system-mt 
            boost_log_setup boost_log_setup-mt 
            boost_log boost_log-mt 
            boost_thread-mt
            pthread
        )
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

endif(WIN32)
