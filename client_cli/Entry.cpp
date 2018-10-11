
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <fstream>
#include "evc/Factory.hpp"
#include "evc/TcpClient.hpp"


int main(int argc, char* argv[]){

#if 0
    try{
        if (argc!=3){
            printf("%s\n", "Usage: <host> <port>");
            printf("%s\n", "example:");
            printf("%s\n", "./EVC 127.0.0.1 8000");
            return -1;
        }


        auto &decoder = Factory::get().createAudioDecoder();
        auto &encoder = Factory::get().createAudioEncoder();
        auto &device = Factory::get().create();



        if (encoder.reInit()){
            if (device.init()){
                if (decoder.reInit()) {
                    TcpClient client(argv[1], argv[2],
                            [&](const char* pData, std::size_t length){
                        // on Received Data
                        std::vector<char> netBuff;
                        netBuff.resize(length);
                        memcpy(netBuff.data(), pData, length);
                        std::vector<short> decodedPcm;
                        decoder.decode(netBuff, decodedPcm);
                        auto ret = device.write(decodedPcm);
                        if (!ret) {
                            std::cout << ret.message() << std::endl;
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
                }
            }
        }

    }
    catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }
#endif
    return 0;
}
