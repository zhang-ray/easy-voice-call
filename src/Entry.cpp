
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <fstream>
#include "evc/Factory.hpp"
#include "evc/TcpClient.hpp"


int _main(int argc, char **argv){
    if (argc!=3){
        printf("%s\n", "usage: EVC IP port");
        printf("%s\n", "example:");
        printf("%s\n", "./EVC 127.0.0.1 8000");
        return -1;
    }


    auto &decoder = Factory::get().createAudioDecoder();
    auto &encoder = Factory::get().createAudioEncoder();
    auto &device = Factory::get().create();

#if 0
    TcpClient client;
    client.connect(argv[1], atoi(argv[2]));

    if (encoder.reInit()){
        if (device.init()){
            std::thread recvThread([&](){
                std::vector<char> netBuff;
                client.recv(netBuff);
                std::vector<short> decodedPcm;
				if (decoder.reInit()) {
					decoder.decode(netBuff, decodedPcm);
					auto ret = device.write(decodedPcm);
					if (!ret) {
						std::cout << ret.message() << std::endl;
					}
				}
            });



            // sending work
            for (;;){
                const auto blockSize = 1920;
                std::vector<short> micBuffer(blockSize);
                // TODO: use RingBuffer?
                auto ret = device.read(micBuffer);
                if (!ret){
                    break;
                }
                std::vector<char> outData;
                auto retEncode = encoder.encode(micBuffer, outData);
                if (!retEncode){
                    std::cout << retEncode.message() << std::endl;
                    break;
                }
                client.send(outData);
            }


            //recvThread.join();
        }
    }

#endif
    return 0;
}



int main(int argc, char* argv[]){
    try{
        if (argc != 3)
        {
            std::cerr << "Usage: chat_client <host> <port>\n";
            return 1;
        }

        TcpClient client(argv[1], argv[2], [](const char* pData, std::size_t length){
            std::cout.write(pData, length);
            std::cout << "\n";
        } );

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
