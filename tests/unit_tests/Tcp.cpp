#include "evc/TcpClient.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "evc/Log.hpp"

enum {body_length = 4};

int main(int argc, char **argv){
    bool successed = true;
    try{
        std::vector<TcpClient *> clients;
        std::vector<std::thread *> threads;
        std::vector<NetPacket *> goldens;

        for (int i = 0; i < 2; i++){
            std::vector<char> tmp;
            tmp.push_back('m');
            tmp.push_back('s');
            tmp.push_back('g');
            tmp.push_back('0'+i);
            goldens.push_back(new NetPacket(NetPacket::PayloadType::textPacket, tmp));
        }

        for (int i =0 ; i < 2; i++){
            clients.push_back(new TcpClient(argv[1], argv[2], [=](const NetPacket &netPacket){
                auto size = netPacket.payloadLength();
                if (size!=body_length){
                    throw;
                }

                auto goldenIndex = 1-i;
                auto _1 = goldens[goldenIndex]->payload();
                auto _2 = netPacket.payload();
                if(memcmp(_1, _2, size)){
                    throw "failed";
                }
            }, std::to_string(i)));
        }

        for (auto i = 0; i < 2; i++){
            auto client = clients[i];
            threads.push_back(new std::thread([=, &successed](){
                try {
                    for (int j = 0; j< 10000; j++){
                        client->send(*(goldens[i]));
                        std::this_thread::sleep_for(std::chrono::microseconds(10));
                    }
                }
                catch (std::exception& e) {
                    successed =false;
                    std::cerr << "Exception: " << e.what() << "\n";
                }
            }));
        }

        for (auto &pThread: threads){
            pThread->join();
        }
    }
    catch (std::exception& e) {
        successed =false;
        std::cerr << "Exception: " << e.what() << "\n";
    }

    if(successed){
        std::cout << "Everything is OK";
    }

    return 0;
}
