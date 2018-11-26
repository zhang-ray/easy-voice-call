

add_executable(sanity_check_audio audio/Entry.cpp)
target_link_libraries(sanity_check_audio client_essential)



add_executable(sanity_check_udp_echo_client net/udp/SanityCheckUdpEchoClient.cpp)
target_link_libraries(sanity_check_udp_echo_client client_essential)


