#pragma once

#include <memory>
#include <thread>
#include <string>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include "Logger.hpp"
#include "Lib.hpp"
#include "AudioVolume.hpp"
#include "CallbackStylePortaudioEndpoint.hpp"
#include "Profiler.hpp"
#include "ReturnType.hpp"
#include "NetClient.hpp"
#include <vector>
#include <climits>
#include <fstream>


class AudioDecoder;
class AudioEncoder;





class AudioIoVolume {
public:
    enum { MAX_VOLUME_LEVEL = 10};

    using Level = uint8_t;

    AudioInOut io_;
    Level level_;
    Level recentMaxLevel_;
public:
    AudioIoVolume(const AudioInOut io, const Level level, const Level recentMaxLevel)
        :io_(io)
        ,level_(level)
    , recentMaxLevel_(recentMaxLevel)
    {

    }
};

enum class NetworkState : unsigned char{
    Disconnected,
    Connecting,
    Connected,
};



/*
TODO
- Packet loss compensation
- cross fading for PLC
- store encoded package?
- Timestamp information should be stored
// https://www.boost.org/doc/libs/1_67_0/doc/html/boost/lockfree/spsc_queue.html  ?
*/
class AudioOutBuffer {
private:
    boost::lockfree::spsc_queue<std::array<int16_t, blockSize>> buffer_;
    std::array<int16_t, blockSize> emptyBuffer_;

    std::shared_ptr<std::ofstream> dumpMono16le16kHzPcmFile_ = nullptr;
public:
    AudioOutBuffer()
        : buffer_(100)
    {
        memset(emptyBuffer_.data(), 0, emptyBuffer_.size() * sizeof(int16_t));
    }
     
    ReturnType setAudioOutDumpPath(const std::string &filePath){
        dumpMono16le16kHzPcmFile_ = std::make_shared<std::ofstream>(filePath, std::ofstream::binary /*don't miss std::ofstream::binary*/);
        if (!dumpMono16le16kHzPcmFile_->is_open()) {
            dumpMono16le16kHzPcmFile_ = nullptr;
            return "!is_open()";
        }

        return 0;
    }

    void fetch(int16_t *outData) {
        std::array<int16_t, blockSize> data;
        if (buffer_.pop(data)) {
            std::memcpy((void*)outData, (void*)data.data(), sizeof(int16_t)*blockSize);
        }
        else {
            Profiler::get().emptyAudioOutBuffer_.addData(true);
            std::memcpy((char*)outData, (char*)emptyBuffer_.data(), emptyBuffer_.size() * sizeof(int16_t));
        }

        if (dumpMono16le16kHzPcmFile_) {
            dumpMono16le16kHzPcmFile_->write((char*)outData, sizeof(int16_t)*blockSize);
        }
    }

    void insert(const int16_t *inData) {
        std::array<int16_t, blockSize> data;
        std::memcpy(data.data(), inData, sizeof(int16_t)*blockSize);
        if (!buffer_.push(data)) {
            LOGD << " could not pushElements from spkBuffer_";
        }
    }

    ~AudioOutBuffer() {
        if (dumpMono16le16kHzPcmFile_) {
            if (dumpMono16le16kHzPcmFile_->is_open()) {
                dumpMono16le16kHzPcmFile_->close();
            }
        }
    }
};


class GeneratedWave {
public:
    virtual ~GeneratedWave(){}
};


class SineWave : public GeneratedWave {
private:
    std::vector<int16_t> wholePcm_;
public:
    double pi() { return std::atan(1) * 4; }
    SineWave(size_t sampleRate, double freq, double maxAmplitude, double durationSecond) {
        auto size_ = sampleRate * durationSecond;
        wholePcm_.resize(size_);
        for (int i = 0; i < size_; i++) {
            wholePcm_[i] = maxAmplitude * SHRT_MAX * std::sin(2 * pi() * freq / sampleRate * i);
        }
#ifdef DUMP_GeneratedWave
        DataDumper<int16_t> d("AudioInStub.txt");
        d.dump(wholePcm_);
#endif // _DEBUG
    }
    operator std::vector<int16_t>() const { return wholePcm_; }
};


