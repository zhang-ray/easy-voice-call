#pragma once

#include <memory>
#include <thread>
#include <QDebug>

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

public:
    ~Worker();

    bool initCodec();
    bool initDevice();

    void asyncStart(const std::string &host, const std::string &port,
                      std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                      );

    void syncStart(const std::string &host, const std::string &port,
                      std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                      );
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
