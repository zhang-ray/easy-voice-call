set (CMAKE_CXX_STANDARD 11)

include_directories(../../include/)
if(WIN32)
	include_directories(../../../include/)
endif(WIN32)


if(WIN32)
    link_directories("C:/projects/VS2010/Win32/Release/"  "C:/projects/build/Release/")
endif(WIN32)

add_executable(Opus Opus.cpp)

if(WIN32)
    target_link_libraries(Opus opus celt silk_common silk_fixed silk_float)
else(WIN32)
	target_link_libraries(Opus opus)
endif(WIN32)

add_executable(Tcp Tcp.cpp)
target_link_libraries(
    Tcp
    boost_system
    pthread
)



add_executable(PortaudioLoopback PortaudioLoopback.cpp)
if(WIN32)
    target_link_libraries(PortaudioLoopback portaudio_static_x86)
else(WIN32)
    target_link_libraries(PortaudioLoopback portaudio boost_system pthread)
endif(WIN32)



if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_executable(AlsaLoopback AlsaLoopback.cpp)
    target_link_libraries(AlsaLoopback asound)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")


