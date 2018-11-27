#include "KcpClient.hpp"


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "Logger.hpp"
#include "ikcp.h"



using boost::asio::ip::udp;

enum { max_length = 1024 };

namespace {
    int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);
}
class KcpClientImpl {
private:
    boost::asio::io_context io_context;
    udp::socket s;
    udp::resolver resolver;
    udp::resolver::results_type endpoints;
    ikcpcb *kcp_ = nullptr;
    bool isGoingOn_ = true;
    std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived_ = nullptr;
    std::vector<char> inputBuffer_;
    std::shared_ptr<NetClient> tcpClient_ = nullptr;
    std::shared_ptr<std::thread> kcpUpdater_ = nullptr;
public:
    KcpClientImpl(
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
        ,endpoints(resolver.resolve(udp::v4(), serverHost, serverPort))
    {
        kcp_ = ikcp_create(0x11223344, this);
        kcp_->output = ::udp_output;
        std::make_shared<std::thread>([this]() {
            for (; isGoingOn_;) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                ikcp_update(kcp_, ProcessTime::get().getProcessUptime());
            }
        })->detach();
        ikcp_nodelay(kcp_, 1, 1, 0, 1);
        //ikcp_wndsize(kcp_, 128, 128);


        kcpUpdater_ = std::make_shared<std::thread>([this]() {
            udp::endpoint sender_endpoint;
            char reply[max_length];

            try{
                for (; isGoingOn_;) {
                    size_t reply_length = s.receive_from(boost::asio::buffer(reply, max_length), sender_endpoint);
                    ikcp_input(kcp_, reply, reply_length);
                    for (; isGoingOn_;) {
                        char kcp_buf[NetPacket::FixHeaderLength + NetPacket::MaxPayloadLength] = "";
                        int kcp_recvd_bytes = ikcp_recv(kcp_, kcp_buf, NetPacket::FixHeaderLength + NetPacket::MaxPayloadLength);
                        if (kcp_recvd_bytes <= 0){
                            break;
                        }

                        auto oldSize = inputBuffer_.size();
                        inputBuffer_.resize(oldSize + kcp_recvd_bytes);
                        std::memcpy(inputBuffer_.data() + oldSize, kcp_buf, kcp_recvd_bytes);

                        auto newPacket = makePacket(inputBuffer_.data(), inputBuffer_.size());
                        if (newPacket) {
                            // digest inputBuffer_
                            {
                                auto oldSize = inputBuffer_.size();
                                auto newSize = oldSize - newPacket->wholePackLength();
                                std::memcpy(inputBuffer_.data(), inputBuffer_.data() + newPacket->wholePackLength(), newSize);
                                inputBuffer_.resize(newSize);
                            }
                            // handle payload
                            onDataReceived_(*tcpClient_, newPacket);
                        }
                    }
                }
            }
            catch (const std::exception &e){
                LOGE_STD_EXCEPTION(e);
            }

        });
    }


    ~KcpClientImpl() {
        isGoingOn_ = false;
        if (kcpUpdater_) {
            kcpUpdater_->join();
        }
        ikcp_release(kcp_);
    }

    ReturnType send(const NetPacket &netPacket) {
        auto ret= ikcp_send(kcp_, netPacket.header(), netPacket.wholePackLength());
        if (ret < 0) {
            return "ret < 0";
        }
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



namespace {
    int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
        if (!user) {
            return -1;
        }

        auto pKcpConnection = (KcpClientImpl*)(user);
        pKcpConnection->sendUdp(buf, len);
        return 0;
    }
}

ReturnType KcpClient::init(
    const std::string &serverHost,
    const std::string &serverPort,
    std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived
) {
    impl_ = std::make_shared<KcpClientImpl>(shared_from_this(), serverHost, serverPort, onDataReceived);
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
