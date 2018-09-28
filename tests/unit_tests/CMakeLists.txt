set (CMAKE_CXX_STANDARD 11)

include_directories(../../include/)

#add_executable(FdkAac FdkAac.cpp)
#target_link_libraries(FdkAac fdk-aac)

add_executable(Opus Opus.cpp)
target_link_libraries(Opus opus)

add_executable(Tcp Tcp.cpp)
target_link_libraries(Tcp uv)
