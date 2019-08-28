#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include <memory>
#include "AudioCommon.hpp"

class JitterBuffer {
public:
    JitterBuffer(): decodedBuffer_(100){}
    boost::lockfree::spsc_queue<std::shared_ptr<std::tuple<uint32_t, std::shared_ptr<PcmSegment>>>> decodedBuffer_;
    void append(const std::shared_ptr<std::tuple<uint32_t, std::shared_ptr<PcmSegment>>> &pcm) {
        // erase old data?
        decodedBuffer_.push(pcm);
    }
    std::shared_ptr<std::tuple<uint32_t, std::shared_ptr<PcmSegment>>> fetch() {
        std::shared_ptr<std::tuple<uint32_t, std::shared_ptr<PcmSegment>>> out = nullptr;
        decodedBuffer_.pop(out);
        return out;
    }
    auto read_available() { return decodedBuffer_.read_available(); }
};