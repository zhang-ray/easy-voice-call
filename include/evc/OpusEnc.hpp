#pragma once 

#include "AudioEncoder.hpp"
#include "Singleton.hpp"

#include <opus/opus.h>

#define MAX_PACKET_SIZE (3*1276)

// libopus 1.2.1
// it seems like Opus' API is very easy to use.
class OpusEnc final : public AudioEncoder, public Singleton<OpusEnc> {
private:
    int err;
    OpusEncoder *encoder = nullptr;
public:
    virtual ReturnType reInit() override {
        encoder = opus_encoder_create(48000, 1, OPUS_APPLICATION_VOIP, &err);
        if (err<0) {
            return err;
        }

        // err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000));
        // if (err < 0 ){
        //     return err;
        // }
        
        return 0;
    }

    virtual ReturnType encode(const std::vector<char> &pcmData, std::vector<char> &encodedData) override {
        printf("OpusEnc::encode\n");
        unsigned char cbits[MAX_PACKET_SIZE];
        /* Encode the frame. */
        /* For example, at 48 kHz the permitted values are 120, 240, 480, 960, 1920, and 2880 */

        auto frame_size = pcmData.size()/2;
        if ((frame_size != 480)&&(frame_size != 960)) {
            printf("invalid frame_size=%d\n", frame_size);
            return "invalid frame_size";
        }
        auto nbBytes = opus_encode(encoder, (const short *)pcmData.data(), frame_size, cbits, MAX_PACKET_SIZE);
        if (nbBytes<0) {
            fprintf(stderr, "%s:%d encode failed: %s\n", __FILE__, __LINE__, opus_strerror(nbBytes));
            return EXIT_FAILURE;
        }


        printf("nbBytes=%d\n", nbBytes);
        encodedData.resize(nbBytes);
        memcpy(encodedData.data(), cbits, nbBytes);

        return 0;
    }
    
    virtual ~OpusEnc(){}
};