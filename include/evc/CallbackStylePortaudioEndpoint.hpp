#pragma once

#include "CallbackStyleAudioEndpoint.hpp"
#include "Singleton.hpp"

#include <portaudio.h>

class CallbackStylePortaudioEndpoint : public CallbackStyleAudioEndpoint, public Singleton<CallbackStylePortaudioEndpoint>{
private:
    PaStream *stream_ = nullptr;
    std::function<void(int16_t *inputBuffer, int16_t *outputBuffer, const uint32_t framesPerBuffer)> callbackFunc_ = nullptr;
    
    static int sFuncPaStreamCallback(
        const void *input, void *output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData) {
        auto instance = static_cast<CallbackStylePortaudioEndpoint*> (userData);
        if (instance->callbackFunc_) {
            instance->callbackFunc_(
                (int16_t *)input,
                (int16_t *)output,
                (uint32_t)frameCount
            );
        }
        return paContinue;
    }

public:
    virtual ReturnType init(const std::function<void(int16_t *inputBuffer, int16_t *outputBuffer, const uint32_t framesPerBuffer)> callbackFunc) override {
        auto err = Pa_Initialize();
        if (err != paNoError) {
            return Pa_GetErrorText(err);
        }

        /* Open an audio I/O stream. */
        err = Pa_OpenDefaultStream(
            &stream_,
            1, 1,
            paInt16, sampleRate,
            blockSize,
            sFuncPaStreamCallback,
            this);
        if (err != paNoError) {
            // error
            return err;
        }

        callbackFunc_ = callbackFunc;
        return 0;
    }


    virtual std::tuple<std::string, std::string> getEndpointName() override {
        // it's hard to get endpoint's name by PortAudio
        return{ "","" };
    }


    virtual ReturnType asyncStart() override {
        auto err = Pa_StartStream(stream_);
        if (err != paNoError) {
            // error
            return err;
        }
        return 0;
    }


    virtual ReturnType syncStop() override {
        auto err = Pa_StopStream(stream_);
        if (err != paNoError) {
            //printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        }
        return 0;
    }

};