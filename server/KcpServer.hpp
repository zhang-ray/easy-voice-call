#pragma once


#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include "Logger.hpp"
#include "NetPacket.hpp"
#include "ikcp.h"
#include <thread>
#include "Server.hpp"

class KcpRoom;
class KcpConnection;

class KcpPriv{
    /// use class instead of namespace for "friend class"
public:
    static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);
    static void processClientMessagePayload(const NetPacket& msg, std::shared_ptr<KcpConnection> sender, KcpRoom &room);
};

class KcpConnection : public std::enable_shared_from_this<KcpConnection> {
    friend class KcpRoom;
private:
    KcpRoom &room_;
    boost::asio::ip::udp::socket &socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    ikcpcb *kcp_ = nullptr;
    std::shared_ptr<NetPacket> makePacket(const char *data, std::size_t bytes_recvd);
    bool isGoingOn_ = true;
    std::vector<char> inputBuffer_;
    std::shared_ptr<std::thread> kcpUpdater_ = nullptr;
public:
    KcpConnection(boost::asio::ip::udp::socket &socket, boost::asio::ip::udp::endpoint remote_endpoint, KcpRoom &room)
        : remote_endpoint_(remote_endpoint)
        , room_(room)
        , socket_(socket)
    {
        kcp_ = ikcp_create(0x11223344, this);
        kcp_->output = KcpPriv::udp_output;
        kcpUpdater_ = std::make_shared<std::thread>([this]() {
            try{
                for (; isGoingOn_;) {
                    ikcp_update(kcp_, ProcessTime::get().getProcessUptime());
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            }
            catch (const std::exception &e) {
                LOGE_STD_EXCEPTION(e);
            }
        });
        ikcp_nodelay(kcp_, 1, 1, 0, 1);
        //ikcp_wndsize(kcp_, 128, 128);
    }
    ~KcpConnection() {
        isGoingOn_ = false;
        if (kcpUpdater_) {
            kcpUpdater_->join();
        }
        ikcp_release(kcp_);
    }
    bool onReceived(const char *data, std::size_t bytes_recvd);
    void send(const NetPacket& msg);
    void sendUdp(const char *buf, int len);
};


class KcpRoom {
    friend class KcpConnection;
    friend class KcpPriv;
private:
    std::set<std::shared_ptr<KcpConnection>> peers_;
    std::shared_ptr<KcpConnection> findPeer(boost::asio::ip::udp::endpoint endpoint);
public:
    void insert(boost::asio::ip::udp::socket &socket, boost::asio::ip::udp::endpoint endpoint, const char *data, std::size_t bytes_recvd);
    void kickMeOut(std::shared_ptr<KcpConnection> me);
};


class KcpServer : public Server {
private:

public:
    KcpServer(boost::asio::io_service& io_context, short port);
    void doReceive();
private:
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    enum { max_length = 1<<10 };
    char data_[max_length];
    KcpRoom room_;
};