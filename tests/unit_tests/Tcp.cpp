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
        std::vector<std::string> goldens;

        for (int i = 0; i < 2; i++){
            std::string msg = "msg";
            msg.append(std::to_string(i));
            goldens.push_back(msg);
        }

        for (int i =0 ; i < 2; i++){
            clients.push_back(new TcpClient(argv[1], argv[2], [=](const char *pData, std::size_t size){
                if (size!=body_length){
                    throw;
                }

                auto goldenIndex = 1-i;
                auto _1 = goldens[goldenIndex].data();
                auto _2 = pData;
                if(memcmp(_1, _2, size)){
                    LOGE << "failed";
                    throw;
                }

            }, std::to_string(i)));
        }

        for (auto i = 0; i < 2; i++){
            auto client = clients[i];
            threads.push_back(new std::thread([=, &successed](){
                try {
                    for (int j = 0; j< 10000; j++){
                        client->send(goldens[i].data(), goldens[i].size());
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
        LOGD << "Everything is OK";
    }


    return 0;
}
