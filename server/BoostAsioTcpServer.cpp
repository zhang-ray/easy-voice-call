
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "evc/NetPacket.hpp"
#include "evc/ReturnType.hpp"
#include "evc/Log.hpp"


class Participant{
public:
    virtual ~Participant() {}
    virtual void deliver(const NetPacket& msg) = 0;
};




using ParticipantPointer = std::shared_ptr<Participant>;
using ClientPacketQueue = std::deque<NetPacket>;








class Room {
public:
    ReturnType join(ParticipantPointer participant){
        auto size__ = participants_.size();
        if (participants_.size()>=2){
            return "participants_.count()>2";
        }
        participants_.insert(participant);

        return 0;
    }



    void leave(ParticipantPointer participant){
        participants_.erase(participant);
    }



    void processClientMessagePayload(const NetPacket& msg,ParticipantPointer sender){
        for (auto participant: participants_){
            if (participant == sender){
                continue;
            }

            participant->deliver(msg);
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
        // LOGV << __FUNCTION__;
    }

    ~Session(){
        // LOGV << __FUNCTION__;
    }


    ReturnType start(){
        auto ret = room_.join(shared_from_this());
        if (!ret){
            std::cout << std::endl << ret.message();
            return ret;
        }

        std::cout << std::endl << "one client connected!";

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

private:
    void readHeader() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.header(), NetPacket::fixHeaderLength),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec && read_msg_.checkHeader()){
                readPayload();
            }
            else{
                room_.leave(shared_from_this());
            }
        });
    }


    void readPayload() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.payload(), read_msg_.payloadLength()),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec){
                room_.processClientMessagePayload(read_msg_, shared_from_this());
                readHeader();
            }
            else{
                room_.leave(shared_from_this());
            }
        });
    }




    void write() {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front().header(), write_msgs_.front().wholePackLength()),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/){
            if (!ec){
                write_msgs_.pop_front();
                if (!write_msgs_.empty()){
                    write();
                }
            }else{
                room_.leave(shared_from_this());
            }
        });
    }


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
        doAccept();
    }

private:
    void doAccept(){
        acceptor_.async_accept(socket_,[this](boost::system::error_code errorCode){
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





int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: EvcServer <port>\n";
            return -1;
        }

        boost::asio::io_service io_service;
        Server server(
                    io_service,
                    boost::asio::ip::tcp::endpoint(
                        boost::asio::ip::tcp::v4(),
                        std::atoi(argv[1])
                    )
                );
        io_service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
