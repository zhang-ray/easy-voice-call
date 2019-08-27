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
#include "Logger.hpp"

using NetPacketQueue = std::deque<NetPacket>;



class TcpClient;

class BoostAsioTcpClient{
    friend class TcpClient;
private:
    std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived_;
    TcpClient *pTcpClient_ = nullptr;
public:
    BoostAsioTcpClient(
            boost::asio::io_service& io_service,
            boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
            decltype(onDataReceived_) onDataReceived,
            TcpClient *pTcpClient
            );



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
                                boost::asio::buffer(read_msg_.header(), NetPacket::FixHeaderLength),
                                [this](boost::system::error_code ec, std::size_t /*length*/){
            if (!ec && read_msg_.checkHeader()){
                readPayload();
            }
            else{
                socket_.close();
            }
        });
    }




    void readPayload();

    void write(){
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().header(),
                                                     write_msgs_.front().wholePackLength()),
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
    friend class BoostAsioTcpClient;
private:
    std::string id_;
    boost::asio::io_service io_service;
    std::shared_ptr<BoostAsioTcpClient> pClient = nullptr;
    std::shared_ptr<std::thread> pThread = nullptr;
    bool isConnected_ = false;
public:
    TcpClient() {}

    virtual ReturnType init(
        const std::string &serverHost,
        const std::string &serverPort,
        std::function<void(const NetClient & myClient, const std::shared_ptr<NetPacket> netPacket) > onDataReceived
    ) override {
        boost::asio::ip::tcp::resolver resolver(io_service);
        pClient = std::make_shared<BoostAsioTcpClient>(io_service, resolver.resolve({ serverHost, serverPort }), onDataReceived, this);
        pThread = std::make_shared<std::thread>([this](){ io_service.run(); });
        return 0;
    }


    ~TcpClient(){
        try {
            pClient->close();
            pThread->join();
        }
        catch (std::exception &e) {
            LOGE_STD_EXCEPTION(e);
        }
    }

    virtual bool isConnected() const override {return isConnected_;}

    std::string id(){return id_;}

    ReturnType send(const NetPacket &netPacket) const override {
        pClient->write(netPacket);
#ifdef _DEBUG
        LOGV << netPacket.info();
#endif // _DEBUG

        return 0;
    }



};







BoostAsioTcpClient::BoostAsioTcpClient(
        boost::asio::io_service& io_service,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
        decltype(onDataReceived_) onDataReceived,
        TcpClient *pTcpClient
        )
    : io_service_(io_service)
    , socket_(io_service)
    , onDataReceived_(onDataReceived)
    , pTcpClient_(pTcpClient)
{
    // connect(endpoint_iterator);

    boost::asio::async_connect(socket_,
                               endpoint_iterator,
                               [this](boost::system::error_code ec,
                               boost::asio::ip::tcp::resolver::iterator){
        if (ec) {
            socket_.close();
            return;
        }

        pTcpClient_->isConnected_ = true;
        readHeader();
    });
}

void BoostAsioTcpClient::readPayload(){
    boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.payload(), read_msg_.payloadLength()),
                            [this](boost::system::error_code ec, std::size_t /*length*/) {
        if (ec) {
            socket_.close();
            return;
        }


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
        onDataReceived_(*pTcpClient_, std::make_shared<NetPacket>(read_msg_));
        readHeader();
    });
}