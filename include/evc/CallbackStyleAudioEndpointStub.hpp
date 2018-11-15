#include "CallbackStyleAudioEndpoint.hpp"
#include <atomic>
#include "Singleton.hpp"
#include <array>
#include <thread>

class CallbackStyleAudioEndpointStub : public CallbackStyleAudioEndpoint{
private:
    std::function<void(const int16_t * inputBuffer, int16_t * outputBuffer, const uint32_t framesPerBuffer) > callbackFunc_ = nullptr;
    std::atomic_bool started_;

    std::vector<int16_t> audioInStub_;
    std::vector<int16_t> &audioOutStub_;
    size_t audioInOffset_ = 0;
    std::shared_ptr<std::thread> theThread_ = nullptr;
private:
    int16_t * getStubMic() {
        int16_t *myMicPointer = nullptr;
        if (audioInOffset_ + blockSize > audioInStub_.size()) {
            // loop
            audioInOffset_ = 0;
        }
        myMicPointer = audioInStub_.data() + audioInOffset_;
        audioInOffset_ += blockSize;
        return myMicPointer;
    }
public:
    static CallbackStyleAudioEndpointStub& get(const std::vector<int16_t> &audioInStub, std::vector<int16_t> &audioOutStub) {
        static CallbackStyleAudioEndpointStub instance(audioInStub, audioOutStub); // local static variable initialization is thread-safe in C++11
        return instance;
    }

    CallbackStyleAudioEndpointStub(const std::vector<int16_t> &audioInStub, std::vector<int16_t> &audioOutStub)
        : audioInStub_(audioInStub)
        , audioOutStub_(audioOutStub)
        , started_(false)
    {

    }
    virtual ReturnType init(const std::function<void(const int16_t * inputBuffer, int16_t * outputBuffer, const uint32_t framesPerBuffer) > callbackFunc) override {
        callbackFunc_ = callbackFunc;
        return 0;
    }


    virtual ReturnType asyncStart() override{
        started_ = true;
        [this]() {
            for (; !started_;) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                auto oldOffset = audioOutStub_.size();
                audioOutStub_.resize(audioOutStub_.size() + blockSize);

                /// TODO: make tempo more precisely
                callbackFunc_(
                    getStubMic(),
                    audioOutStub_.data()+oldOffset,
                    blockSize);
            }
        }();
        return 0;
    }


    virtual ReturnType syncStop() override{
        started_ = false;
        return 0;
    }


    virtual std::tuple<std::string, std::string> getEndpointName() override{
        return{ "", "" };
    }

};