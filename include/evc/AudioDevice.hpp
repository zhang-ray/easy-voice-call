#pragma once

#include "ReturnType.hpp"

#include <vector>

/// for mic and speaker
enum {sampleRate = 16000};
enum {blockSize = sampleRate/100};

///
/// \brief The AudioDevice class
/// TODO:
///    - a mechanism to monitor and avoid overrun and underrun?
class AudioDevice {
public:
    virtual ReturnType init(std::string &micInfo, std::string &spkInfo) = 0;

    virtual ReturnType read(std::vector<short> &buffer) = 0;
    virtual ReturnType write(const std::vector<short> &buffer) = 0;

    virtual ~AudioDevice(){}
};
