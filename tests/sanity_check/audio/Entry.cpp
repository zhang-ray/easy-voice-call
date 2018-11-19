#include "Worker.hpp"

#include "ZeroCrossingRate.hpp"

#include "FileUtils.hpp"


namespace {
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


    class Case {
    protected:
        boost::property_tree::ptree root_;
    public:
        Case() {
            root_.put("server.host", "127.0.0.1");
            root_.put("server.port", "1222");
            root_.put("needRealAudioEndpoint", false);
            root_.put("audioOutDumpPath", "audioOut_mono16le16kHz.pcm");
            root_.put("audioInStub", "audioInStub.pcm");
        }


        virtual ReturnType run() {
            Worker worker;
            auto ret = worker.init(root_, nullptr, nullptr, nullptr);
            if (!ret) {
                return ret;
            }


            std::thread bg([&]() {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                worker.syncStop();
            });
            bg.detach();

            ret = worker.syncStart(&onNetworkChanged);
            if (!ret) {
                return ret;
            }

            return 0;
        }

        virtual ReturnType validate() = 0;

        virtual ~Case() {}
    };
}

class Case_noDevice : public Case {
private:
    inline ReturnType corrTowPcmFile(std::array<const std::string, 2> filePaths) {
        /// TOBE IMPL
        std::array<std::vector<int16_t>, 2> lists = { loadPcmFile(filePaths[0]), loadPcmFile(filePaths[1]) };

        auto minSize = std::min(lists[0].size(), lists[1].size());

        std::array<std::vector<double>, 2> results;
        for (int i = 0; i < 2; i++) {
            auto &theResult = results[i];
            const auto &theList = lists[i];
            for (int pos = 0; pos + blockSize < minSize; pos += blockSize) {
                std::array<int16_t, blockSize> arr;
                std::copy_n(theList.begin() + pos, blockSize, arr.begin());
                theResult.push_back(ZeroCrossingRate(arr));
            }
            auto sum = std::accumulate(theResult.begin(), theResult.end(), 0.0);
            LOGD << filePaths[i] << "\t feature = " << sum;
            if (std::fabs(sum) < std::fabs(std::numeric_limits<double>::lowest())) {
                LOGE << filePaths[i] << "\t is too low";
                return -1;
            }
            
        }
        return 0;
    }
public:
    virtual ReturnType validate() override {
        return corrTowPcmFile({ 
            root_.get<std::string>("audioOutDumpPath"), 
            root_.get<std::string>("audioInStub")
        });
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
        auto ret = oneCase->run();
        if (!ret) {
            LOGE << ret.message(); 
            LOGE << className << "run failed";
            return -1;
        }
        ret = oneCase->validate();
        if (!ret) {
            LOGE << ret.message();
            LOGE << className << "validate failed";
            return -1;
        }
        LOGI << className << " OK";
    }

    LOGI << "all tests were passed!\n\n\n";
    return 0;
}