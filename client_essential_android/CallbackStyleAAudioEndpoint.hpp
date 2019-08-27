#pragma once

#include "CallbackStyleAudioEndpoint.hpp"
#include "Singleton.hpp"

#include <memory>
#include <vector>

class CallbackStyleAAudioEndpoint : public CallbackStyleAudioEndpoint, public Singleton<CallbackStyleAAudioEndpoint>{
private:
    std::function<void(const int16_t *inputBuffer, int16_t *outputBuffer, const uint32_t framesPerBuffer)> callbackFunc_ = nullptr;
    

public:
    virtual ReturnType init(
        const std::function<void(const int16_t *inputBuffer, int16_t *outputBuffer, const uint32_t framesPerBuffer)> callbackFunc
    ) override {
        return 0;
    }


    virtual std::tuple<std::string, std::string> getEndpointName() override {
        // it's hard to get endpoint's name by PortAudio
        return{ "","" };
    }


    virtual ReturnType asyncStart() override {
        return 0;
    }


    virtual ReturnType syncStop() override {
        return 0;
    }

};
