#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <fstream>

#include "evc/Worker.hpp"
#include "evc/Logger.hpp"


int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    std::string port = "1222";
    Worker worker;

    boost::property_tree::ptree root;
    if (worker.init(
        root,
        [](const std::string &, const std::string &) {},
        [](const AudioIoVolume) {},
        [](const bool) {}
    )) {
        worker.syncStart(
            host, port,
            nullptr,
            [](
                const NetworkState newState,
                const std::string extraMessage
                )
        {
            BOOST_LOG_TRIVIAL(info) << "newState=" << (uint8_t)(newState);
            BOOST_LOG_TRIVIAL(info) << "extraMessage=" << extraMessage;
        }
        );
    }
    return 0;
}
