#include "evc/TcpClient.hpp"
#include <iostream>


int main(void){
    printf("%s:%d @%s\n", __FILE__, __LINE__, __FUNCTION__);
    TcpClient client;
    auto ret = client.connect("127.0.0.1", 8000);
    if (!ret){
        printf("%s:%d @%s\n", __FILE__, __LINE__, __FUNCTION__);
        std::cout << ret.message() << std::endl;
    }


    printf("%s:%d @%s\n", __FILE__, __LINE__, __FUNCTION__);
    std::vector<char> hehe;
    hehe.push_back('w');
    hehe.push_back('t');
    hehe.push_back('f');

    if (auto retSend = client.send(hehe)){
        std::vector<char> receivedData;
        printf("%s:%d @%s\n", __FILE__, __LINE__, __FUNCTION__);
        if (client.recv(receivedData)){
            printf("%s:%d @%s\n", __FILE__, __LINE__, __FUNCTION__);
            printf("received: %s\n", receivedData.data());
        }
    }
    else{
        std::cout << retSend.message() << std::endl;
    }


    printf("%s:%d @%s\n", __FILE__, __LINE__, __FUNCTION__);
    return 0;
}