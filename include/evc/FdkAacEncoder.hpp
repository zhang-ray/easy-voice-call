#pragma once

#include "AudioEncoder.hpp"
#include "evc/Singleton.hpp"
#include <iostream>

extern "C"{
#include <fdk-aac/aacenc_lib.h>
}


class FdkAacEncoder final : public AudioEncoder, public Singleton<FdkAacEncoder> {
private:
    HANDLE_AACENCODER hAacEncoder = nullptr;
    AACENC_ERROR ErrorStatus;
    
    LIB_INFO lib_info_;
    AACENC_InfoStruct info_= { 0 };
public:
    FdkAacEncoder(){
        aacEncGetLibInfo(&lib_info_);
        std::cout << "title = " << lib_info_.title << std::endl;
        std::cout << "build_date = " << lib_info_.build_date << std::endl;
        std::cout << "versionStr = " << lib_info_.versionStr << std::endl;
    }

    virtual ReturnType reInit() override {
        if ((ErrorStatus = aacEncOpen(&hAacEncoder,0,1)) != AACENC_OK ) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_AOT, 2)) != AACENC_OK ) { // low-delay
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_SAMPLERATE, sampleRate)) != AACENC_OK ) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_CHANNELMODE, MODE_1)) != AACENC_OK ) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_CHANNELORDER, 1)) != AACENC_OK) {
            // fprintf(stderr, "Unable to set the wav channel order\n");
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_TRANSMUX, 0)) != AACENC_OK){
            return ErrorStatus;
        }
        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_AFTERBURNER, 0)) != AACENC_OK){
            return ErrorStatus;
        }

        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_BITRATEMODE, 0)) != AACENC_OK){
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }

        if ((ErrorStatus = aacEncoder_SetParam(hAacEncoder, AACENC_BITRATE, 1000)) != AACENC_OK ) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }

        if ((ErrorStatus = aacEncEncode(hAacEncoder, NULL, NULL, NULL, NULL)) != AACENC_OK) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " Unable to initialize the encoder: " << ErrorStatus << std::endl;
            return ErrorStatus;
        }

        // if ((ErrorStatus = aacEncInfo(hAacEncoder, &info_)) != AACENC_OK) {
        //     std::cout << "Unable to get encoder info: " <<  ErrorStatus << std::endl;
        //     return ErrorStatus;
        // }
        // std::cout << "info_.frameLength=" << info_.frameLength << std::endl;
        return 0;
    }

    virtual ReturnType encode(const std::vector<char> &pcmData, std::vector<char> &encodedData) override {
        AACENC_BufDesc in_buf = {0};
        void *in_ptr;
        in_ptr = (void *)pcmData.data();
        int bufferIdentifiers = IN_AUDIO_DATA;
        int bufSizes = pcmData.size();
        int bufElSizes = 2;
        
        in_buf.numBufs = 1;
        in_buf.bufs = &in_ptr;
        in_buf.bufferIdentifiers = &bufferIdentifiers;
        in_buf.bufSizes = &bufSizes;
        in_buf.bufElSizes = &bufElSizes;
    
        AACENC_BufDesc out_buf = {0};
        void *out_ptr;

        int out_buffer_identifier = OUT_BITSTREAM_DATA;
        int out_buffer_size, out_buffer_element_size;


        char outBuffer[1000];
        out_ptr = outBuffer;
        out_buffer_size           = 1000;
        out_buffer_element_size   = 1;
        out_buf.numBufs           = 1;
        out_buf.bufs              = &out_ptr;
        out_buf.bufferIdentifiers = &out_buffer_identifier;
        out_buf.bufSizes          = &out_buffer_size;
        out_buf.bufElSizes        = &out_buffer_element_size;


        AACENC_InArgs  in_args  = { 0 };
        AACENC_OutArgs out_args = { 0 };


        in_args.numInSamples     = pcmData.size()/2;

        std::cout << "out_buf.bufSizes=" << out_buf.bufSizes[0] << std::endl;

        if ((ErrorStatus = aacEncEncode(hAacEncoder, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK ) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " ErrorStatus=" <<  ErrorStatus << std::endl;
            return ErrorStatus;
        }
        std::cout << "out_args.numOutBytes=" << out_args.numOutBytes << std::endl;
        std::cout << "out_buf.bufSizes=" << out_buf.bufSizes[0] << std::endl;
        
        encodedData.resize(out_args.numOutBytes);
        memcpy(encodedData.data(), outBuffer, out_args.numOutBytes);

        return 0;
    }

    virtual ~FdkAacEncoder(){
        std::cout << __FILE__ << ":" <<  __LINE__ << std::endl;
    }
};
