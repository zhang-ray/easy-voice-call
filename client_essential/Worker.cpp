#include "Worker.hpp"

#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"

#include "TcpClient.hpp"
#include "Logger.hpp"

//// TODO
//// don't include WEBRTC directly...
#include "../audio-processing-module/independent_vad/src/webrtc_vad.hpp"
#include "../audio-processing-module/independent_aec/src/echo_cancellation.h"
#include "../audio-processing-module/independent_ns/src/noise_suppression_x.hpp"

namespace {
VadInst* vad;
void *aec = nullptr;
decltype(WebRtcNsx_Create()) ns_ = nullptr;
}

using namespace webrtc;

Worker::Worker()
    :volumeMeterIn_(AudioInOut::In)
    ,volumeMeterOut_(AudioInOut::Out)
{

}

Worker::~Worker(){
    try{
        syncStop();
        Profiler::get().dump();
    }
    catch(std::exception &e){
        LOGE_STD_EXCEPTION(e);
    }
}

ReturnType Worker::init(
    const boost::property_tree::ptree &configRoot,
    std::function<void(const std::string &, const std::string &)> reportInfo,
    std::function<void(const AudioIoVolume)> reportVolume,
    std::function<void(const bool)> vadReporter
) {
    configRoot_ = configRoot;

    needAec_ = configRoot.get<bool>("needAec", needAec_);
    
    if (needAec_) {
        aec = WebRtcAec_Create();
        if (auto ret = WebRtcAec_Init(aec, sampleRate, sampleRate)) {
            throw "WebRtcAec_Init failed";
        }
    }


    downStreamProcessor_ = std::make_shared<DownstreamProcessor>(needAec_, aec, configRoot.get("audioOutDumpPath", ""));

    try {
        {
            vad = WebRtcVad_Create();
            if (WebRtcVad_Init(vad)) {
                return "WebRtcVad_Init failed";
            }

            if (WebRtcVad_set_mode(vad, 3)) {
                return "WebRtcVad_set_mode failed";
            }
        }

        {
            ns_ = WebRtcNsx_Create();
            if (WebRtcNsx_Init(ns_, sampleRate)) {
                return "WebRtcNsx_Init failed";
            }

            if (WebRtcNsx_set_policy(ns_, 3)) {
                return "WebRtcNsx_set_policy failed";
            }
        }


        if (!initCodec()) {
            return "initCodec fail";
        }

        audioInStub_ = AudioInStub::create(
            configRoot.get<std::string>("audioInStub", "")
        );

        endpoint_ = &CallbackStyleAudioEndpoint::create(
            configRoot.get<bool>("needRealAudioEndpoint", true)
        );


        auto ret = endpoint_->init([this](const int16_t *inputBuffer, int16_t *outputBuffer, const uint32_t framesPerBuffer) {
            auto finalInputBuffer = inputBuffer;
            if (audioInStub_) {
                finalInputBuffer = audioInStub_->get();
            }
            nsAecVolumeVadSend(finalInputBuffer);

            PcmSegment outData;
            if (downStreamProcessor_) {
                downStreamProcessor_->fetch(outData);
                memcpy(outputBuffer, outData.data(), sizeof(int16_t) * outData.size());
            }

            volumeReporter_(volumeMeterOut_.calculate(outData));
        });
        if (!ret) {
            return ret;
        }

        pClient = std::make_shared<TcpClient>();

        volumeReporter_ = reportVolume;
    }
    catch (const std::exception &e) {
        LOGE_STD_EXCEPTION(e);
        return e.what();
    }

    return 0;
}

void Worker::setDurationReporter(decltype(durationReporter_) __)
{
    durationReporter_ = __;
}

void Worker::setMute(bool mute)
{
    mute_ = mute;
}

bool Worker::initCodec(){
    encoder = &(AudioEncoder::create());

    if (encoder->reInit()){
        return true;
    }
    return false;
}

void Worker::asyncStart(std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState) {
    netThread_ = std::make_shared<std::thread>(std::bind(&Worker::syncStart, this, toggleState));
}


