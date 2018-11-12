#pragma once

#include "CallbackStyleAudioEndpoint.hpp"
#include "Singleton.hpp"

#include <portaudio.h>
#include <memory>
#include <vector>

class CallbackStylePortaudioEndpoint : public CallbackStyleAudioEndpoint, public Singleton<CallbackStylePortaudioEndpoint>{
private:
    PaStream *stream_ = nullptr;
    std::shared_ptr<std::vector<int16_t>> stubMic_ = nullptr;
    size_t fakeAudioInOffset = 0;
    std::function<void(const int16_t *inputBuffer, int16_t *outputBuffer, const uint32_t framesPerBuffer)> callbackFunc_ = nullptr;
    
    int16_t * getStubMic() {
        if (stubMic_ == nullptr) { return nullptr; }
        int16_t *myMicPointer = nullptr;
        if (fakeAudioInOffset + blockSize > stubMic_->size()) {
            fakeAudioInOffset = 0;
        }
        myMicPointer = stubMic_->data()+fakeAudioInOffset;
        fakeAudioInOffset += blockSize;
        return myMicPointer;
    }

    static int sFuncPaStreamCallback(
        const void *input, void *output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData) {
        auto instance = static_cast<CallbackStylePortaudioEndpoint*> (userData);
        if (instance->callbackFunc_) {
            auto myMicPointer = instance->getStubMic();
            if (myMicPointer == nullptr)myMicPointer =(decltype(myMicPointer))input;
            instance->callbackFunc_(
                myMicPointer,
                (int16_t *)output,
                (uint32_t)frameCount
            );
        }
        return paContinue;
    }

public:
    virtual ReturnType init(
        std::shared_ptr<std::vector<int16_t>> stubMic, 
        const std::function<void(const int16_t *inputBuffer, int16_t *outputBuffer, const uint32_t framesPerBuffer)> callbackFunc
    ) override {
        auto err = Pa_Initialize();
        if (err != paNoError) {
            return Pa_GetErrorText(err);
        }

        /* Open an audio I/O stream. */
        err = Pa_OpenDefaultStream(
            &stream_,
            1,
            1,
            paInt16, sampleRate,
            blockSize,
            sFuncPaStreamCallback,
            this);
        if (err != paNoError) {
            // error
            return err;
        }

        callbackFunc_ = callbackFunc;
        stubMic_ = stubMic;
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