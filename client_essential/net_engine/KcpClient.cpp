#include "KcpClient.hpp"


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "Logger.hpp"

using boost::asio::ip::udp;

enum { max_length = 1024 };

//*

class KcpClientImpl {
private:
    boost::asio::io_context io_context;
    udp::socket s;
    udp::resolver resolver;
    udp::resolver::results_type endpoints;
public:
    KcpClientImpl(
        const std::string &serverHost,
        const std::string &serverPort
    )
        : io_context()
        , s(io_context, udp::endpoint(udp::v4(), 0))
        , resolver(io_context)
        ,endpoints(resolver.resolve(udp::v4(), serverHost, serverPort))
    {}

    ReturnType send(const NetPacket &netPacket) {
        try {
            s.send_to(boost::asio::buffer(netPacket.header(), netPacket.wholePackLength()), *endpoints.begin());
        }
        catch (std::exception &e) {
            LOGE_STD_EXCEPTION(e);
            return e.what();
        }
        return 0;
    }
};
//*/

ReturnType KcpClient::init(
    const std::string &serverHost,
    const std::string &serverPort,
    std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived
) {
    impl_ = std::make_shared<KcpClientImpl>(serverHost, serverPort);
    onDataReceived_ = onDataReceived;
    return true;
}

ReturnType KcpClient::send(const NetPacket &netPacket) const
{
    return impl_->send(netPacket);
    return 0;
}

bool KcpClient::isConnected() const
{
    // UDP is connectionless...
    return true;
}