ReturnType Worker::syncStart(std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState) {
    try{
        gotoStop_ = false;
        bool isLogin = false;
                
        if (durationReporter_){
            //start Timer
            durationTimer_ = std::make_shared<std::thread>([this]() {
                for (uint32_t s = 0u;!gotoStop_; s++) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    durationReporter_(s);
                }
            });
        }
        
        std::string host = configRoot_.get<std::string>("server.host", "127.0.0.1");
        std::string port = configRoot_.get<std::string>("server.port", "80");

        pClient->init(
                    host.c_str(),
                    port.c_str(),
            [&](const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) {
            // on Received Data
            switch (netPacket->payloadType()){
            case NetPacket::PayloadType::HeartBeatRequest:{
                myClient.send(NetPacket(NetPacket::PayloadType::HeartBeatResponse));
                break;
            }
            case NetPacket::PayloadType::LoginResponse: {
                isLogin = true;
                break;
            }
            case NetPacket::PayloadType::AudioMessage: {
                if (downStreamProcessor_) {
                    downStreamProcessor_->append(netPacket);
                }
                break;
            }
            default:
                break;
            }
        });


        /// TCP/UDP connecting phase
        /// timeout: 100ms
        {
            //
            bool isOK = false;
            for (int i = 0; i < 10; i++){
                if (pClient->isConnected()){
                    isOK = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (!isOK) {
                toggleState(NetworkState::Disconnected, "Could not connect to Server...");
                if (durationReporter_) {
                    durationReporter_(0);
                }
                return "Could not connect to Server...";
            }
        }


        /// App Login phase
        /// timeout: 300ms
        {
            pClient->send(NetPacket(NetPacket::PayloadType::LoginRequest));
            for (int i = 0; i < 10; i++){
                if (isLogin){
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }

        if (!isLogin){
            toggleState(NetworkState::Disconnected, "Could not Login to Server...");
            return "Could not Login to Server...";
        }

        toggleState(NetworkState::Connected, "");

        endpoint_->asyncStart();
        

        // sending work
        for (;!gotoStop_;){
            // send heartbeat
            sendHeartbeat();
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }


        /// App logout
        pClient->send(NetPacket(NetPacket::PayloadType::LogoutRequest));


    }
    catch (std::exception& e){
        return e.what();
    }

    return 0;
}

void Worker::nsAecVolumeVadSend(const short *buffer){
    if (mute_) {
        /// nothing to do, just return!
        return; 
    }

    std::vector<short> denoisedBuffer(blockSize);
    std::vector<short> tobeSend(blockSize);
    std::vector<float> out(blockSize);


    {
        auto outTemp = denoisedBuffer.data();
        WebRtcNsx_Process(ns_, &buffer, 1, &outTemp);
    }



    if (needAec_) {
        std::vector<float> floatNearend(blockSize);
        for (int i = 0; i < blockSize; i++) {
            floatNearend[i] = (float)denoisedBuffer[i] / (1 << 15);
        }
        for (int i = 0; i < blockSize; i += 160) {
            auto temp = &(floatNearend[i]);
            auto temp2 = &(out[i]);

            auto ret =
                WebRtcAec_Process(aec,
                    &temp,
                    sampleRate / 16000,
                    &temp2,
                    blockSize,
                    blockSize,
                    0);
            if (ret) {
                throw;
            }

            for (int j = i; j < i + blockSize; j++) {
                tobeSend[j] = (short)(out[j] * (1 << 15));
            }
        }
    }

    {
        PcmSegment seg;
        memcpy(seg.data(), denoisedBuffer.data(), denoisedBuffer.size() * sizeof(int16_t));

        volumeReporter_(volumeMeterIn_.calculate(seg));
    }

    auto haveVoice = (1 == WebRtcVad_Process(vad, sampleRate, denoisedBuffer.data(), sampleRate / 100));
    if (vadReporter_) {
        vadReporter_(haveVoice);
    }


    if (haveVoice) {
        vadCounter_++;
        needSend_ = true;
    }


    {
        static auto lastTimeStamp = std::chrono::system_clock::now();
        auto now = std::chrono::system_clock::now();
        auto elapsed = now - lastTimeStamp;

        if (elapsed > std::chrono::seconds(1)) {
            if (vadCounter_ == 0) {
                /// 100 section per one second
                /// in last one second, no voice found
                /// so that we predict there's no voice in future
                needSend_ = false;
            }
            lastTimeStamp = std::chrono::system_clock::now();

            vadCounter_ = 0;
        }
    }

    if (needSend_) {
        std::vector<char> outData;
        auto retEncode = encoder->encode(needAec_ ? tobeSend : denoisedBuffer, outData);
        if (!retEncode) {
            std::cout << retEncode.message() << std::endl;
            //break;
            throw;
        }

        pClient->send(NetPacket(NetPacket::PayloadType::AudioMessage, ++sn_sendingAudio_, outData));
    }
}


void Worker::sendHeartbeat(){
    static auto lastTimeStamp = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    auto elapsed = now - lastTimeStamp;
    if (elapsed > std::chrono::seconds(10)) {
        pClient->send(NetPacket(NetPacket::PayloadType::HeartBeatRequest, ++sn_sendingHeartBeat_));
        lastTimeStamp = std::chrono::system_clock::now();
    }
}


void Worker::syncStop()
{
    gotoStop_ = true;
    
    if (endpoint_) {
        endpoint_->syncStop();
        endpoint_ = nullptr;
    }

    if (netThread_) {
        if (netThread_->joinable()) {
            netThread_->join();
        }
        netThread_ = nullptr;
    }



    if (durationTimer_) {
        if (durationTimer_->joinable()) {
            durationTimer_->join();
        }
        durationTimer_ = nullptr;
    }

}



std::shared_ptr<IWorker> IWorker::create() {
    return std::make_shared<Worker>();
}