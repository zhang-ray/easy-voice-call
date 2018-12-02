#include "RawUdpServer.hpp"

// Server.cpp
extern bool isEchoMode;

std::shared_ptr<NetPacket> RawUdpConnection::makePacket(const char *data, std::size_t bytes_recvd)
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

RawUdpConnection::RawUdpConnection(
    boost::asio::ip::udp::socket &socket,
    boost::asio::ip::udp::endpoint remote_endpoint,
    RawUdpRoom &room
) : remote_endpoint_(remote_endpoint)
, room_(room)
, socket_(socket)
{

}

RawUdpConnection::~RawUdpConnection()
{
    isGoingOn_ = false;
}

bool RawUdpConnection::onReceived(const char *data, std::size_t bytes_recvd) {
    auto newPacket = makePacket(data, bytes_recvd);
    if (newPacket) {
        // handle payload
        RawUdpPriv::processClientMessagePayload(*newPacket, shared_from_this(), room_);
    }
    return true;
}

void RawUdpConnection::send(const NetPacket& msg)
{
    sendUdp(msg.header(), msg.wholePackLength());
}

void RawUdpConnection::sendUdp(const char *buf, int len)
{
    socket_.async_send_to(
        boost::asio::buffer(buf, len),
        remote_endpoint_,
        [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
        //doReceive();
    });
}

RawUdpServer::RawUdpServer(boost::asio::io_service& io_context, short port) 
    : socket_(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
{
    doReceive();
}

void RawUdpServer::doReceive()
{
    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length),
        sender_endpoint_,
        [this](boost::system::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0) {
            //LOGV << sender_endpoint_;
            room_.insert(socket_, sender_endpoint_, data_, bytes_recvd);
        }

        doReceive();
    });
}


void RawUdpPriv::processClientMessagePayload(const NetPacket& msg, std::shared_ptr<RawUdpConnection> sender , RawUdpRoom &room)
{
    static uint32_t counterUserMessage_ = 0;
    auto payloadType = msg.payloadType();

    bool traceDump = true;
    switch (payloadType) {
        //LOGI << "payloadType=" << NetPacket::getDescription(payloadType);
    case NetPacket::PayloadType::HeartBeatRequest: {
        sender->send(NetPacket(NetPacket::PayloadType::HeartBeatResponse));
        break;
    }
    case NetPacket::PayloadType::TextMessage:
    case NetPacket::PayloadType::AudioMessage:
    {
        counterUserMessage_++;

        if (isEchoMode) {
            /// echo mode
            sender->send(msg);;
        }
        else {
            /// broadcast mode
            for (auto participant : room.peers_) {
                if (participant == sender) {
                    continue;
                }

                participant->send(msg);
            }
        }

        traceDump = false;
        break;
    }
    case NetPacket::PayloadType::LoginRequest:
    {
        //// GOT login request!!!
        sender->send(NetPacket(NetPacket::PayloadType::LoginResponse));
        break;
    }
    case NetPacket::PayloadType::LogoutRequest:
    {
        room.kickMeOut(sender);
        break;
    }

    default:
        // WHAT?
        break;
    }

}

std::shared_ptr<RawUdpConnection> RawUdpRoom::findPeer(boost::asio::ip::udp::endpoint endpoint)
{
    for (auto &peer : peers_) {
        if (peer->remote_endpoint_ == endpoint) {
            return peer;
        }
    }
    return nullptr;
}

void RawUdpRoom::insert(boost::asio::ip::udp::socket &socket, boost::asio::ip::udp::endpoint endpoint, const char *data, std::size_t bytes_recvd)
{
    auto peer = findPeer(endpoint);
    if (!peer) {
        peer = std::make_shared<RawUdpConnection>(socket, std::move(endpoint), *this);
        peers_.insert(peer);
    }
    peer->onReceived(data, bytes_recvd);
}

void RawUdpRoom::kickMeOut(std::shared_ptr<RawUdpConnection> me)
{
    peers_.erase(me);
}
