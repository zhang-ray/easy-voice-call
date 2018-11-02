#include "evc/Worker.hpp"


#include "evc/Factory.hpp"
#include "evc/TcpClient.hpp"
#include "evc/RingBuffer.hpp"
#include <mutex> // for std::once_flag

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

//const char *constPort ="80";

Worker::Worker(bool needAec)
    :needAec_(needAec)
    , s2cPcmBuffer_(sizeof(short)*160,100)
{
    if (needAec_){
        aec = WebRtcAec_Create();
        if (auto ret = WebRtcAec_Init(aec, sampleRate, sampleRate)){
            throw "WebRtcAec_Init failed";
        }
    }



    {
        vad = WebRtcVad_Create();
        if (WebRtcVad_Init(vad)){
            std::cerr << "WebRtcVad_Init failed" << std::endl;
            throw;
        }

        if (WebRtcVad_set_mode(vad, 3)){
            std::cerr << "WebRtcVad_set_mode failed" << std::endl;
            throw;
        }
    }

    {
        ns_ = WebRtcNsx_Create();
        if (WebRtcNsx_Init(ns_, sampleRate)){
            throw;
        }

        if (WebRtcNsx_set_policy(ns_, 3)){
            throw;
        }
    }
}


Worker::~Worker(){
    try{
        syncStop();
    }
    catch(std::exception &e){
        //qDebug() << e.what();
    }
}

bool Worker::initCodec(){
    decoder = &(Factory::get().createAudioDecoder());
    encoder = &(Factory::get().createAudioEncoder());

    if (encoder->reInit()){
        if (decoder->reInit()) {
            return true;
        }
    }
    return false;
}

bool Worker::initDevice(std::function<void(const std::string &, const std::string &)> reportInfo,
                        std::function<void(const AudioIoVolume)> reportVolume,
                        std::function<void(const bool)> vadReporter){
    device_ = &(Factory::get().create());

    std::string micInfo;
    std::string spkInfo;
    if (device_->init(micInfo, spkInfo)){
        reportInfo(micInfo, spkInfo);
        volumeReporter_ = reportVolume;
        vadReporter_ = vadReporter;

        /// TODO: LPC

        playbackThread_ = std::make_shared<std::thread>([this](){
            std::vector<short> pcmLPC(s2cPcmBuffer_.bytePerElement(), 0);
            std::vector<short> tmp(s2cPcmBuffer_.bytePerElement());

            for (;!gotoStop_;){
                auto pcm = &pcmLPC;
                if (s2cPcmBuffer_.popElements((uint8_t*)tmp.data(), 1)){
                    pcm = &tmp;
                }

                device_->write(*pcm);

                if (volumeReporter_){
                    auto currentLevel = sav.calculate(*pcm, AudioIoVolume::MAX_VOLUME_LEVEL);
                    static auto recentMaxLevel = currentLevel;

                    static auto lastTimeStamp = std::chrono::system_clock::now();
                    auto now = std::chrono::system_clock::now();
                    auto elapsed = now - lastTimeStamp;

                    // hold on 1s
                    if (elapsed > std::chrono::seconds(1)){
                        recentMaxLevel=0;
                        lastTimeStamp = std::chrono::system_clock::now();
                    }


                    if (currentLevel>recentMaxLevel){
                        recentMaxLevel=currentLevel;
                        // re calculate hold-on time
                        lastTimeStamp = std::chrono::system_clock::now();
                    }

                    volumeReporter_({AudioInOut::Out, currentLevel, recentMaxLevel});
                }
            }
        });

        return true;
    }
    return false;
}

void Worker::asyncStart(const std::string &host,const std::string &port, std::function<void (const NetworkState &, const std::string &)> toggleState){
    syncStop();
    netThread_.reset(new std::thread(std::bind(&Worker::syncStart, this, host, port, toggleState)));
}


