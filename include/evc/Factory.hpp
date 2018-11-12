#pragma once

#include "Singleton.hpp"
#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"
#include "CallbackStyleAudioEndpoint.hpp"

// TODO: naming misleading
class Factory : public Singleton<Factory> {
public:
    CallbackStyleAudioEndpoint &createCallbackStyleAudioEndpoint();
    AudioEncoder &createAudioEncoder();
    AudioDecoder &createAudioDecoder();

    Factory(){}
    Factory(const Factory&) = delete;
    Factory(const Factory&&) = delete;
    Factory& operator=(Factory const&) = delete;
    Factory& operator=(Factory &&) = delete;
};
