#pragma once

#include "ReturnType.hpp"

#include <vector>

class AudioEncoder {
public:
    virtual ReturnType reInit() = 0;
    virtual ReturnType encode(const std::vector<short> &pcmData, std::vector<char> &encodedData) = 0;
    
    virtual ~AudioEncoder(){}
};

