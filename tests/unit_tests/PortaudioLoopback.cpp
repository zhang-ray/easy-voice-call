
#include <iostream>
#include <thread>
#include <chrono>
#include "evc/PortAudio.hpp"

int main(int argc, char **argv){
	auto &device = PortAudio::get();

    std::string micInfo;
    std::string spkInfo;
    if (device.init(micInfo, spkInfo)) {
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
    return 0;
}
