
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <fstream>
#include "evc/Factory.hpp"
#include "evc/TcpClient.hpp"


int main(int argc, char **argv){
    if (argc!=3){
        printf("%s\n", "usage: EVC IP port");
        return -1;
    }


    AudioEncoder &encoder = Factory::get().createAudioEncoder();
    AudioDevice &device = Factory::get().create();
    TcpClient client;
    client.connect(argv[1], atoi(argv[2]));
    if (encoder.reInit()){
        if (device.init()){
            std::thread recvThread([&](){
            //     std::vector<char> buff;
            //     client.recv(buff);
            //     decoder.
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
            }


            //recvThread.join();
        }
    }
    return 0;
}
