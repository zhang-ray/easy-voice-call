#pragma once
#include "NetClient.hpp"
#include "ReturnType.hpp"

#include <stdio.h>
#include <cstring>

#include <cstdlib>
#include <string>
#include <vector>











#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "NetPacket.hpp"


using NetPacketQueue = std::deque<NetPacket>;




class BoostAsioTcpClient{
private:
    std::function<void(const char* pData, std::size_t length)> onDataReceived_;
public:
    BoostAsioTcpClient(
            boost::asio::io_service& io_service,
            boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
            decltype(onDataReceived_) onDataReceived
            )
        : io_service_(io_service)
        , socket_(io_service)
        , onDataReceived_(onDataReceived) {
        // connect(endpoint_iterator);

        boost::asio::async_connect(socket_,
                                   endpoint_iterator,
                                   [this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator){
            if (ec) {
                socket_.close();
                return;
            }

            readHeader();
        });
    }



    void write(const NetPacket& msg) {
        io_service_.post([this, msg]() {
            bool write_in_progress = !write_msgs_.empty();
            write_msgs_.push_back(msg);
            if (!write_in_progress){
                write();
            }
        });
    }

    void close(){
        io_service_.post([this]() { socket_.close(); });
    }

private:

    void readHeader(){
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), NetPacket::header_length),
                                [this](boost::system::error_code ec, std::size_t /*length*/){
            if (!ec && read_msg_.decodeHeader()){
                readBody();
            }
            else{
                socket_.close();
            }
        });
    }




    void readBody(){
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                ////////////////////////////////
                ////////////////////////////////
                ////////////////////////////////
                ////////////////////////////////
                /* GOT DATA! */
                ////////////////////////////////
                ////////////////////////////////
                ////////////////////////////////
                ////////////////////////////////
                ////////////////////////////////
                ////////////////////////////////
                onDataReceived_(read_msg_.body(), read_msg_.body_length());
                readHeader();
            }
            else{
                socket_.close();
            }
        });
    }

    void write(){
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this](boost::system::error_code ec, std::size_t /*length*/){
            if (!ec){
                write_msgs_.pop_front();
                if (!write_msgs_.empty())
                {
                    write();
                }
            }
            else{
                socket_.close();
            }
        });
    }

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    NetPacket read_msg_;
    NetPacketQueue write_msgs_;
};


class TcpClient : public NetClient {
private:
    std::string id_;
    boost::asio::io_service io_service;
    std::shared_ptr<BoostAsioTcpClient> pClient = nullptr;
    std::shared_ptr<std::thread> pThread = nullptr;
public:
    TcpClient(const char *host, const char *ip,
              std::function<void(const char* pData, std::size_t length)> onDataReceived_,
              const std::string &id="") : id_(id){
        boost::asio::ip::tcp::resolver resolver(io_service);
        pClient = std::make_shared<BoostAsioTcpClient>(io_service, resolver.resolve({ host, ip }), onDataReceived_);
        pThread = std::make_shared<std::thread>([this](){ io_service.run(); });

        /*
        char line[NetPacket::max_body_length + 1];
        while (std::cin.getline(line, NetPacket::max_body_length + 1))
        {
            NetPacket msg;
            msg.body_length(std::strlen(line));
            std::memcpy(msg.body(), line, msg.body_length());
            msg.encode_header();
            pClient->write(msg);
        }
        */
    }


    ~TcpClient(){
        pClient->close();
        pThread->join();
    }

    std::string id(){return id_;}

    ReturnType send(const NetPacket &msg){
        pClient->write(msg);
        return 0;
    }


};




