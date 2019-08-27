#pragma once 

#include "AudioEncoder.hpp"
#include "Singleton.hpp"
#include <cstring>
#include <set>

#if defined(WIN32) || defined(ANDROID)
#include <opus.h>
#else
#include <opus/opus.h>
#endif
#include "AudioCommon.hpp"

#define MAX_PACKET_SIZE (3*1276)

// libopus 1.2.1
// it seems like Opus' API is very easy to use.
class OpusEnc final : public AudioEncoder, public Singleton<OpusEnc> {
private:
    int err;
    OpusEncoder *encoder = nullptr;
public:
    virtual ReturnType reInit() override {
        encoder = opus_encoder_create(sampleRate, 1, OPUS_APPLICATION_VOIP, &err);
        if (err<0) {
            return err;
        }

        // err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000));
        // if (err < 0 ){
        //     return err;
        // }
        
        return 0;
    }

    virtual ReturnType encode(const std::vector<short> &pcmData, std::vector<char> &encodedData) override {
        unsigned char cbits[MAX_PACKET_SIZE];
        /* Encode the frame. */
        auto frame_size = pcmData.size();
        auto nbBytes = opus_encode(encoder, pcmData.data(), frame_size, cbits, MAX_PACKET_SIZE);
        if (nbBytes<0) {
            fprintf(stderr, "%s:%d encode failed: %s\n", __FILE__, __LINE__, opus_strerror(nbBytes));
            return EXIT_FAILURE;
        }


        encodedData.resize(nbBytes);
        memcpy(encodedData.data(), cbits, nbBytes);

        return 0;
    }
    
    virtual ~OpusEnc(){}
};
