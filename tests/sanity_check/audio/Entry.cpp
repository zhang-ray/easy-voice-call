#include "Worker.hpp"






class Case {
public:
    virtual ~Case() {}
    virtual int run() = 0;
};



class Case_noDevice : public Case {
public:
    virtual int run() override {
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

        worker.asyncStart([this](
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
        });

        std::this_thread::sleep_for(std::chrono::seconds(10));
        return 0;
    }

};


/*
TODO
- comparing audioInStub with DUMP_AUDIO_OUT
*/

int main(void) {
    auto cases = std::initializer_list<Case*> { new Case_noDevice()};

    for (auto &oneCase : cases) {
        auto className = typeid(*oneCase).name();
        LOGI << "\n\n\nstart check " << className;
        if (oneCase->run()) {
            LOGE << className << " failed";
            return -1;
        }
        else {
            LOGI << className << " OK";
        }
    }

    LOGI << "all tests were passed!";
    return 0;
}