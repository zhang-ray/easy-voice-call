#include "Worker.hpp"






class Case {
public:
    virtual ~Case() {}
    virtual ReturnType run() = 0;
};



void onNetworkChanged(const NetworkState &newState, const std::string &extraMessage) {
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
}

class Case_noDevice : public Case {
public:
    virtual ReturnType run() override {
        boost::property_tree::ptree root;

        root.put<std::string>("server.host", "127.0.0.1");
        root.put<std::string>("server.port", "1222");
        root.put<bool>("needRealAudioEndpoint", false);
        root.put<std::string>("audioInStub", "sine");
        root.put<std::string>("audioOutDumpPath", "audioOut_mono16le16kHz.pcm");

        Worker worker;
        auto ret = worker.init(
            root,
            [](const std::string &, const std::string &) {},
            [](const AudioIoVolume) {},
            [](const bool) {}
        );
        if (!ret) {
            LOGE << ret.message();
            return -1;
        }


        std::thread bg([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            worker.syncStop();
        });
        bg.detach();

        auto ret2 = worker.syncStart(&onNetworkChanged);
        if (!ret2) {
            return ret2;
        }

        return 0;
    }
};


/*
TODO
- comparing audioInStub with DUMP_AUDIO_OUT
*/

int main(void) {
    TrickyBoostLog trickyBoostLog(boost::log::trivial::severity_level::debug);

    auto cases = std::initializer_list<Case*>{ new Case_noDevice() };

    for (auto &oneCase : cases) {
        auto className = typeid(*oneCase).name();
        LOGI << "\n\n\nstart check " << className;
        if (!oneCase->run()) {
            LOGE << className << " failed";
            return -1;
        }
        LOGI << className << " OK";
    }

    LOGI << "all tests were passed!\n\n\n";
    return 0;
}