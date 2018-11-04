#include "evc/RingBuffer.hpp"
#include "evc/AudioDevice.hpp"
#include <thread>
#include <chrono>
#include <cstdio>

int main(void){
    RingBuffer ringBuff(1*1, 100);
    auto s = ringBuff.size();
    std::thread t([&]() {
        for (int i = 0; ; i++) {
            uint8_t data;
            if (!ringBuff.popElements(&data, 1)) {
                printf("pop failed\n");
            }
            else {
                printf("got %d, remain %d element(s)\n" , data, ringBuff.size());
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });


    for (int i =0; ;i++) {
        uint8_t data;
        data = i%255;
        if (!ringBuff.pushElements(&data, 1)) {
            printf("push failed\n");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    t.join();
    return 0;
}