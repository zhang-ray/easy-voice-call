#pragma once
#include "../NetClient.hpp"



class KcpClientImpl;

class KcpClient : public NetClient {
private:
    std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived_ = nullptr;
    std::shared_ptr<KcpClientImpl> impl_ = nullptr;
public:
    virtual ReturnType init(
        const std::string &serverHost, 
        const std::string &serverPort,
        std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived
    ) override;


    virtual ReturnType send(const NetPacket &netPacket) const override;


    virtual bool isConnected() const override;

};