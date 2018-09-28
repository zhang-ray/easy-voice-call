#pragma once

#include "ReturnType.hpp"

#include <vector>

class AudioDecoder {
public:
    virtual ReturnType reInit() = 0;
    virtual ReturnType decode(const std::vector<char> &encodedData, std::vector<short> &pcmData) = 0;
    
    virtual ~AudioDecoder(){}
};

