set (CMAKE_CXX_STANDARD 11)

include_directories(../../include/)

add_executable(FdkAac FdkAac.cpp)
target_link_libraries(FdkAac fdk-aac)