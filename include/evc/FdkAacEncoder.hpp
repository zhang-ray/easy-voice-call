#pragma once

#include "AudioEncoder.hpp"
#include <fdk-aac/aacenc_lib.h>

class FdkAacEncoder : public AudioEncoder, public Singleton<FdkAacEncoder> {
private:
    HANDLE_AACENCODER hAacEncoder = nullptr;
    AACENC_ERROR ErrorStatus;
public:
    FdkAacEncoder(){
        LIB_INFO info;
        aacEncGetLibInfo(&info);
        std::cout << "title = " << info.title << std::endl;
        std::cout << "build_date = " << info.build_date << std::endl;
        std::cout << "versionStr = " << info.versionStr << std::endl;
    }
    virtual ReturnType reInit() override {
        if ((ErrorStatus = aacEncOpen(&hAacEncoder,0,0)) != AACENC_OK ) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_AOT, 23)) != AACENC_OK ) { // low-delay
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_SAMPLERATE, 48000)) != AACENC_OK ) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_CHANNELMODE, MODE_1)) != AACENC_OK ) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_BITRATE, 10)) != AACENC_OK ) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }

        return 0;
    }

    virtual ReturnType encode(const std::vector<char> &pcmData, std::vector<char> &encodedData) {
        aacEncEncode(hAacEncoder, nullptr, nullptr, nullptr, nullptr);
        return 0;
    }

    virtual ~FdkAacEncoder(){
        std::cout << __FILE__ << ":" <<  __LINE__ << std::endl;
    }
};