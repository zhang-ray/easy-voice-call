#pragma once
#include "NetPacket.hpp"
#include <cstring>

class Participant {
public:
    virtual ~Participant() {}
    virtual void deliver(const NetPacket& msg) = 0;
    virtual std::string info() = 0;
};

using ParticipantPointer = std::shared_ptr<Participant>;
using ClientPacketQueue = std::deque<NetPacket>;