class AudioInStub {
private:
    std::vector<int16_t> wholePcm_;
    size_t size_ = 0;
    size_t pos_ = 0;
    const bool loop_ = true;
public:
    AudioInStub(const decltype(wholePcm_) &wholePcm)
        : wholePcm_(wholePcm) 
        , size_(wholePcm_.size())
    { }

    const int16_t *get() {
        if (pos_ + blockSize >= size_) {
            pos_ = 0;
        }
        auto ret = wholePcm_.data() + pos_;
        pos_ += blockSize;
        return ret;
    }

    static std::shared_ptr<AudioInStub> loadFile(const std::string &filePathOrTag) {
        std::ifstream ifs(filePathOrTag.c_str(), std::ios::binary | std::ios::ate);
        auto fileSize = ifs.tellg();
        if (fileSize <= 0) {
            return nullptr;
        }
        std::vector<int16_t > wholePcm_;
        ifs.seekg(0, std::ios::beg);
        wholePcm_.resize((size_t)fileSize / sizeof(int16_t));
        if (!ifs.read((char*)wholePcm_.data(), fileSize)) {
            return nullptr;
        }
        wholePcm_.shrink_to_fit();

        return std::make_shared<AudioInStub>(wholePcm_);
    }


    static std::shared_ptr<AudioInStub> create(const std::string &filePathOrTag) {
        auto ret = loadFile(filePathOrTag);

        if (ret) {
            return ret;
        }
        
        if (filePathOrTag == "sine") {
            return std::make_shared<AudioInStub>(SineWave(sampleRate, 440, .8, 10));
        }

        return nullptr;
    }
};



/// TODO:
///    - make timestamp
///    - use reliable UDP in weak network
///    - check version (server & client)
///    - display build date and git commit version in GUI and server side


class IWorker {
public:
    virtual ~IWorker() {}
    virtual ReturnType init(
        const boost::property_tree::ptree &configRoot,
        std::function<void(const std::string &, const std::string &)> reportInfo,
        std::function<void(const AudioIoVolume)> reportVolume,
        std::function<void(const bool)> vadReporter
    ) = 0;
};

class Worker : public IWorker {
private:
    CallbackStyleAudioEndpoint *endpoint_ = nullptr;
    AudioDecoder * decoder = nullptr;
    AudioEncoder * encoder = nullptr;
    bool gotoStop_ = false;
    std::shared_ptr<std::thread> netThread_ = nullptr;
    std::function<void(const AudioIoVolume)>  volumeReporter_ = nullptr;
    std::function<void(const bool)>  vadReporter_ = nullptr;
    std::function<void(const uint32_t)> durationReporter_ = nullptr;
    boost::property_tree::ptree configRoot_;
    bool needAec_ = false;

    uint8_t vadCounter_ = 0; // nbActivated
    bool mute_ = false;
    bool needSend_ = true;
    SuckAudioVolume sav;
    std::shared_ptr<std::thread> durationTimer_ = nullptr;
    std::shared_ptr<NetClient> pClient = nullptr;
    uint32_t largestReceivedMediaDataTimestamp_ = 0;
    /*
    RingBuffer micBuffer_;
    */
    std::shared_ptr<AudioInStub> audioInStub_ = nullptr;
    AudioOutBuffer audioOutBuffer_;
    std::shared_ptr<std::vector<int16_t>> audioOutStub_ = nullptr;

    uint32_t sn_sendingAudio_ = 0;
    uint32_t sn_sendingHeartBeat_ = 0;
    uint32_t lastReceivedAudioSn_ = 0;
private:
    bool initCodec();
    void nsAecVolumeVadSend(const short *buffer);
    void decodeOpusAndAecBufferFarend(const NetPacket& netPacket);
    void sendHeartbeat();
public:
    EVC_API Worker();
    EVC_API ~Worker();
    EVC_API virtual ReturnType init(
        const boost::property_tree::ptree &configRoot,
        std::function<void(const std::string &, const std::string &)> reportInfo,
        std::function<void(const AudioIoVolume)> reportVolume,
        std::function<void(const bool)> vadReporter
    ) override;
    
    EVC_API void setDurationReporter(decltype(durationReporter_) __);
    EVC_API void setMute(bool mute);

    EVC_API void asyncStart(std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState);

    EVC_API ReturnType syncStart(std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState);


    ///// TODO: blocked when server down?
    EVC_API void syncStop();
};
