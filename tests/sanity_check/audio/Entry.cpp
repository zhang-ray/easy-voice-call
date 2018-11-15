#include "Worker.hpp"






class Case {
public:
    virtual ~Case() {}
    virtual int run() = 0;
};




class Case_bypassAudioEndpoint : public Case {
public:
    virtual int run() override
    {
        boost::property_tree::ptree root;
        
        root.put<bool>("needNetworkStub", true);
        root.put<bool>("bypassLocalAudioEndpoing", true);
        root.put<bool>("needAec", false);

        Worker worker;
        if (worker.init(
            root,
            [](const std::string &, const std::string &) {},
            [](const AudioIoVolume) {},
            [](const bool) {}
        )) {
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

        return -1;
    }

};


//////////////////////////////////////////////////////////////////////////
// needNetworkStub
//////////////////////////////////////////////////////////////////////////
class Case_bypassServer : public Case {
public:
    virtual int run() override
    {
        boost::property_tree::ptree root;

        root.put<bool>("needNetworkStub", true);
        root.put<bool>("needAec", false);

        Worker worker;
        if (worker.init(
            root,
            [](const std::string &, const std::string &) {},
            [](const AudioIoVolume) {},
            [](const bool) {}
        )) {
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

        return -1;
    }
};



int main(void) {
    auto cases = std::initializer_list<Case*> { new Case_bypassAudioEndpoint() , new Case_bypassServer() };

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