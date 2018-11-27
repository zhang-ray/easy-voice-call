#pragma once


#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include "Logger.hpp"

class UdpEndpoint : public boost::asio::ip::udp::endpoint {

};

class UdpServer
{
public:
    UdpServer(boost::asio::io_service& io_context, short port)
        : socket_(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)) {
        do_receive();
    }

    void do_receive() {
        socket_.async_receive_from(
            boost::asio::buffer(data_, max_length),
            sender_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
            if (!ec && bytes_recvd > 0){
                LOGV << sender_endpoint_;
                auto static lastEndpoint = sender_endpoint_;
                if (lastEndpoint != sender_endpoint_) {
                    LOGV << "lastEndpoint!=sender_endpoint_";
                }
                do_send(bytes_recvd);
            }
            else{
                do_receive();
            }
        });
    }

    void do_send(std::size_t length){
        socket_.async_send_to(
            boost::asio::buffer(data_, length), 
            sender_endpoint_,
            [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/){
            do_receive();
        });
    }

private:
    boost::asio::ip::udp::socket socket_;
    UdpEndpoint sender_endpoint_;
    enum { max_length = 1<<10 };
    char data_[max_length];
};