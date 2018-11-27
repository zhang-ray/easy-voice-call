#include "TcpServer.hpp"
#include "UdpServer.hpp"
#include "KcpServer.hpp"


/// I don't like this kind of global variable
bool isEchoMode = false;


int main(int argc, char* argv[]) {
    try {
        int port = 80;

        // init port
        {
            if ((argc == 2) || (argc == 3)) {
                if (argc == 3) {
                    if (!strcmp("broadcast", argv[2])) {
                        isEchoMode = false;
                    }
                    else if (!strcmp("echo", argv[2])) {
                        isEchoMode = true;
                    }
                    else {
                        printUsage();
                        return -1;
                    }
                }
                port = std::atoi(argv[1]);
            }
            else {
                printUsage();
                return -1;
            }
        }

        LOGI << "PORT=" << port;


        boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);

        boost::asio::io_service io_service;
        
        KcpServer server(io_service, port);

        io_service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}