#pragma once

#include <vector>

class GeneratedWave {
public:
    virtual ~GeneratedWave(){}
};


class SineWave : public GeneratedWave {
private:
    std::vector<int16_t> wholePcm_;
public:
    double pi() { return std::atan(1) * 4; }
    SineWave(size_t sampleRate, double freq, double maxAmplitude, double durationSecond) {
        auto size_ = sampleRate * durationSecond;
        wholePcm_.resize(size_);
        for (int i = 0; i < size_; i++) {
            wholePcm_[i] = maxAmplitude * SHRT_MAX * std::sin(2 * pi() * freq / sampleRate * i);
        }
#ifdef DUMP_GeneratedWave
        DataDumper<int16_t> d("AudioInStub.txt");
        d.dump(wholePcm_);
#endif // _DEBUG
    }
    operator std::vector<int16_t>() const { return wholePcm_; }
};

