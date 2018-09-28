#include "evc/OpusDec.hpp"
#include "evc/OpusEnc.hpp"

#include <numeric>
#include <cmath>
#include<algorithm>

ReturnType compre(const std::vector<char> orig, const std::vector<char> rec){
    if (orig.size()!=rec.size()){
        printf("orig.size=%d != rec.size=%d\n", orig.size(), rec.size());
        return -1;
    }

    auto nbShort = orig.size()/2;
    auto pOrig = (const short *)(orig.data());
    auto pRec = (const short *)(rec.data());


    std::vector<double> diffList;
    for (int i = 0; i < nbShort; i++){
        diffList.push_back(((double)(pOrig[i]- pRec[i]))/(1<<15));
    }

    const auto &resultSet = diffList;

    double sum = std::accumulate(std::begin(resultSet), std::end(resultSet), 0.0);
    double mean =  sum / resultSet.size(); 
    double accum  = 0.0;	
    std::for_each (std::begin(resultSet), std::end(resultSet), [&](const double d) {
        accum  += (d-mean)*(d-mean);	
    }); 	
    double stdev = sqrt(accum/(resultSet.size()-1)); 


    printf("diffList:\n");
    printf("mean=%f var=%f\n", mean, stdev);
    return 0;
}

int main(void){
    AudioEncoder &encoder = OpusEnc::get();
    AudioDecoder &decoder = OpusDec::get();
    if (decoder.reInit()){
        if (encoder.reInit()){
            std::vector<char> origPcm(960*2, 0);
            
            srand(time(NULL));
            for (auto &v : origPcm){
                v = rand();
                // printf("%d ", v);
            }
            printf("\n");

            // for (;;){
                std::vector<char> opusData;
                auto ret = encoder.encode(origPcm, opusData);
                for (auto &v : opusData){
                    // printf("0x%x ", (unsigned char)v);
                }
                if (ret){
                    printf("hehe\n");
                    std::vector<char> decodedData;
                    auto ret2 = decoder.decode(opusData, decodedData);
                    if (ret2){
                        // break;
                        printf("decode OK\n");
                        compre(origPcm, decodedData);
                    }
                }
            // }
        }
    }



    return 0;
}