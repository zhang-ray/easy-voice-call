#pragma once

#include "ReturnType.hpp"

#include <vector>


class AudioDevice {
public:

    virtual ReturnType init() = 0;

    virtual ReturnType read(std::vector<short> &buffer) = 0;
    virtual ReturnType write(const std::vector<short> &buffer) = 0;

    virtual ~AudioDevice(){}
};
