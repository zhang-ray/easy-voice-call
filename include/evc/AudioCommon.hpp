#pragma once

/// for mic and speaker
enum { sampleRate = 16000 };
enum { blockSize = sampleRate / 100 }; // TODO, make a proper name

enum class AudioInOut : uint8_t {
    In,
    Out
};
