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
#include "ReturnType.hpp"
#include "NetClient.hpp"
#include <vector>
#include <cmath>
#include <numeric>

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
    /* TODO
    statistics media data
      - packet order
      - arrival time's Evenness 
      - 
    */
    class Statistician {
    private:
        std::vector<uint32_t> durationList;
    public:
        Statistician() {}
        Statistician(const Statistician&) = delete;
        
        void addDurationMs(const uint32_t dur) {
            durationList.push_back(dur);
        }

        std::string calc(){
            const auto &v = durationList;
            double sum = std::accumulate(v.begin(), v.end(), 0.0);
            double mean = sum / v.size();

            double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
            double stdev = std::sqrt(sq_sum / v.size() - mean * mean);

            return "[" + std::to_string(mean) + "±"  + std::to_string(stdev) + "]";
        }

        ~Statistician() {
            LOGI << calc();
        }
    };


    class Timer {
    private:
        Statistician *stat_ = nullptr;
        decltype(std::chrono::system_clock::now()) startPoint_;
    public:
        Timer(Statistician *stat)
            : stat_(stat)
        {
            startPoint_ = std::chrono::system_clock::now();
        }

        ~Timer() {
            auto dur = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startPoint_).count());
            if (stat_) {
                stat_->addDurationMs(dur);
            }
        }
    };


    class Profiler {
    public:
        Statistician __1;
        Statistician __2;
    };


private:
    CallbackStyleAudioEndpoint *endpoint_ = nullptr;
    AudioDecoder * decoder = nullptr;
    AudioEncoder * encoder = nullptr;
    bool gotoStop_ = false;
    bool gotoStopTimer_ = false;
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
    void stopTimer();
    void nsAecVolumeVadSend(const short *buffer);
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
        std::shared_ptr<std::ofstream> dumpMono16le16kHzPcmFile,
        std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
    );

    EVC_API void syncStart(const std::string &host, const std::string &port,
        std::shared_ptr<std::ofstream> dumpMono16le16kHzPcmFile,
        std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
    );


    ///// TODO: blocked when server down?
    EVC_API void syncStop();
};
