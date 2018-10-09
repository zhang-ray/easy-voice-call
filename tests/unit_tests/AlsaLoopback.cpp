#include "evc/Alsa.hpp"
#include "evc/Log.hpp"
#include <iostream>


int main(int argc, char **argv){
    LOGV;

    auto &device = Alsa::get();

    if (device.init()) {
        for (;;) {
            const auto blockSize = 1920;
            std::vector<short> micBuffer(blockSize);
            // TODO: use RingBuffer?
            auto retRead = device.read(micBuffer);
            if (retRead) {
                auto retWrite = device.write(micBuffer);
                if (!retWrite) {
                    std::cout << retWrite.message() << std::endl;
                    exit(-1);
                }
            }
        }
    }

    LOGV;
    return 0;
}
