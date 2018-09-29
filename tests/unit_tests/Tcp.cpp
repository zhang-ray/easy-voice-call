#include "evc/TcpClient.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "evc/Log.hpp"

int main(int argc, char **argv){
    
    TcpClient client1;
    {
        auto ret = client1.connect(argv[1], atoi(argv[2]));
        if (!ret){
            std::cout << ret.message() << std::endl;
        }
    }


    TcpClient client2;
    {
        auto ret = client2.connect(argv[1], atoi(argv[2]));
        if (!ret){
            
            std::cout << ret.message() << std::endl;
        }
    }

    std::vector<char> msg1;
    msg1.push_back('m');
    msg1.push_back('s');
    msg1.push_back('g');
    msg1.push_back('1');
    msg1.push_back('\0');


	auto sender = &client1;
	auto receiver = &client2;
	for (int runIndex = 0; runIndex < 2; runIndex++) {
		log("runIndex=%d", runIndex);
		{
			if (auto retSend = sender->send(msg1)) {
				std::vector<char> receivedData;
				if (receiver->recv(receivedData)) {
					for (int i = 0; i < msg1.size(); i++) {
						if (msg1[i] != receivedData[i]) {
							log("msg1[%d](%c) != receivedData[%d](%c)", i, msg1[i], i, receivedData[i]);
							exit(-1);
						}
					}
				}
			}
			else {
				std::cout << retSend.message() << std::endl;
			}
			// std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		sender = &client2;
		receiver = &client1;
	}

    return 0;
}