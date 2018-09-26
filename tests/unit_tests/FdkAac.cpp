
#include "evc/FdkAacEncoder.hpp"
#include "evc/FdkAacDecoder.hpp"


int main(void){
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    AudioEncoder &encoder = FdkAacEncoder::get();

            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    AudioDecoder &decoder = FdkAacDecoder::get();
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    if (decoder.reInit()){
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        if (encoder.reInit()){
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
            std::vector<char> zeroValueMonoPcm(1<<12, 0);
            for (auto &v : zeroValueMonoPcm){
                // v = rand();
                // printf("%d ", v);
            }

            // for (;;){
                std::vector<char> outData;
                auto ret = encoder.encode(zeroValueMonoPcm, outData);
                for (auto &v : outData){
                    printf("0x%x ", (unsigned char)v);
                }
                if (ret){
                    std::vector<char> recData;
                    auto ret2 = decoder.decode(outData, recData);
                    if (!ret){
                        // break;
                    }
                }
            // }
        }
    }


    return 0;
}