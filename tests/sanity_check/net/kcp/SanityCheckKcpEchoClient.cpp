
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

enum { max_length = 1<<9 };




/*
TODO
use NetPacket's TextMessage
use random text 
check broadcast and echo mode
*/

int main(int argc, char* argv[]){
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: blocking_udp_echo_client <host> <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        udp::socket s(io_context, udp::endpoint(udp::v4(), 0));

        udp::resolver resolver(io_context);
        udp::resolver::results_type endpoints =
            resolver.resolve(udp::v4(), argv[1], argv[2]);

        for (int counter = 0; counter < 100000; counter++) {
            char request[max_length];
            for (int i = 0; i < max_length - 1; i++) {
                request[i] = 'a' + counter % 25;
            }
            request[max_length - 1] = '\0';

            size_t request_length = std::strlen(request);
            s.send_to(boost::asio::buffer(request, request_length), *endpoints.begin());

            char reply[max_length];
            udp::endpoint sender_endpoint;
            size_t reply_length = s.receive_from(
                boost::asio::buffer(reply, max_length), sender_endpoint);
            if (reply_length != request_length) {
                throw "reply_length != request_length";
            }
            for (int i = 0; i < reply_length; i++) {
                if (request[i] != reply[i]) {
                    throw "request[i] != reply[i]";
                }
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    getchar();
    return 0;
}