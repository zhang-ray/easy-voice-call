#pragma once 

#include "AudioCommon.hpp"
#include <memory>


class PacketLossConcealment {
public:
    enum class Type : uint8_t {
        // https://en.wikipedia.org/wiki/Packet_loss_concealment
        ZeroInsertion,
        WaveformSubstitution,
        ModelBasedMethods
    };


    virtual bool predict(std::vector<std::shared_ptr<PcmSegment>> older, std::shared_ptr<PcmSegment> &outputSegment) = 0;
};
