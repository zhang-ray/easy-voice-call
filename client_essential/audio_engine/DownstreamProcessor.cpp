#include "DownstreamProcessor.hpp"
#include "AudioCommon.hpp"

#include "../../audio-processing-module/independent_aec/src/echo_cancellation.h"
#include "Profiler.hpp"

void DownstreamProcessor::decodeOpusAndAecBufferFarend(const std::shared_ptr<NetPacket> netPacket, std::vector<short> &decodedPcm)
{
    std::vector<char> netBuff;
    netBuff.resize(netPacket->payloadLength());
    memcpy(netBuff.data(), netPacket->payload(), netPacket->payloadLength());
    decoder_->decode(netBuff, decodedPcm);

    if (needAec_) {
        std::vector<float> floatFarend(decodedPcm.size());
        for (auto i = 0u; i < decodedPcm.size(); i++) {
            floatFarend[i] = ((float)decodedPcm[i]) / (1 << 15);
        }
        for (int i = 0; i < blockSize; i += blockSize) {
            auto ret = webrtc::WebRtcAec_BufferFarend(aec_, &(floatFarend[i]), 160);
            if (ret) {
                throw;
            }
        }
    }


    {
        /// RUNTIME VERIFICATION
        /// UNEXPECTED: descending order
        auto currentTS = netPacket->timestamp();
        if (currentTS < largestReceivedMediaDataTimestamp_) {
            // unexpected data order!!!!
            LOGE << "currentTS < largestReceivedMediaDataTimestamp_";
            // TODO: AND THEN? THROW?
            throw;
        }
        else {
            largestReceivedMediaDataTimestamp_ = currentTS;
        }
    }

    {
        /// RUNTIME VERIFICATION
        /// SN 
        auto currentSn = netPacket->serialNumber();
        if (!lastReceivedAudioSn_) {
            if (lastReceivedAudioSn_ + 1 != currentSn) {
                LOGE << "lastReceivedAudioSn_ + 1 != currentSn";
                LOGE << "lastReceivedAudioSn_=" << lastReceivedAudioSn_;
                LOGE << "currentSn=" << currentSn;
            }
        }
        lastReceivedAudioSn_ = currentSn;
    }
}

DownstreamProcessor::DownstreamProcessor(bool needAec, void *aec, const std::string &audioOutDumpPath)
    :encodedBuffer_(100)
    , decoder_(&AudioDecoder::create())
    , needAec_(needAec)
    , aec_(aec)
{
    if (decoder_) {
        decoder_->reInit();
    }

    if (!audioOutDumpPath.empty()) {
        dumpMono16le16kHzPcmFile_ = std::make_shared<std::ofstream>(audioOutDumpPath, std::ofstream::binary /*don't miss std::ofstream::binary*/);
        if (!dumpMono16le16kHzPcmFile_->is_open()) {
            dumpMono16le16kHzPcmFile_ = nullptr;
        }
    }
}

DownstreamProcessor::~DownstreamProcessor()
{
    if (dumpMono16le16kHzPcmFile_) {
        if (dumpMono16le16kHzPcmFile_->is_open()) {
            dumpMono16le16kHzPcmFile_->close();
        }
    }
}

void DownstreamProcessor::fetch(int16_t * const outData)
{
    std::shared_ptr<NetPacket> packet = nullptr;
    std::vector<int16_t> data;
    if (encodedBuffer_.pop(packet)) {
        decodeOpusAndAecBufferFarend(packet, data);
        memcpy(outData, data.data(), blockSize * sizeof(int16_t));

        Profiler::get().packageDelayList_.addData(
            (int32_t)(ProcessTime::get().getProcessUptime()) -
            (int32_t)(packet->timestamp())
        );
    }
    else {
        Profiler::get().emptyAudioOutBuffer_.addData(true);
    }

    if (dumpMono16le16kHzPcmFile_) {
        dumpMono16le16kHzPcmFile_->write((char*)outData, sizeof(int16_t)*blockSize);
    }
}