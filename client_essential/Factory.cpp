#include "Factory.hpp"
#include "OpusEnc.hpp"
#include "OpusDec.hpp"
#include "CallbackStylePortaudioEndpoint.hpp"

CallbackStyleAudioEndpoint & Factory::createCallbackStyleAudioEndpoint() {
    return (CallbackStyleAudioEndpoint &)(CallbackStylePortaudioEndpoint::get());
}


AudioEncoder &Factory::createAudioEncoder() {
    return (AudioEncoder &)(OpusEnc::get());
}

AudioDecoder &Factory::createAudioDecoder() {
    return (AudioDecoder &)(OpusDec::get());
}
