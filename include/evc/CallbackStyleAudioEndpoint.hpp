#pragma once

#include <vector>
#include <tuple>
#include <functional>
#include "AudioCommon.hpp"
#include "ReturnType.hpp"

//////////////////////////////////////////////////////////////////////////
////  callback style AudioEndpoint  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CallbackStyleAudioEndpoint {
public:
    virtual ~CallbackStyleAudioEndpoint() { }
    virtual ReturnType init(
        const std::function<void(int16_t *inputBuffer, int16_t *outputBuffer, const uint32_t framesPerBuffer)> callbackFunc
    ) = 0;
    virtual ReturnType asyncStart() = 0;
    virtual ReturnType syncStop() = 0;
    virtual std::tuple<std::string, std::string> getEndpointName() = 0;
};