#include <iostream>
#include <thread>
#include <chrono>
#include "evc/CallbackStylePortaudioEndpoint.hpp"
#include "evc/Logger.hpp"


/*
subjective test
- delay
- glitch noise?
*/

int main(int argc, char **argv) {
    try{
        auto device = CallbackStylePortaudioEndpoint::get();

        if (auto ret = device.init(
            nullptr,
            [](const int16_t * inputBuffer, int16_t * outputBuffer, const uint32_t framesPerBuffer) {
            std::memcpy(outputBuffer, inputBuffer, framesPerBuffer * sizeof(int16_t));
        })) {
            auto names = device.getEndpointName();
            device.asyncStart();
            std::this_thread::sleep_for(std::chrono::seconds(100));
            device.syncStop();
        }
        else {
            LOGV << "device.init -> " << ret;
        }
    }
    catch (const std::exception &e) {
        dumpException(e);
    }
    return 0;
}