void Worker::syncStart(const std::string &host,const std::string &port,
                       std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                       ){

    try{
        gotoStop_ = false;
        bool isLogin = false;
        TcpClient client(
                    host.c_str(),
                    port.c_str(),
                    [&](TcpClient *_TcpClient, const NetPacket& netPacket){
            // on Received Data
            switch (netPacket.payloadType()){
            case NetPacket::PayloadType::HeartBeatRequest:{
                _TcpClient->send(NetPacket(NetPacket::PayloadType::HeartBeatResponse));
                break;
            }
            case NetPacket::PayloadType::LoginResponse: {
                isLogin = true;
                //qDebug() << "&isLogin=" << &isLogin << "\t" << __FUNCTION__;
                break;
            }
            case NetPacket::PayloadType::AudioMessage: {
                std::vector<char> netBuff;
                netBuff.resize(netPacket.payloadLength());
                memcpy(netBuff.data(), netPacket.payload(), netPacket.payloadLength());
                std::vector<short> decodedPcm;
                decoder->decode(netBuff, decodedPcm);

                if (needAec_) {
                    std::vector<float> floatFarend(decodedPcm.size());
                    for (int i=0;i<decodedPcm.size(); i++){
                        floatFarend[i] = decodedPcm[i]/(1<<15);
                    }
                    for (int i = 0; i < blockSize; i+=160){
                        auto ret = WebRtcAec_BufferFarend(aec, &(floatFarend[i]), 160);
                        if (ret){
                            throw;
                        }
                    }
                }

                auto bRet = s2cPcmBuffer_.pushElements((uint8_t*)decodedPcm.data(), 1);
                if (!bRet){
                    //qDebug() << "pushElements failed!";
                }

                break;
            }
            }
        });


        /// TCP/UDP connecting phase
        /// timeout: 100ms
        {
            //
            bool isOK = false;
            for (int i = 0; i < 10; i++){
                if (client.isConnected()){
                    isOK = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (!isOK) {
                toggleState(NetworkState::Disconnected, "Could not connect to Server...");
                return;
            }
        }


        /// App Login phase
        /// timeout: 300ms
        {
            client.send(NetPacket(NetPacket::PayloadType::LoginRequest));
            for (int i = 0; i < 10; i++){
                if (isLogin){
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }

        // qDebug() << "&isLogin=" << &isLogin << "\t" << __FUNCTION__;
        if (!isLogin){
            toggleState(NetworkState::Disconnected, "Could not Login to Server...");
            return;
        }

        toggleState(NetworkState::Connected, "");

        // sending work
        for (;!gotoStop_;){
            std::vector<short> denoisedBuffer(blockSize);
            std::vector<short> tobeSend(blockSize);
            std::vector<float> out(blockSize);


            /// TODO:
            /// in practice, device_->read would be unblock in first several blocks
            /// so let's clear microphone's buffer on first time???
            if (!mute_){
                {
                    std::vector<short> micBuffer(blockSize);
                    auto ret = device_->read(micBuffer);
                    if (!ret){
                        break;
                    }
                    auto temp = (short*)micBuffer.data();
                    auto outTemp = denoisedBuffer.data();
                    WebRtcNsx_Process(ns_, &temp, 1, &outTemp);
                }



                if (needAec_){
                    std::vector<float> floatNearend(blockSize);
                    for (int i=0;i<blockSize; i++){
                        floatNearend[i] = (float)denoisedBuffer[i]/(1<<15);
                    }
                    for (int i = 0; i < blockSize; i+=160){
                        auto temp = &(floatNearend[i]);
                        auto temp2 = &(out[i]);

                        auto ret =
                                WebRtcAec_Process(aec,
                                                  &temp,
                                                  sampleRate / 16000,
                                                  &temp2,
                                                  blockSize,
                                                  blockSize,
                                                  0 );
                        if (ret){
                            throw;
                        }

                        for (int j = i; j< i+blockSize; j++){
                            tobeSend[j]=out[j]*(1<<15);
                        }
                    }
                }



                if (volumeReporter_){
                    auto currentLevel = sav.calculate(denoisedBuffer, AudioIoVolume::MAX_VOLUME_LEVEL);
                    static auto recentMaxLevel = currentLevel;

                    static auto lastTimeStamp = std::chrono::system_clock::now();
                    auto now = std::chrono::system_clock::now();
                    auto elapsed = now - lastTimeStamp;

                    // hold on 1s
                    if (elapsed > std::chrono::seconds(1)){
                        recentMaxLevel=0;
                        lastTimeStamp = std::chrono::system_clock::now();
                    }


                    if (currentLevel>recentMaxLevel){
                        recentMaxLevel=currentLevel;
                        // re calculate hold-on time
                        lastTimeStamp = std::chrono::system_clock::now();
                    }

                    volumeReporter_({AudioInOut::In, currentLevel, recentMaxLevel});
                }


                auto haveVoice = (1==WebRtcVad_Process(vad, sampleRate, denoisedBuffer.data(), sampleRate/100));
                if (vadReporter_){
                    vadReporter_(haveVoice);
                }


                if (haveVoice){
                    vadCounter_++;
                    needSend_=true;
                }


                {
                    static auto lastTimeStamp = std::chrono::system_clock::now();
                    auto now = std::chrono::system_clock::now();
                    auto elapsed = now - lastTimeStamp;

                    if (elapsed > std::chrono::seconds(1)){
                        if (vadCounter_==0){
                            /// 100 section per one second
                            /// in last one second, no voice found
                            /// so that we predict there's no voice in future
                            needSend_ = false;
                        }
                        lastTimeStamp = std::chrono::system_clock::now();

                        vadCounter_=0;
                    }
                }


                if (needSend_){
                    std::vector<char> outData;
                    auto retEncode = encoder->encode(needAec_? tobeSend: denoisedBuffer, outData);
                    if (!retEncode){
                        std::cout << retEncode.message() << std::endl;
                        break;
                    }

                    client.send(NetPacket(NetPacket::PayloadType::AudioMessage, outData));
                }
            }


            // send heartbeat
            {
                static auto lastTimeStamp = std::chrono::system_clock::now();
                auto now = std::chrono::system_clock::now();
                auto elapsed = now - lastTimeStamp;
                if (elapsed > std::chrono::seconds(10)){
                    client.send(NetPacket(NetPacket::PayloadType::HeartBeatRequest));
                    lastTimeStamp = std::chrono::system_clock::now();
                }
            }
        }


        /// App logout
        client.send(NetPacket(NetPacket::PayloadType::LogoutRequest));


    }
    catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }

    //        emit updateUiState(NetworkState::Disconnected);

    /* ... here is the expensive or blocking operation ... */
    //        emit resultReady(result);

}
