#include "evc/TcpClient.hpp"
#include <iostream>
#include <thread>
#include <chrono>

#define log(format , ...) do{ printf("%s:%d @%s\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }while(0);

int main(int argc, char **argv){
    log();
    
    TcpClient client1;
    {
        auto ret = client1.connect(argv[1], atoi(argv[2]));
        if (!ret){
            log();
            std::cout << ret.message() << std::endl;
        }
    }


    TcpClient client2;
    {
        auto ret = client2.connect(argv[1], atoi(argv[2]));
        if (!ret){
            
            std::cout << ret.message() << std::endl;
        }
    }

    log();
    std::vector<char> msg1;
    msg1.push_back('m');
    msg1.push_back('s');
    msg1.push_back('g');
    msg1.push_back('1');
    msg1.push_back('\0');

    for (int i = 0; i < 100000; i++){
        if (auto retSend = client1.send(msg1)){
            std::vector<char> receivedData;
            if (client2.recv(receivedData)){
                for (int i = 0; i < msg1.size(); i++){
                    if (msg1[i]!=receivedData[i]){
                        log("msg1[%d](%c) != receivedData[%d](%c)",i, msg1[i], i, receivedData[i]);
                        exit(-1);
                    }
                }
            }
        }
        else{
            std::cout << retSend.message() << std::endl;
        }
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    log();
    return 0;
}