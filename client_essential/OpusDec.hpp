#pragma once

#include "AudioDecoder.hpp"
#include "Singleton.hpp"
#include <vector>
#include <cstring>

#if defined(WIN32) || defined(ANDROID)
#include <opus.h>
#else
#include <opus/opus.h>
#endif
#include "AudioCommon.hpp"

// libopus 1.2.1
class OpusDec final : public AudioDecoder,public Singleton<OpusDec>{
private:
    int err;
    OpusDecoder *decoder = nullptr;
public:
    virtual ReturnType reInit() override {
        decoder = opus_decoder_create(sampleRate, 1, &err);
        if (err<0) {
            return err;
        }
        return 0;
    }


#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)
    virtual ReturnType decode(const std::vector<char> &encodedData, std::vector<short> &pcmData) override {
        short out[MAX_FRAME_SIZE];
        auto nbBytes = encodedData.size();
        auto frame_size = opus_decode(decoder, (unsigned char*)encodedData.data(), nbBytes, out, MAX_FRAME_SIZE, 0);
        if (frame_size<0) {
            fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
            return EXIT_FAILURE;
        }
        
        pcmData.resize(frame_size);
        memcpy(pcmData.data(), out, pcmData.size()*2);
        return 0;
    }
    
    virtual ~OpusDec(){}
};

