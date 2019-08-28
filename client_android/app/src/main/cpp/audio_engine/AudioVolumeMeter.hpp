#pragma once
#include <vector>
#include "AudioCommon.hpp"
#include <tuple>

class AudioVolumeMeter {
public:
    // mono
    virtual AudioIoVolume calculate(const PcmSegment &buffer)  = 0;
};

