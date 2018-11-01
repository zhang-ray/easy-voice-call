#pragma once
#include <vector>

class AudioVolume {
public:
    // mono
    virtual uint8_t calculate(const std::vector<short> &buffer, const uint8_t MAX_VOLUME_LEVEL) = 0;
};


class SuckAudioVolume : public AudioVolume {
public:
    virtual uint8_t calculate(const std::vector<short> &buffer, const uint8_t MAX_VOLUME_LEVEL) override {
        double sum =0;
        for (const auto &sample: buffer){
            sum+=std::abs(sample);
        }
        sum *= MAX_VOLUME_LEVEL;
        auto ret = sum / (1<<15) / buffer.size();
        return ret ;
    }
};
