#pragma once

#include <array>

/// for mic and speaker
enum { sampleRate = 16000 };
enum { blockSize = sampleRate / 100 };

enum class AudioInOut : uint8_t {
    In,
    Out
};


class AudioIoVolume {
public:
    enum { MAX_VOLUME_LEVEL = 10 };

    using Level = uint8_t;

    AudioInOut io_;
    Level level_;
    Level recentMaxLevel_;
public:
    AudioIoVolume(const AudioInOut io, const Level level, const Level recentMaxLevel)
        :io_(io)
        , level_(level)
        , recentMaxLevel_(recentMaxLevel)
    {

    }
};

using PcmSegment = std::array<int16_t, blockSize>;

