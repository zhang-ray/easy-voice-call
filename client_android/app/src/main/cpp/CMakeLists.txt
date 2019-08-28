cmake_minimum_required(VERSION 3.4.1)

add_library( client_essential SHARED Entry.cpp )
find_library( log-lib log )

target_link_libraries(client_essential log )