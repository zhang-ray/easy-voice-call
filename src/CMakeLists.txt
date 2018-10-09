
set (CMAKE_CXX_STANDARD 11)

include_directories(../include/)

if(WIN32)
	include_directories(../../include/)
endif(WIN32)

file(GLOB_RECURSE FILES_SOURCES *.cpp *.c)
file(GLOB_RECURSE FILES_INCLUDE *.hpp *.h ../include/*.hpp ../include/*.h)


if(WIN32)
    link_directories("C:/projects/VS2010/Win32/Release/"  "C:/projects/build/Release/")
endif(WIN32)

add_executable(${PROJECT_NAME} ${FILES_INCLUDE} ${FILES_SOURCES} )
#target_link_libraries(${PROJECT_NAME} asound pthread)


if(WIN32)
    target_link_libraries(${PROJECT_NAME} opus celt silk_common silk_fixed silk_float portaudio_static_x86 
        boost_system-vc140-mt-x32-1_67 
        boost_log-vc140-mt-x32-1_67
        pthread
    )
else(WIN32)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries(${PROJECT_NAME} opus pthread 
            boost_system 
            boost_log
            pthread
            asound
        )
    else(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        # macOS
        target_link_libraries(${PROJECT_NAME} portaudio opus pthread 
            boost_system 
            boost_log 
            boost_log-mt 
            pthread
        )
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

endif(WIN32)
