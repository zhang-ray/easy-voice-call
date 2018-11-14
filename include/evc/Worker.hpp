#pragma once

#include <memory>
#include <thread>
#include <string>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include "Logger.hpp"
#include "Lib.hpp"
#include "evc/AudioVolume.hpp"
#include "evc/CallbackStylePortaudioEndpoint.hpp"
#include "evc/Profiler.hpp"
#include "ReturnType.hpp"
#include "NetClient.hpp"
#include <vector>
class AudioDecoder;
class AudioEncoder;





class AudioIoVolume {
public:
    enum { MAX_VOLUME_LEVEL = 10};

    using Level = uint8_t;

    AudioInOut io_;
    Level level_;
    Level recentMaxLevel_;
public:
    AudioIoVolume(const AudioInOut io, const Level level, const Level recentMaxLevel)
        :io_(io)
        ,level_(level)
    , recentMaxLevel_(recentMaxLevel)
    {

    }
};

enum class NetworkState : unsigned char{
    Disconnected,
    Connecting,
    Connected,
};



/*
TODO
- Packet loss compensation
- cross fading for PLC
// https://www.boost.org/doc/libs/1_67_0/doc/html/boost/lockfree/spsc_queue.html  ?
*/

class AudioOutBuffer {
private:
    boost::lockfree::spsc_queue<std::array<int16_t, blockSize>> buffer_;
    std::array<int16_t, blockSize> emptyBuffer_;
public:
    AudioOutBuffer()
        : buffer_(100)
    {
        memset(emptyBuffer_.data(), 0, emptyBuffer_.size() * sizeof(int16_t));
    }

    void fetch(int16_t *outData) {
        std::array<int16_t, blockSize> data;
        if (buffer_.pop(data)) {
            std::memcpy((void*)outData, (void*)data.data(), sizeof(int16_t)*blockSize);
        }
        else {
            Profiler::get().emptyAudioOutBufferTS_.mark();
            std::memcpy((char*)outData, (char*)emptyBuffer_.data(), emptyBuffer_.size() * sizeof(int16_t));
        }
    }

    void insert(const int16_t *inData) {
        std::array<int16_t, blockSize> data;
        std::memcpy(data.data(), inData, sizeof(int16_t)*blockSize);
        if (!buffer_.push(data)) {
            LOGD << " could not pushElements from spkBuffer_";
        }
    }
};

/// TODO:
///    - make timestamp
///    - use reliable UDP in weak network
///    - check version (server & client)
///    - display build date and git commit version in GUI and server side


class IWorker {
public:
    virtual ~IWorker() {}
    virtual ReturnType init(
        const boost::property_tree::ptree &configRoot,
        std::function<void(const std::string &, const std::string &)> reportInfo,
        std::function<void(const AudioIoVolume)> reportVolume,
        std::function<void(const bool)> vadReporter
    ) = 0;
};

class Worker : public IWorker {
private:
    CallbackStyleAudioEndpoint *endpoint_ = nullptr;
    AudioDecoder * decoder = nullptr;
    AudioEncoder * encoder = nullptr;
    bool gotoStop_ = false;
    std::shared_ptr<std::thread> netThread_ = nullptr;
    std::function<void(const AudioIoVolume)>  volumeReporter_ = nullptr;
    std::function<void(const bool)>  vadReporter_ = nullptr;
    std::function<void(const uint32_t)> durationReporter_ = nullptr;
    bool needAec_ = false;

    uint8_t vadCounter_ = 0; // nbActivated
    bool mute_ = false;
    bool needSend_ = true;
    bool needNetworkStub_ = false;
    SuckAudioVolume sav;
    std::shared_ptr<std::thread> durationTimer_ = nullptr;
    std::shared_ptr<NetClient> pClient = nullptr;
    uint32_t largestReceivedMediaDataTimestamp_ = 0;
    bool bypassLocalAudioEndpoing_ = false;
    /*
    RingBuffer micBuffer_;
    */
    AudioOutBuffer audioOutBuffer_;

    uint32_t sn_sendingAudio_ = 0;
    uint32_t sn_sendingHeartBeat_ = 0;
    uint32_t lastReceivedAudioSn_ = 0;
private:
    bool initCodec();
    void nsAecVolumeVadSend(const short *buffer);
    void decodeOpusAndAecBufferFarend(const NetPacket& netPacket);
    void sendHeartbeat();
public:
    EVC_API Worker();
    EVC_API ~Worker();
    EVC_API virtual ReturnType init(
        const boost::property_tree::ptree &configRoot,
        std::function<void(const std::string &, const std::string &)> reportInfo,
        std::function<void(const AudioIoVolume)> reportVolume,
        std::function<void(const bool)> vadReporter
    ) override;
    
    EVC_API void setDurationReporter(decltype(durationReporter_) __);
    EVC_API void setMute(bool mute);

    EVC_API void asyncStart(const std::string &host, const std::string &port,
        std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
    );

    EVC_API void syncStart(const std::string &host, const std::string &port,
        std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
    );


    ///// TODO: blocked when server down?
    EVC_API void syncStop();
};
