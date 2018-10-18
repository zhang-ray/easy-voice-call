#pragma once

#include <memory>
#include <thread>
#include <QDebug>
#include <string>

class AudioDecoder;
class AudioEncoder;
class AudioDevice;

enum class NetworkState : unsigned char{
    Disconnected,
    Connecting,
    Connected,
};


/// TODO:
///    - make timestamp
///    - use reliable UDP in weak network
///    - check version (server & client)
///    - display build date and git commit version in CLI/GUI and server side


class Worker {
private:
    AudioDecoder * decoder = nullptr;
    AudioEncoder * encoder = nullptr;
    AudioDevice *  device_ = nullptr;
    bool gotoStop_ = false;
    std::shared_ptr<std::thread> netThread_ = nullptr;
    std::function<void(const uint8_t)>  micVolumeReporter_ = nullptr;
    std::function<void(const uint8_t)>  spkVolumeReporter_ = nullptr;

public:
    ~Worker();

    bool initCodec();
    bool initDevice(std::function<void(const std::string &,const std::string &)> reportInfo,
                    std::function<void(const uint8_t)> reportMicVolume,
                    std::function<void(const uint8_t)> reportSpkVolume);

    void asyncStart(const std::string &host,
                      std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                      );

    void syncStart(const std::string &host,
                      std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                      );


    ///// TODO: blocked when server down?
    void syncStop(){
        if (netThread_){
            gotoStop_  = true;
            if(netThread_->joinable()){
                netThread_->join();
            }
            netThread_ = nullptr;
        }
    }
};
