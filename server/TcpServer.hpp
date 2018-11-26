#pragma once
#include <deque>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include "NetPacket.hpp"
#include "ReturnType.hpp"
#include "Logger.hpp"
#include <functional>
#include "Room.hpp"




// one client-server connection
class TcpSession : public Participant, public std::enable_shared_from_this<TcpSession> {
public:
    TcpSession(boost::asio::ip::tcp::socket socket, Room& room)
        : socket_(std::move(socket))
        , room_(room) {
        BOOST_LOG_TRIVIAL(trace) << __FUNCTION__;
    }

    ~TcpSession() {
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



    virtual void deliver(const NetPacket& msg) override {
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
class TcpServer {
public:
    TcpServer(boost::asio::io_service& io_service, int port)
        : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
        , socket_(io_service) {
        BOOST_LOG_TRIVIAL(info) << "Server started: port = " << port << " " << (isEchoMode ? "echo" : "broadcast") << " mode";
        doAccept();
    }

private:
    void doAccept() {
        acceptor_.async_accept(socket_, [this](boost::system::error_code errorCode) {
            if (!errorCode) {
                std::make_shared<TcpSession>(std::move(socket_), room_)->start();
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




