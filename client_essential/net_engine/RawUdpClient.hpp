#pragma once
#include "../NetClient.hpp"

class RawUdpClientImpl;

class RawUdpClient : public NetClient, public std::enable_shared_from_this<RawUdpClient>{
private:
    std::shared_ptr<RawUdpClientImpl> impl_ = nullptr;
public:
    virtual ReturnType init(
        const std::string &serverHost, 
        const std::string &serverPort,
        std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived
    ) override;


    virtual ReturnType send(const NetPacket &netPacket) const override;


    virtual bool isConnected() const override;

};