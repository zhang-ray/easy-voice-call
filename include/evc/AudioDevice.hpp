#pragma once

#include "ReturnType.hpp"

class AudioDevice {
public:

    virtual ReturnType init() = 0;
    virtual int read(char *pBuffer, int nBytes) = 0;
    virtual int write(const char *pBuffer, int nBytes) = 0;

    virtual ~AudioDevice(){}
};
