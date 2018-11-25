#pragma once

#include <memory>
#include "NetPacket.hpp"
#include "Logger.hpp"
#include "AudioDecoder.hpp"
#include "JitterBuffer.hpp"

//////////////////////////////////////////////////////////////////////////
// Decoder, Jitter buffer, Packet loss concealment
// 
// input: received AudioMessage(opus)
// output: audio endpoint
//////////////////////////////////////////////////////////////////////////
class DownstreamProcessor {
private:
    JitterBuffer jitterBuffer_;
    AudioDecoder *decoder_ = nullptr;
    bool needAec_ = false;
    uint32_t largestReceivedMediaDataTimestamp_ = 0;
    uint32_t lastReceivedAudioSn_ = 0;
    void *aec_ = nullptr;
    std::shared_ptr<std::ofstream> dumpMono16le16kHzPcmFile_ = nullptr;
    std::shared_ptr<std::tuple<uint32_t, std::shared_ptr<PcmSegment>>> m1_ = nullptr; // -1 
    std::shared_ptr<std::tuple<uint32_t, std::shared_ptr<PcmSegment>>> m2_ = nullptr; // -2
private:
    void decodeOpusAndAecBufferFarend(const std::shared_ptr<NetPacket> netPacket, std::vector<short> &decodedPcm);
#if 0
    size_t availableSize() {
        auto result = jitterBuffer_.read_available();
        if (m1_)result++;
        if (m2_)result++;
        return result;
    }
#endif
public:
    DownstreamProcessor(bool needAec, void *aec, const std::string &audioOutDumpPath);
    ~DownstreamProcessor();
    void append(const std::shared_ptr<NetPacket> &netPacket);
    void fetch(PcmSegment &outData);
};