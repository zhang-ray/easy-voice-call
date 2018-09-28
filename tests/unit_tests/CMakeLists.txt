set (CMAKE_CXX_STANDARD 11)

include_directories(../../include/)

#add_executable(FdkAac FdkAac.cpp)
#target_link_libraries(FdkAac fdk-aac)

if(WIN32)
    link_directories(../VS2010/Win32/Debug)
endif(WIN32)


add_executable(Opus Opus.cpp)
target_link_libraries(Opus opus)

add_executable(Tcp Tcp.cpp)
