#pragma once

#include <boost/lockfree/spsc_queue.hpp>

class JitterBuffer {
public:
    JitterBuffer(): encodedBuffer_(100){}
    boost::lockfree::spsc_queue<std::shared_ptr<NetPacket>> encodedBuffer_;
    void append(const std::shared_ptr<NetPacket> &netPacket) {
        // erase old data?
        encodedBuffer_.push(netPacket);
    }
    std::shared_ptr<NetPacket> fetch() {
        std::shared_ptr<NetPacket> out = nullptr;
        encodedBuffer_.pop(out);
        return out;
    }
    auto read_available() { return encodedBuffer_.read_available(); }
};