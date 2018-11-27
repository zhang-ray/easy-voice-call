#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <fstream>

#include "IWorker.hpp"
#include "Logger.hpp"



void printUsage() {
    std::cout << "Usage: CLI config.json\n";
}


void onNetworkStateChanged(
    const NetworkState newState,
    const std::string extraMessage
)
{
    switch (newState)
    {
    case NetworkState::Connected:
        LOGI << "Connected";
        break;
    case NetworkState::Connecting:
        LOGI << "Connecting";
        break;
    case NetworkState::Disconnected:
        LOGI << "Disconnected";
        break;
    default:
        break;
    }


    if (!extraMessage.empty()) {
        LOGI << "extraMessage=" << extraMessage;
    }
};


int main(int argc, char* argv[]) {
#ifndef _DEBUG
    TrickyBoostLog trickyBoostLog(boost::log::trivial::severity_level::debug);
#endif // _DEBUG

    try {
        auto file = argv[1];

        boost::property_tree::ptree root;
        boost::property_tree::read_json(file, root);

        auto workingDuration = root.get <int>("cli.workingduration", -1);

        auto worker = IWorker::create();
        if (worker->init(
            root,
            [](const std::string &, const std::string &) {},
            [](const AudioIoVolume) {},
            [](const bool) {}
        )) {
            if (workingDuration < 0) {
                // infinite
                worker->syncStart(onNetworkStateChanged);
            }
            else {
                worker->asyncStart(onNetworkStateChanged);
                std::this_thread::sleep_for(std::chrono::seconds(workingDuration));
            }
        }
    }
    catch (const std::exception &e) {
        printUsage();
        LOGE_STD_EXCEPTION(e);
        return -1;
    }


    return 0;
}
