#pragma once

#include "Singleton.hpp"
#include "AudioDevice.hpp"
#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"

// TODO: naming misleading
class Factory : public Singleton<Factory> {
public:
    AudioDevice &create();
    AudioEncoder &createAudioEncoder();
    AudioDecoder &createAudioDecoder();

    Factory(){}
    Factory(const Factory&) = delete;
    Factory(const Factory&&) = delete;
    Factory& operator=(Factory const&) = delete;
    Factory& operator=(Factory &&) = delete;
};
