#include <deque>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include "NetPacket.hpp"
#include "ReturnType.hpp"
#include "ServerLogger.hpp"

class Participant {
public:
    virtual ~Participant() {}
    virtual void deliver(const NetPacket& msg) = 0;
    virtual std::string info() = 0;
};


bool isEchoMode = false;


using ParticipantPointer = std::shared_ptr<Participant>;
using ClientPacketQueue = std::deque<NetPacket>;





/// TODO:
/// Response some voice like "waiting for someone else login" ?
///



class Room {
public:
    ReturnType join(ParticipantPointer participant) {
        auto size__ = participants_.size();
        if (participants_.size() >= 2) {
            return "participants_.count()>2";
        }
        participants_.insert(participant);

        return 0;
    }



    void leave(ParticipantPointer participant) {
        participants_.erase(participant);
    }



    void processClientMessagePayload(const NetPacket& msg, ParticipantPointer sender) {
        static uint32_t counterUserMessage_ = 0;
        auto payloadType = msg.payloadType();

        bool traceDump = true;
        switch (payloadType) {
        case NetPacket::PayloadType::HeartBeatRequest: {
            sender->deliver(NetPacket(NetPacket::PayloadType::HeartBeatResponse));
            break;
        }
        case NetPacket::PayloadType::TextMessage:
        case NetPacket::PayloadType::AudioMessage:
        {
            counterUserMessage_++;

            if (isEchoMode) {
                /// echo mode
                sender->deliver(msg);;
            }
            else {
                /// broadcast mode
                for (auto participant : participants_) {
                    if (participant == sender) {
                        continue;
                    }

                    participant->deliver(msg);
                }
            }

            traceDump = false;
            break;
        }
        case NetPacket::PayloadType::LoginRequest:
        {
            //// GOT login request!!!
            sender->deliver(NetPacket(NetPacket::PayloadType::LoginResponse));
            break;
        }
        case NetPacket::PayloadType::LogoutRequest:
        {
            leave(sender);
            break;
        }

        default:
            // WHAT?
            break;
        }

        if (traceDump || counterUserMessage_ % 100 == 0) {
            BOOST_LOG_TRIVIAL(trace) << "[" << sender->info() << "] " << msg.info() ;
            if (counterUserMessage_ % 100 == 0) {
                BOOST_LOG_TRIVIAL(trace) << "\t\tgot " << counterUserMessage_ << "\tUserMessage totally";
            }
        }
    }

private:
    std::set<ParticipantPointer> participants_;
};









// one client-server connection
class Session : public Participant, public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::ip::tcp::socket socket, Room& room)
        : socket_(std::move(socket))
        , room_(room) {
        BOOST_LOG_TRIVIAL(trace) << __FUNCTION__;
    }

    ~Session() {
        BOOST_LOG_TRIVIAL(trace) << __FUNCTION__;
    }


    ReturnType start() {
        auto ret = room_.join(shared_from_this());
        if (!ret) {
            BOOST_LOG_TRIVIAL(error) << "Session start failed:" << ret.message();
            return ret;
        }

        BOOST_LOG_TRIVIAL(info) << "on accepted: " << socket_.remote_endpoint().address();

        readHeader();

        return 0;
    }



    void deliver(const NetPacket& msg) {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress) {
            write();
        }
    }

    virtual std::string info() override {
        auto __ = socket_.remote_endpoint();
        return __.address().to_string() + ":" + std::to_string(__.port());
    }

private:
    void readHeader() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.header(), NetPacket::FixHeaderLength),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec && read_msg_.checkHeader()) {
                readPayload();
            }
            else {
                room_.leave(shared_from_this());
            }
        });
    }


    void readPayload() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.payload(), read_msg_.payloadLength()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                room_.processClientMessagePayload(read_msg_, shared_from_this());
                readHeader();
            }
            else {
                room_.leave(shared_from_this());
            }
        });
    }




    void write() {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front().header(), write_msgs_.front().wholePackLength()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                write_msgs_.pop_front();
                if (!write_msgs_.empty()) {
                    write();
                }
            }
            else {
                room_.leave(shared_from_this());
            }
        });
    }

private:
    boost::asio::ip::tcp::socket socket_;
    Room& room_;
    NetPacket read_msg_;
    ClientPacketQueue write_msgs_;
};




// one TCP server
class Server {
public:
    Server(boost::asio::io_service& io_service, const boost::asio::ip::tcp::endpoint& endpoint)
        : acceptor_(io_service, endpoint)
        , socket_(io_service) {

            {
                BOOST_LOG_TRIVIAL(info) << "Server started: port = " << endpoint.port() << " " << (isEchoMode ? "echo" : "broadcast") << " mode";
            }

            doAccept();
    }

private:
    void doAccept() {
        acceptor_.async_accept(socket_, [this](boost::system::error_code errorCode) {
            if (!errorCode) {
                std::make_shared<Session>(std::move(socket_), room_)->start();
            }

            //            LOGV << "async_accept new connection. socket_ = " << socket_;
            doAccept();
        });
    }

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    Room room_;
};


void printUsage() {
    std::cerr << "Usage: EvcServer <port>\n";
    std::cerr << "   or: EvcServer <port> <broadcast/echo>\n";
}


int main(int argc, char* argv[]) {
    try {
        int port = 80;

        // init port
        {
            if ((argc == 2) || (argc == 3)) {
                if (argc == 3) {
                    if (!strcmp("broadcast", argv[2])) {
                        isEchoMode = false;
                    }
                    else if (!strcmp("echo", argv[2])) {
                        isEchoMode = true;
                    }
                    else {
                        printUsage();
                        return -1;
                    }
                }
                port = std::atoi(argv[1]);
            }
            else {
                printUsage();
                return -1;
            }
        }

        BOOST_LOG_TRIVIAL(trace) << "PORT=" << port;


        /// init logger
        {
            //boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
        }

        boost::asio::io_service io_service;
        Server server(
            io_service,
            boost::asio::ip::tcp::endpoint(
                boost::asio::ip::tcp::v4(),
                port
            )
        );
        io_service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

