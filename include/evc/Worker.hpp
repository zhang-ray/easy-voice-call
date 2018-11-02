#pragma once

#include <memory>
#include <thread>
#include <string>
#include "evc/RingBuffer.hpp"
#include "evc/AudioVolume.hpp"

class AudioDecoder;
class AudioEncoder;
class AudioDevice;


enum class AudioInOut : uint8_t{
    In,
    Out
};


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


class Worker {
private:
    AudioDecoder * decoder = nullptr;
    AudioEncoder * encoder = nullptr;
    AudioDevice *  device_ = nullptr;
    bool gotoStop_ = false;
    std::shared_ptr<std::thread> netThread_ = nullptr;
    std::function<void(const AudioIoVolume)>  volumeReporter_ = nullptr;
    std::function<void(const bool)>  vadReporter_ = nullptr;
    bool needAec_ = false;

    uint8_t vadCounter_ = 0; // nbActivated
    bool mute_ = false;
    bool needSend_ = true;
    SuckAudioVolume sav;
private:
    std::shared_ptr<std::thread> playbackThread_ = nullptr;
    RingBuffer s2cPcmBuffer_;
public:
    Worker(bool needAec);
    ~Worker();

    void setMute(bool mute){mute_ = mute;}
    bool initCodec();
    bool initDevice(std::function<void(const std::string &,const std::string &)> reportInfo,
                    std::function<void(const AudioIoVolume)> reportVolume,
                    std::function<void(const bool)> vadReporter
                    );

    void asyncStart(const std::string &host, const std::string &port,
                      std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                      );

    void syncStart(const std::string &host, const std::string &port,
                      std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                      );


    ///// TODO: blocked when server down?
    void syncStop(){
        gotoStop_  = true;
        if (netThread_){
            if(netThread_->joinable()){
                netThread_->join();
            }
            netThread_ = nullptr;
        }
        if (playbackThread_){
            if (playbackThread_->joinable()){
                playbackThread_->join();
            }
            playbackThread_=nullptr;
        }
    }
};
