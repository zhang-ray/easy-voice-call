#pragma once

#include "Singleton.hpp"
#include "AudioDevice.hpp"

class Factory : public Singleton<Factory> {
public:
    AudioDevice &create();

    Factory(){}
    Factory(const Factory&) = delete;
    Factory(const Factory&&) = delete;
    Factory& operator=(Factory const&) = delete;
    Factory& operator=(Factory &&) = delete;
};
