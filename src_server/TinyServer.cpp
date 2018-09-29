#include <stdio.h>
#include <string.h>   
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif   
#include <stdlib.h>
#include <string>
#include <thread>
#include <iostream>

#include "evc/ReturnType.hpp"

// 1v1 server
class TinyServer {
private:
	int socket_desc_;
	sockaddr_in server_;
	enum ___{
		block_size_ = 1 << 10
	};
public:
    ReturnType init(char *ip, int port){
#ifdef WIN32
        WSADATA wsaData;
        WSAStartup( MAKEWORD(2, 2), &wsaData);
#endif
        //Create socket
        socket_desc_ = socket(AF_INET , SOCK_STREAM , 0);
        if (socket_desc_ == -1) {
            return "Could not create socket";
        }
        
        
        //Prepare the sockaddr_in structure
        server_.sin_family = AF_INET;
        server_.sin_addr.s_addr = inet_addr(ip);
        server_.sin_port = htons(port);
        
        //Bind
        if( bind(socket_desc_,(struct sockaddr *)&server_ , sizeof(server_)) < 0) {
            //print the error message
            perror("bind failed. Error");
            return 1;
        }
        
        //Listen
        listen(socket_desc_ , 3);
        
        return 0;
    }

    ReturnType start(){
        struct sockaddr_in client1;
        struct sockaddr_in client2;
        //accept connection from an incoming client
        auto c = sizeof(struct sockaddr_in);

        auto client_sock_1 = accept(socket_desc_, (struct sockaddr *)&client1, (socklen_t*)&c);
        if (client_sock_1 < 0){
            return "accept failed";
        }

        auto client_sock_2 = accept(socket_desc_, (struct sockaddr *)&client2, (socklen_t*)&c);
        if (client_sock_2 < 0){
            return "accept failed";
        }
        

        // 1to2
        std::thread _1to2([&](){
            int read_size_1to2;
            char buff[block_size_];
            //Receive a message from client

            while( (read_size_1to2 = recv(client_sock_1 , buff , block_size_, 0)) > 0 ) {
                //Send the message back to client
                ::send(client_sock_2 , buff, read_size_1to2, 0);
            }
            
            if(read_size_1to2 == 0) {
                //return "Client disconnected";
                fflush(stdout);
            }
            else if(read_size_1to2 == -1) {
                //return "recv failed";
            }
        });


        // 2to1
        {
            int read_size_2to1;
            char client_message[block_size_];
            //Receive a message from client
            while( (read_size_2to1 = recv(client_sock_2 , client_message , block_size_ , 0)) > 0 ) {
                //Send the message back to client
                ::send(client_sock_1 , client_message , read_size_2to1, 0);
            }
            
            if(read_size_2to1 == 0) {
                return "Client disconnected";
                fflush(stdout);
            }
            else if(read_size_2to1 == -1) {
                return "recv failed";
            }
        }        


        // _1to2.join();
        return 0;
    }
};



int main(int argc, char **argv){
    TinyServer server;
    if (server.init(argv[1], atoi(argv[2]))){
        auto ret = server.start();
        if (!ret){
            std::cout << ret.message() << std::endl;
        }
    }

    return 0;
}