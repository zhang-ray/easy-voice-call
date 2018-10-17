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



class Worker {
private:
    AudioDecoder * decoder = nullptr;
    AudioEncoder * encoder = nullptr;
    AudioDevice *  device_ = nullptr;
    bool gotoStop_ = false;
    std::shared_ptr<std::thread> netThread_ = nullptr;
    std::function<void(const double)>  micVolumeReporter_ = nullptr;
    std::function<void(const double)>  spkVolumeReporter_ = nullptr;

public:
    ~Worker();

    bool initCodec();
    bool initDevice(std::function<void(const std::string &,const std::string &)> reportInfo, std::function<void(const double)> reportMicVolume, std::function<void(const double)> reportSpkVolume);

    void asyncStart(const std::string &host, const std::string &port,
                      std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                      );

    void syncStart(const std::string &host, const std::string &port,
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
