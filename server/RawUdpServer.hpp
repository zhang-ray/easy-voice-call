#pragma once


#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include "Logger.hpp"
#include "NetPacket.hpp"
#include <thread>
#include "Server.hpp"


class RawUdpRoom;
class RawUdpConnection;

class RawUdpPriv{
    /// use class instead of namespace for "friend class"
public:
    static void processClientMessagePayload(const NetPacket& msg, std::shared_ptr<RawUdpConnection> sender, RawUdpRoom &room);
};

class RawUdpConnection : public std::enable_shared_from_this<RawUdpConnection> {
    friend class RawUdpRoom;
private:
    RawUdpRoom &room_;
    boost::asio::ip::udp::socket &socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    std::shared_ptr<NetPacket> makePacket(const char *data, std::size_t bytes_recvd);
    bool isGoingOn_ = true;
public:
    RawUdpConnection(boost::asio::ip::udp::socket &socket, boost::asio::ip::udp::endpoint remote_endpoint, RawUdpRoom &room);
    ~RawUdpConnection();
    bool onReceived(const char *data, std::size_t bytes_recvd);
    void send(const NetPacket& msg);
    void sendUdp(const char *buf, int len);
};


class RawUdpRoom {
    friend class RawUdpConnection;
    friend class RawUdpPriv;
private:
    std::set<std::shared_ptr<RawUdpConnection>> peers_;
    std::shared_ptr<RawUdpConnection> findPeer(boost::asio::ip::udp::endpoint endpoint);
public:
    void insert(boost::asio::ip::udp::socket &socket, boost::asio::ip::udp::endpoint endpoint, const char *data, std::size_t bytes_recvd);
    void kickMeOut(std::shared_ptr<RawUdpConnection> me);
};


class RawUdpServer : public Server
{
private:

public:
    RawUdpServer(boost::asio::io_service& io_context, short port);
    void doReceive();
private:
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    enum { max_length = 1<<10 };
    char data_[max_length];
    RawUdpRoom room_;
};