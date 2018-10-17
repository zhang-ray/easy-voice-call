project(client_essential)

set (CMAKE_CXX_STANDARD 11)

include_directories(../include/)

if(WIN32)
	include_directories(
        ../../include/
        "C:/Libraries/boost_1_67_0/boost include/"
        )
endif(WIN32)

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
    target_link_libraries(${PROJECT_NAME} opus celt silk_common silk_fixed silk_float portaudio_static_x86
        boost_system-vc140-mt-x32-1_67
    )
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
            boost_system
            pthread
        )
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

endif(WIN32)
