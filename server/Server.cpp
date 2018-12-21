#include "TcpServer.hpp"
#include "KcpServer.hpp"
#include "RawUdpServer.hpp"



/// I don't like this kind of global variable
bool isEchoMode = false;



void printUsage() {
    std::cerr << "Usage: EvcServer <port>\n";
    std::cerr << "   or: EvcServer <port> <broadcast/echo>\n";
}






int main____(int argc, char* argv[]) {
    std::shared_ptr<Server> server_ = nullptr;
    try {
        int port = 80;

        if (argc != 4) {
            printUsage();
            return -1;
        }
        


        if (!strcmp("broadcast", argv[3])) {
            isEchoMode = false;
        }
        else if (!strcmp("echo", argv[3])) {
            isEchoMode = true;
        }
        else {
            printUsage();
            return -1;
        }
        
        
        port = std::atoi(argv[2]);


        LOGI << "PORT=" << port;


        boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);

        boost::asio::io_service io_service;

        if (!strcmp("raw_udp", argv[1])) {
            server_ = std::make_shared<RawUdpServer>(io_service, port);
        }
        else if (!strcmp("raw_tcp", argv[1])) {
            server_ = std::make_shared<TcpServer>(io_service, port);
        }
        else if (!strcmp("kcp_udp", argv[1])) {
            server_ = std::make_shared<KcpServer>(io_service, port);
        }
        else {
            LOGE << "unknown protocol : " << argv[1];
        }

        io_service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}


int main(int argc, char* argv[]) {
	std::shared_ptr<Server> server_ = nullptr;
	try {
		int port = 80;

		if ((argc != 2)&&(argc != 3)) {
			printUsage();
			return -1;
		}


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


		LOGI << "PORT=" << port;
		if (isEchoMode ) LOGI<< "ECHO MODE";

		boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);

		boost::asio::io_service io_service;

		server_ = std::make_shared<RawUdpServer>(io_service, port);
		

		io_service.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}