
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <fstream>
#include "evc/Factory.hpp"

int main_(void){

    AudioDevice &device = Factory::get().create();
    if (device.init()){
        std::vector<short> micBuffer(1<<9);

        std::thread micThread([&](){
            std::ifstream ifs("in.pcm");
            std::vector<char> inFile;
            if (ifs.is_open()){
                auto begin = ifs.tellg();
                ifs.seekg (0, std::ios::end);
                auto end = ifs.tellg();
                auto totalFileSize = end-begin;
                ifs.seekg (0, std::ios::beg);

                inFile.resize(totalFileSize);
                ifs.read(inFile.data(), totalFileSize);
            }


            for (auto pos = 0u; ; ){
                auto ret = device.write((char *)(&(inFile[pos])), 1<<9);
                if(ret>0){
                    pos+=ret*2;
                }
                if (pos>inFile.size()){
                    //break;
                    pos = 0;
                }
            }
        });



        std::ofstream ofs("hehe.pcm");
        for (;;){
            auto ret = device.read((char *)micBuffer.data(), 1<<10);
            if (ret>0){
                ofs.write((const char*)micBuffer.data(), ret);
            }
            if (micBuffer[0]>10000){
            }
        }

        micThread.join();
    }
    return 0;
}



int loopback(){
    AudioDevice &device = Factory::get().create();
    if (device.init()){
        for (;;){
            std::vector<char> micBuffer(1<<12);
            const auto blockSize = 1<<7;
            // TODO: use RingBuffer?
            auto ret = device.read((char *)micBuffer.data(), blockSize);
            if (ret>0){
                device.write((char *)(micBuffer.data()), blockSize);
            }

            if (ret!=blockSize){
                throw;
            }
        }
    }
    return 0;
}


int main(void){
    AudioEncoder &encoder = Factory::get().createAudioEncoder();
#if 0
    AudioDevice &device = Factory::get().create();
    if (encoder.reInit()){
        if (device.init()){
            for (;;){
                std::vector<char> micBuffer(1<<12);
                const auto blockSize = 1<<7;
                // TODO: use RingBuffer?
                auto ret = device.read((char *)micBuffer.data(), blockSize);
                std::vector<char> outData;
                ret = encoder.encode(micBuffer, outData);

            }
        }
    }
#endif
    return 0;
}
