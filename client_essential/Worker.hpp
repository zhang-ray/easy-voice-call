#pragma once

#include <memory>
#include <thread>
#include <string>
#include <fstream>
#include "Logger.hpp"
#include "Lib.hpp"

#include "CallbackStylePortaudioEndpoint.hpp"
#include "Profiler.hpp"
#include "ReturnType.hpp"
#include "NetClient.hpp"
#include <vector>
#include <climits>
#include <fstream>

#include "IWorker.hpp"

#include "audio_engine/AudioInStub.hpp"
#include "audio_engine/DownstreamProcessor.hpp"
#include "audio_engine/AudioVolumeMeter_RMS.hpp"
#include "audio_engine/AudioVolumeMeter_Peak.hpp"


#include "../audio-processing-module/independent_cng/src/webrtc_cng.hpp"


class AudioEncoder;




class Worker : public IWorker {
private:
    CallbackStyleAudioEndpoint *endpoint_ = nullptr;

    AudioEncoder * encoder = nullptr;
    bool gotoStop_ = false;
    std::shared_ptr<std::thread> netThread_ = nullptr;
    std::function<void(const AudioIoVolume)>  volumeReporter_ = nullptr;
    std::function<void(const bool)>  vadReporter_ = nullptr;
    std::function<void(const uint32_t)> durationReporter_ = nullptr;
    boost::property_tree::ptree configRoot_;
    bool needAec_ = false;

    uint8_t vadCounter_ = 0; // nbActivated
    bool mute_ = false;
    bool sendOpus_ = true;
    AudioVolumeMeter_Peak volumeMeterIn_;
    AudioVolumeMeter_Peak volumeMeterOut_;
    std::shared_ptr<std::thread> durationTimer_ = nullptr;
    std::shared_ptr<NetClient> pClient = nullptr;
    std::shared_ptr<DownstreamProcessor> downStreamProcessor_ = nullptr;
    std::shared_ptr<AudioInStub> audioInStub_ = nullptr;
    std::shared_ptr<std::vector<int16_t>> audioOutStub_ = nullptr;

    uint32_t sn_sendingAudio_ = 0;
    uint32_t sn_sendingHeartBeat_ = 0;
private:
	webrtc::ComfortNoiseEncoder cnEncoder;
private:
    bool initCodec();
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

    EVC_API void asyncStart(std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState);

    EVC_API ReturnType syncStart(std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState);


    ///// TODO: blocked when server down?
    EVC_API void syncStop();
};
