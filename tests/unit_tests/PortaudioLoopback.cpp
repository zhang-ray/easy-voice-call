
#include <iostream>
#include <thread>
#include <chrono>
#include "evc/PortAudio.hpp"

#define log(format , ...) do{ printf("%s:%d @%s\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }while(0);

int main(int argc, char **argv){
    log();


	auto &device = PortAudio::get();

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
    log();
    return 0;
}