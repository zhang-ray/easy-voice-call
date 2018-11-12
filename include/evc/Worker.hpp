#pragma once

#include <memory>
#include <thread>
#include <string>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Logger.hpp"
#include "Lib.hpp"
#include "evc/RingBuffer.hpp"
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
    RingBuffer spkBuffer_;
    Profiler profiler_;
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
