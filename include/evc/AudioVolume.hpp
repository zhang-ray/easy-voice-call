#pragma once
#include <vector>

class AudioVolume {
public:
    // mono
    virtual uint8_t calculate(const std::vector<short> &buffer) = 0;
};


class SuckAudioVolume : public AudioVolume {
public:
    virtual uint8_t calculate(const std::vector<short> &buffer) override {
        double sum =0;
        for (const auto &sample: buffer){
            sum+=std::abs(sample);
        }
        sum *= 100;
        auto ret = sum / (1<<15) / buffer.size();
        return ret ;
    }
};
