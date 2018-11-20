#pragma once 

#include "AudioCommon.hpp"


class PacketLossConcealment {
public:
    enum class Type : uint8_t {
        // https://en.wikipedia.org/wiki/Packet_loss_concealment
        ZeroInsertion,
        WaveformSubstitution,
        ModelBasedMethods
    };

    virtual bool predict(const PcmSegment *, const PcmSegment outputSegment) = 0;
};

class PlcZeroInsertion : public PacketLossConcealment{

};
