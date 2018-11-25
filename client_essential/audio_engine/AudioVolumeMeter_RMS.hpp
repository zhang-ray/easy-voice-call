#pragma once
#include <vector>
#include "AudioVolumeMeter.hpp"

class AudioVolumeMeter_RMS : public AudioVolumeMeter {
    uint8_t calculate(const PcmSegment &buffer, const uint8_t MAX_VOLUME_LEVEL) {
        auto sum = 0.0;
        for (int i = 0; i < buffer.size(); i++) {
            auto hehe = ((double)buffer[i] / (1 << 15)) * ((double)buffer[i] / (1 << 15));
            sum += hehe;
        }
        sum /= buffer.size();
        if (true/*scaling up*/) {
            sum = sqrt(sum);
        }
        return sqrt(sum) * MAX_VOLUME_LEVEL;
    }

    AudioInOut inOut_;

public:
    AudioVolumeMeter_RMS(AudioInOut inOut) :inOut_(inOut) {}

    AudioIoVolume calculate(const PcmSegment &buffer) {
        auto currentLevel = calculate(buffer, AudioIoVolume::MAX_VOLUME_LEVEL);
        static uint8_t recentMaxLevel = currentLevel;
        static auto lastTimeStamp = std::chrono::system_clock::now();;

        auto now = std::chrono::system_clock::now();
        auto elapsed = now - lastTimeStamp;

        // hold on 1s
        if (elapsed > std::chrono::seconds(1)) {
            recentMaxLevel = 0;
            lastTimeStamp = std::chrono::system_clock::now();
        }


        if (currentLevel > recentMaxLevel) {
            recentMaxLevel = currentLevel;
            // re calculate hold-on time
            lastTimeStamp= std::chrono::system_clock::now();
        }

        return { inOut_, currentLevel, recentMaxLevel };
    }

};
