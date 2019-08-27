#include "CallbackStyleAudioEndpoint.hpp"
#include <atomic>
#include "Singleton.hpp"
#include <array>
#include <thread>

class CallbackStyleAudioEndpointStub : public CallbackStyleAudioEndpoint{
private:
    std::function<void(const int16_t * inputBuffer, int16_t * outputBuffer, const uint32_t framesPerBuffer) > callbackFunc_ = nullptr;
    std::atomic_bool started_;

    std::array<int16_t, blockSize> audioInStub_;
    std::array<int16_t, blockSize> audioOutStub_;
    std::shared_ptr<std::thread> theThread_ = nullptr;
public:
    static CallbackStyleAudioEndpointStub& get() {
        static CallbackStyleAudioEndpointStub instance; // local static variable initialization is thread-safe in C++11
        return instance;
    }

    CallbackStyleAudioEndpointStub()
        : started_(false)
    {

    }
    virtual ReturnType init(const std::function<void(const int16_t * inputBuffer, int16_t * outputBuffer, const uint32_t framesPerBuffer) > callbackFunc) override {
        callbackFunc_ = callbackFunc;
        return 0;
    }


    virtual ReturnType asyncStart() override{
        started_ = true;
        [this]() {
            for (; started_;) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                /// TODO: make tempo more precisely
                callbackFunc_(
                    audioInStub_.data(),
                    audioOutStub_.data(),
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