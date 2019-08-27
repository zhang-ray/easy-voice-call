#include "RawUdpClient.hpp"


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "Logger.hpp"





using boost::asio::ip::udp;

enum { max_length = 1024 };


class RawUdpClientImpl {
private:
    boost::asio::io_context io_context;
    udp::socket s;
    udp::resolver resolver;
    udp::resolver::results_type endpoints;
    bool isGoingOn_ = true;
    std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived_ = nullptr;
    std::shared_ptr<NetClient> tcpClient_ = nullptr;
    std::shared_ptr<std::thread> receiver_ = nullptr;
public:
    RawUdpClientImpl(
        std::shared_ptr<NetClient> pTcpClient,
        const std::string &serverHost,
        const std::string &serverPort,
        std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived
    )
        : tcpClient_(pTcpClient)
        , onDataReceived_(onDataReceived)
        , io_context()
        , s(io_context, udp::endpoint(udp::v4(), 0))
        , resolver(io_context)
        , endpoints(resolver.resolve(udp::v4(), serverHost, serverPort))
    {
        receiver_ = std::make_shared<std::thread>([this]() {
            udp::endpoint sender_endpoint;
            char reply[max_length];

            try {
                for (; isGoingOn_;) {
                    size_t reply_length = s.receive_from(boost::asio::buffer(reply, max_length), sender_endpoint);
                    auto newPacket = makePacket(reply, reply_length);
                    if (newPacket) {
                        // handle payload
                        onDataReceived_(*tcpClient_, newPacket);
                    }
                }
            }
            catch (const std::exception &e) {
                LOGE_STD_EXCEPTION(e);
            }

        });
    }




    ~RawUdpClientImpl() {
        isGoingOn_ = false;
        if (receiver_) {
            receiver_->join();
        }
    }

    ReturnType send(const NetPacket &netPacket) {
        sendUdp(netPacket.header(), netPacket.wholePackLength());
        return 0;
    }


    ReturnType sendUdp(const char *buf, int len){
        try {
            s.send_to(boost::asio::buffer(buf, len), *endpoints.begin());
        }
        catch (std::exception &e) {
            LOGE_STD_EXCEPTION(e);
            return e.what();
        }
        return 0;
    }



    std::shared_ptr<NetPacket> makePacket(const char *data, std::size_t bytes_recvd)
    {
        if (bytes_recvd < NetPacket::FixHeaderLength) {
            return nullptr;
        }

        auto ret = std::make_shared<NetPacket>();
        std::memcpy(ret->header(), data, NetPacket::FixHeaderLength);
        if (!ret->checkHeader()) {
            return nullptr;
        }


        if (bytes_recvd < NetPacket::FixHeaderLength + ret->payloadLength()) {
            return nullptr;
        }

        std::memcpy(ret->payload(), data + NetPacket::FixHeaderLength, ret->payloadLength());
        return ret;
    }


};


ReturnType RawUdpClient::init(
    const std::string &serverHost,
    const std::string &serverPort,
    std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived
) {
    impl_ = std::make_shared<RawUdpClientImpl>(shared_from_this(), serverHost, serverPort, onDataReceived);
    return true;
}

ReturnType RawUdpClient::send(const NetPacket &netPacket) const
{
    return impl_->send(netPacket);
    return 0;
}

bool RawUdpClient::isConnected() const
{
    // UDP is connectionless...
    return true;
}
