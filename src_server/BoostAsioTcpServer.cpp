
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



class Participant{
public:
    virtual ~Participant() {}
    virtual void deliver(const NetPacket& msg) = 0;
};




using ParticipantPointer = std::shared_ptr<Participant>;
using ClientPacketQueue = std::deque<NetPacket>;








class Room {
public:
    void join(ParticipantPointer participant){
        participants_.insert(participant);
    }



    void leave(ParticipantPointer participant){
        participants_.erase(participant);
    }



    void deliver(const NetPacket& msg,ParticipantPointer sender){
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
        , room_(room) { }


    void start(){
        room_.join(shared_from_this());
        readHeader();
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
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.data(), NetPacket::header_length),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec && read_msg_.decode_header()){
                readBody();
            }
            else{
                room_.leave(shared_from_this());
            }
        });
    }


    void readBody() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec){
                room_.deliver(read_msg_, shared_from_this());
                readHeader();
            }
            else{
                room_.leave(shared_from_this());
            }
        });
    }




    void write() {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
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
        do_accept();
    }

private:
    void do_accept(){
        acceptor_.async_accept(socket_,[this](boost::system::error_code errorCode){
            if (!errorCode) {
                std::make_shared<Session>(std::move(socket_), room_)->start();
            }

            do_accept();
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
