#include "KcpClient.hpp"



ReturnType KcpClient::init(
    const std::string &serverHost, 
    const std::string &serverPort,
    std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived
){

    onDataReceived_ = onDataReceived;
}

ReturnType KcpClient::send(const NetPacket &netPacket) const
{
    throw std::logic_error("The method or operation is not implemented.");
}

bool KcpClient::isConnected() const
{
    throw std::logic_error("The method or operation is not implemented.");
}
