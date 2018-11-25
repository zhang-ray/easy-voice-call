#pragma once


class AudioVolumeMeter_Peak : public AudioVolumeMeter{
    AudioInOut inOut_;
public:
    AudioVolumeMeter_Peak(AudioInOut inOut) :inOut_(inOut) {}

    AudioIoVolume calculate(const PcmSegment &buffer) {
        AudioIoVolume::Level currentLevel = 0;
        int currentL = 0;
        for (auto &b : buffer) {
            auto absB = std::abs(b);
            if (absB > currentL) {
                currentL = absB;
            }
        }
        currentLevel = (currentL*AudioIoVolume::MAX_VOLUME_LEVEL >> 15);
        if (currentLevel > AudioIoVolume::MAX_VOLUME_LEVEL-1) {
            currentLevel = AudioIoVolume::MAX_VOLUME_LEVEL-1;
        }


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
            lastTimeStamp = std::chrono::system_clock::now();
        }

        return{ inOut_, currentLevel, recentMaxLevel };
    }


};