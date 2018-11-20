#pragma once
#include "NetClient.hpp"

#error DEPRECATED

class NetClientStub_EchoMediaData : public NetClient , public Singleton<NetClientStub_EchoMediaData>{
private:
    std::function<void(const NetClient & myClient, const NetPacket & netPacket) > onDataReceived_ = nullptr;
public:
    NetClientStub_EchoMediaData(){}

    virtual ReturnType init(const std::string &serverHost, const std::string &serverPort, std::function<void(const NetClient & myClient, const NetPacket & netPacket) > onDataReceived) override
    {
        onDataReceived_ = onDataReceived;
        return 0;
    }


    virtual ReturnType send(const NetPacket &netPacket) const override
    {
        static uint32_t counterUserMessage_ = 0;
        auto payloadType = netPacket.payloadType();

        bool traceDump = true;
        switch (payloadType) {
        case NetPacket::PayloadType::HeartBeatRequest: {
            onDataReceived_(*this, NetPacket(NetPacket::PayloadType::HeartBeatResponse));
            break;
        }
        case NetPacket::PayloadType::TextMessage:
        case NetPacket::PayloadType::AudioMessage:
        {
            onDataReceived_(*this, netPacket);
            break;
        }
        case NetPacket::PayloadType::LoginRequest:
        {
            onDataReceived_(*this, NetPacket(NetPacket::PayloadType::LoginResponse));
            break;
        }
        case NetPacket::PayloadType::LogoutRequest:
        {
            onDataReceived_(*this, NetPacket(NetPacket::PayloadType::LogoutResponse));
            break;
        }

        default:
            // WHAT?
            break;
        }

        return 0;
    }

    virtual bool isConnected() const override { return true; }
};