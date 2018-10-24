#include "Worker.hpp"


#include "evc/Factory.hpp"
#include "evc/TcpClient.hpp"
#include "evc/AudioVolume.hpp"
#include <mutex> // for std::once_flag

//// TODO
//// don't include WEBRTC directly...
#include "../audio-processing-module/independent_vad/src/webrtc_vad.hpp"
#include "../audio-processing-module/independent_aec/src/echo_cancellation.h"

namespace {
VadInst* vad;
std::once_flag once_vad;
void *aec = nullptr;
}

using namespace webrtc;

const char *constPort ="1222";

Worker::Worker(){
    aec = WebRtcAec_Create();
    if (auto ret = WebRtcAec_Init(aec, 48000, 48000)){
        throw "WebRtcAec_Init failed";
    }
}

Worker::~Worker(){
    try{
        syncStop();
    }
    catch(std::exception &e){
        qDebug() << e.what();
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
                        std::function<void(const uint8_t)> reportMicVolume,
                        std::function<void(const uint8_t)> reportSpkVolume,
                        std::function<void(const bool)> vadReporter){
    device_ = &(Factory::get().create());

    std::string micInfo;
    std::string spkInfo;
    if (device_->init(micInfo, spkInfo)){
        reportInfo(micInfo, spkInfo);
        micVolumeReporter_ = reportMicVolume;
        spkVolumeReporter_ = reportSpkVolume;
        vadReporter_ = vadReporter;
        return true;
    }
    return false;
}

void Worker::asyncStart(const std::string &host, std::function<void (const NetworkState &, const std::string &)> toggleState){
    syncStop();
    netThread_.reset(new std::thread(std::bind(&Worker::syncStart, this, host, toggleState)));
}

void Worker::syncStart(const std::string &host,
                        std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                          ){


    try{
        gotoStop_ = false;
        bool isLogin = false;
        TcpClient client(
                    host.c_str(),
                    constPort,
                    [&](TcpClient *_TcpClient, const NetPacket& netPacket){
            // on Received Data
            switch (netPacket.payloadType()){
                case NetPacket::PayloadType::HeartBeatRequest:{
                    _TcpClient->send(NetPacket(NetPacket::PayloadType::HeartBeatResponse));
                    break;
                }
                case NetPacket::PayloadType::LoginResponse: {
                    isLogin = true;
                    qDebug() << "&isLogin=" << &isLogin << "\t" << __FUNCTION__;
                    break;
                }
                case NetPacket::PayloadType::AudioMessage: {
                    std::vector<char> netBuff;
                    netBuff.resize(netPacket.payloadLength());
                    memcpy(netBuff.data(), netPacket.payload(), netPacket.payloadLength());
                    std::vector<short> decodedPcm;
                    decoder->decode(netBuff, decodedPcm);

                    if (false/*TODO*/) {
                        std::vector<float> floatFarend(decodedPcm.size());
                        for (int i=0;i<decodedPcm.size(); i++){
                            floatFarend[i] = decodedPcm[i]/(1<<15);
                        }
                        for (int i = 0; i < 1920; i+=160){
                            auto ret = WebRtcAec_BufferFarend(aec, &(floatFarend[i]), 160);
                            if (ret){
                                throw;
                            }
                        }
                    }
                    auto ret = device_->write(decodedPcm);
                    if (spkVolumeReporter_){
                        static SuckAudioVolume sav;
                        spkVolumeReporter_(sav.calculate(decodedPcm));
                    }
                    if (!ret) {
                        std::cout << ret.message() << std::endl;
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

        qDebug() << "&isLogin=" << &isLogin << "\t" << __FUNCTION__;
        if (!isLogin){
            toggleState(NetworkState::Disconnected, "Could not Login to Server...");
            return;
        }

        toggleState(NetworkState::Connected, "");

        // sending work
        for (;!gotoStop_;){
            const auto blockSize = 1920;
            std::vector<short> micBuffer(blockSize);
            /// TODO:
            /// in practice, device_->read would be unblock in first several blocks
            /// so let's clear microphone's buffer on first time???
            auto ret = device_->read(micBuffer);
            if (!ret){
                break;
            }

            if (false/*TODO*/) {
                std::vector<float> floatNearend(micBuffer.size());
                for (int i=0;i<micBuffer.size(); i++){
                    floatNearend[i] = micBuffer[i]/(1<<15);
                }
                std::vector<float> out(micBuffer.size());
                for (int i = 0; i < 1920; i+=160){
                    auto temp = &(floatNearend[i]);
                    auto temp2 = &(out[i]);

                    auto ret =
                            WebRtcAec_Process(aec,

                                              ///TODO:
                                              /// split_bands_const_f
                                              &temp,

                                              48000 / 16000
                                              , &temp2, 160, 48000, 0 );
                    if (ret){
                        throw;
                    }
                }
            }
            if (micVolumeReporter_){
                static SuckAudioVolume sav;
                micVolumeReporter_(sav.calculate(micBuffer));
            }
            if (vadReporter_){

                static int once = [&](){
                    vad = WebRtcVad_Create();
                    if (WebRtcVad_Init(vad)){
                        std::cerr << "WebRtcVad_Init failed" << std::endl;
                        return -1;
                    }

                    if (WebRtcVad_set_mode(vad, 3)){
                        std::cerr << "WebRtcVad_set_mode failed" << std::endl;
                        return -1;
                    }
                    return 0;
                } ();
                if (!once){
                    for (int i = 0 ; i < 4; i++){
                        vadReporter_(1==WebRtcVad_Process(vad, 48000, &(micBuffer[i*480]), 480));
                    }
                }
            }

            std::vector<char> outData;
            auto retEncode = encoder->encode(micBuffer, outData);
            if (!retEncode){
                std::cout << retEncode.message() << std::endl;
                break;
            }

            client.send(NetPacket(NetPacket::PayloadType::AudioMessage, outData));



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
