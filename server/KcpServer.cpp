#include "KcpServer.hpp"


std::shared_ptr<NetPacket> KcpConnection::makePacket(const char *data, std::size_t bytes_recvd)
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

bool KcpConnection::onReceived(const char *data, std::size_t bytes_recvd)
{
    ikcp_input(kcp_, data, bytes_recvd);

    char kcp_buf[NetPacket::FixHeaderLength + NetPacket::MaxPayloadLength] = "";
    int kcp_recvd_bytes = ikcp_recv(kcp_, kcp_buf, sizeof(kcp_buf));
    if (kcp_recvd_bytes <= 0)
    {
        LOGD << "kcp_recvd_bytes<=0 " << kcp_recvd_bytes << std::endl;
        return false;
    }

    auto oldSize = inputBuffer_.size();
    inputBuffer_.resize(oldSize + kcp_recvd_bytes);
    std::memcpy(inputBuffer_.data() + oldSize, kcp_buf, kcp_recvd_bytes);

    auto newPacket = makePacket(inputBuffer_.data(), inputBuffer_.size());
    if (newPacket) {
        // digest inputBuffer_
        auto oldSize = inputBuffer_.size();
        auto newSize = oldSize - newPacket->wholePackLength();
        std::memcpy(inputBuffer_.data(), inputBuffer_.data() + newPacket->wholePackLength(), newSize);
        inputBuffer_.resize(newSize);
    }

    if (newPacket) {
        // handle payload
        KcpPriv::processClientMessagePayload(*newPacket, shared_from_this(), room_);
    }
    return true;
}

void KcpConnection::send(const NetPacket& msg)
{
    ikcp_send(kcp_, msg.header(), msg.wholePackLength());
}

void KcpConnection::sendUdp(const char *buf, int len)
{
    socket_.async_send_to(
        boost::asio::buffer(buf, len),
        remote_endpoint_,
        [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
        //doReceive();
    });
}

KcpServer::KcpServer(boost::asio::io_service& io_context, short port) 
    : socket_(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
{
    doReceive();
}

void KcpServer::doReceive()
{
    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length),
        sender_endpoint_,
        [this](boost::system::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0) {
            LOGV << sender_endpoint_;
            room_.insert(socket_, sender_endpoint_, data_, bytes_recvd);
        }
        else {
            doReceive();
        }
    });
}


int KcpPriv::udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    if (!user) {
        return -1;
    }
    
    auto pKcpConnection = (KcpConnection*)(user);
    pKcpConnection->sendUdp(buf, len);
    return 0;
}

void KcpPriv::processClientMessagePayload(const NetPacket& msg, std::shared_ptr<KcpConnection> sender , KcpRoom &room)
{
    static bool isEchoMode = true;
    static uint32_t counterUserMessage_ = 0;
    auto payloadType = msg.payloadType();

    bool traceDump = true;
    switch (payloadType) {
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
            //for (auto participant : participants_) {
            //    if (participant == sender) {
            //        continue;
            //    }

            //    participant->deliver(msg);
            //}
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

std::shared_ptr<KcpConnection> KcpRoom::findPeer(boost::asio::ip::udp::endpoint endpoint)
{
    for (auto &peer : peers_) {
        if (peer->remote_endpoint_ == endpoint) {
            return peer;
        }
    }
    return nullptr;
}

void KcpRoom::insert(boost::asio::ip::udp::socket &socket, boost::asio::ip::udp::endpoint endpoint, const char *data, std::size_t bytes_recvd)
{
    auto peer = findPeer(endpoint);
    if (!peer) {
        peer = std::make_shared<KcpConnection>(socket, std::move(endpoint), *this);
        peers_.insert(peer);
    }
    peer->onReceived(data, bytes_recvd);
}

void KcpRoom::kickMeOut(std::shared_ptr<KcpConnection> me)
{
    peers_.erase(me);
}
