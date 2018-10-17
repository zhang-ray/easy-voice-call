set (CMAKE_CXX_STANDARD 11)

include_directories(../../include/)
if(WIN32)
	include_directories(
        ../../../include/
        "C:/Libraries/boost_1_67_0/"
        )
endif(WIN32)


if(WIN32)
    link_directories(
        "C:/projects/VS2010/Win32/Release/"
        "C:/projects/build/Release/"
        "C:/Libraries/boost_1_67_0/lib32-msvc-14.0/"
    )
endif(WIN32)

add_executable(Opus Opus.cpp)

if(WIN32)
    target_link_libraries(Opus opus celt silk_common silk_fixed silk_float)
else(WIN32)
	target_link_libraries(Opus opus)
endif(WIN32)

if(WIN32)
# TODO...
else(WIN32)
    add_executable(Tcp Tcp.cpp)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries(
            Tcp
            boost_system 
            pthread
        )
    else(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries(
            Tcp
            boost_system
            pthread
        )
    endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
endif(WIN32)



add_executable(PortaudioLoopback PortaudioLoopback.cpp)
if(WIN32)
    target_link_libraries(PortaudioLoopback
        portaudio_static_x86
        boost_system-vc140-mt-x32-1_67
    )
else(WIN32)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries(PortaudioLoopback portaudio 
            boost_system
            pthread
        )
    else(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries(PortaudioLoopback portaudio 
            boost_system
            pthread
        )
    endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
endif(WIN32)



if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_executable(AlsaLoopback AlsaLoopback.cpp)
    target_link_libraries(AlsaLoopback asound 
        boost_system
        pthread
    )
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")


