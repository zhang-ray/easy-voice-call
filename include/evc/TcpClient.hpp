#pragma once
#include "NetClient.hpp"
#include "ReturnType.hpp"

#include <stdio.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <string>
#include <vector>

class TcpClient : public NetClient {
private:
    int sock_;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    char *ip_;
    int port_;
    std::string id_;
public:
    TcpClient(const std::string &id="") : id_(id){ }
    std::string id(){return id_;}
    ReturnType connect(char *ip, int port){
        //Create socket
        sock_ = socket(AF_INET , SOCK_STREAM , 0);
        if (sock_ == -1) {
            return "Could not create socket";
        }
        
        server.sin_addr.s_addr = inet_addr(ip);
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
    
        //Connect to remote server
        if (::connect(sock_ , (struct sockaddr *)&server , sizeof(server)) < 0) {
            return "connect failed. Error";
        }

        ip_ = ip;
        port_ = port;
        return 0;
    }

    ReturnType send(const char *buf, const size_t len){
        if(::send(sock_ , buf, len, 0) < 0) {
            return "Send failed";
        }
        return 0;
    }

    ReturnType send(const std::vector<char> &data){
        if(::send(sock_ , data.data() , data.size() , 0) < 0) {
            return "Send failed";
        }
        return 0;
    }

    ReturnType recv(std::vector<char> &data){
        data.resize(2000);
        if(::recv(sock_ , data.data() , data.size() , 0) < 0) {
            return "recv failed";
        }
        
        return 0;
    }
};