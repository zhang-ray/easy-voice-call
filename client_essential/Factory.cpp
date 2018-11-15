#include "Factory.hpp"
#include "OpusEnc.hpp"
#include "OpusDec.hpp"
#include "CallbackStylePortaudioEndpoint.hpp"
#include "CallbackStyleAudioEndpointStub.hpp"

CallbackStyleAudioEndpoint &Factory::createCallbackStyleAudioEndpoint(const std::vector<int16_t> &audioInStub, std::vector<int16_t> &audioOutStub){
    if (audioInStub.size() > 0) {
        return CallbackStyleAudioEndpointStub::get(audioInStub, audioOutStub);
    }
    
    return (CallbackStyleAudioEndpoint &)(CallbackStylePortaudioEndpoint::get());
}
CallbackStyleAudioEndpoint &Factory::createCallbackStyleAudioEndpoint() {
    return (CallbackStyleAudioEndpoint &)(CallbackStylePortaudioEndpoint::get());
}

AudioEncoder &Factory::createAudioEncoder() {
    return (AudioEncoder &)(OpusEnc::get());
}

AudioDecoder &Factory::createAudioDecoder() {
    return (AudioDecoder &)(OpusDec::get());
}
