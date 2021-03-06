#include "evc/Alsa.hpp"
#include <iostream>


int main(int argc, char **argv){
    auto &device = Alsa::get();

    std::string micInfo;
    std::string spkInfo;
    if (device.init(micInfo, spkInfo)) {
        for (;;) {
            std::vector<short> micBuffer(blockSize);
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

    return 0;
}
