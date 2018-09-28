#pragma once

#include "AudioDecoder.hpp"
#include "Singleton.hpp"
#include <vector>
#include <cstring>

#ifdef WIN32
#include <opus.h>
#else
#include <opus/opus.h>
#endif

// libopus 1.2.1
class OpusDec final : public AudioDecoder,public Singleton<OpusDec>{
private:
    int err;
    OpusDecoder *decoder = nullptr;
public:
    virtual ReturnType reInit() override {
        decoder = opus_decoder_create(48000, 1, &err);
        if (err<0) {
            return err;
        }
        return 0;
    }


#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)
    virtual ReturnType decode(const std::vector<char> &encodedData, std::vector<char> &pcmData) override {
        short out[MAX_FRAME_SIZE];
        auto nbBytes = encodedData.size();
        printf("nbBytes=%d\n", nbBytes);
        auto frame_size = opus_decode(decoder, (unsigned char*)encodedData.data(), nbBytes, out, MAX_FRAME_SIZE, 0);
        if (frame_size<0) {
            fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
            return EXIT_FAILURE;
        }
        printf("frame_size=%d\n", frame_size);

        pcmData.resize(frame_size*2);
        memcpy(pcmData.data(), out, pcmData.size());
        return 0;
    }
    
    virtual ~OpusDec(){}
};

