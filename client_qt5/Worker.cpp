#include "Worker.hpp"


#include "evc/Factory.hpp"
#include "evc/TcpClient.hpp"

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
                        std::function<void(const double)> reportMicVolume,
                        std::function<void(const double)> reportSpkVolume){
    device_ = &(Factory::get().create());

    std::string micInfo;
    std::string spkInfo;
    if (device_->init(micInfo, spkInfo)){
        reportInfo(micInfo, spkInfo);
        micVolumeReporter_ = reportMicVolume;
        spkVolumeReporter_ = reportSpkVolume;
        return true;
    }
    return false;
}

void Worker::asyncStart(const std::string &host, const std::string &port, std::function<void (const NetworkState &, const std::string &)> toggleState){
    syncStop();
    netThread_.reset(new std::thread(std::bind(&Worker::syncStart, this, host, port, toggleState)));
}

void Worker::syncStart(const std::string &host,
                        const std::string &port,
                        std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState
                          ){


    try{
        gotoStop_ = false;
        TcpClient client(
                    host.c_str(),
                    port.c_str(),
                    [&](const NetPacket& netPacket){
            // on Received Data
            if (netPacket.payloadType()==NetPacket::PayloadType::HeartBeatRequest){
                // MSVC could not capture `client` reference...
                // client.send(NetPacket(NetPacket::PayloadType::HeartBeatResponse));
            }
            else if (netPacket.payloadType()==NetPacket::PayloadType::HeartBeatResponse){
                //throw;
            }
            else if (netPacket.payloadType()==NetPacket::PayloadType::AudioMessage){
                std::vector<char> netBuff;
                netBuff.resize(netPacket.payloadLength());
                memcpy(netBuff.data(), netPacket.payload(), netPacket.payloadLength());
                std::vector<short> decodedPcm;
                decoder->decode(netBuff, decodedPcm);
                auto ret = device_->write(decodedPcm);
                if (!ret) {
                    std::cout << ret.message() << std::endl;
                }
            }
        });


        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (!client.isConnected()){
            toggleState(NetworkState::Disconnected, "Could not connect to Server...");
            return;
        }


        toggleState(NetworkState::Connected, "");

        // sending work
        for (;!gotoStop_;){
            const auto blockSize = 1920;
            std::vector<short> micBuffer(blockSize);
            auto ret = device_->read(micBuffer);
            if (!ret){
                break;
            }

            // calculate mic Volume
            {
                // mock:
                micVolumeReporter_((double)micBuffer[0]/(1<<15));
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
    }
    catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }

    //        emit updateUiState(NetworkState::Disconnected);

    /* ... here is the expensive or blocking operation ... */
    //        emit resultReady(result);

}
