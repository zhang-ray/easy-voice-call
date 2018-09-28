#include <stdio.h>
#include <string.h>   
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>   
#include <stdlib.h>
#include <string>

#include "evc/ReturnType.hpp"

class TinyServer {
private:
    int socket_desc_;
    sockaddr_in server_;
    const int block_size_ = 1<<10;
    
public:
    ReturnType init(char *ip, int port){
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
        struct sockaddr_in client;
        char client_message[2000];
        //accept connection from an incoming client
        auto c = sizeof(struct sockaddr_in);

        auto client_sock = accept(socket_desc_, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock < 0){
            return "accept failed";
        }
        
        int read_size;
        //Receive a message from client
        while( (read_size = recv(client_sock , client_message , block_size_ , 0)) > 0 ) {
            //Send the message back to client
            write(client_sock , client_message , strlen(client_message));
        }
        
        if(read_size == 0) {
            return "Client disconnected";
            fflush(stdout);
        }
        else if(read_size == -1) {
            return "recv failed";
        }
        
        return 0;
    }
};



int main(int argc, char **argv){
    TinyServer server;
    if (server.init(argv[1], atoi(argv[2]))){
        server.start();
    }

    return 0;
}