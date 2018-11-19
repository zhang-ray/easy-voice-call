#pragma once

/// for mic and speaker
enum { sampleRate = 16000 };
enum { blockSize = sampleRate / 100 };

enum class AudioInOut : uint8_t {
    In,
    Out
};

using OneSegment = std::array<int16_t, blockSize>;

