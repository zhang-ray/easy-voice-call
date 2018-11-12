#include "evc/Factory.hpp"
#include "evc/OpusEnc.hpp"
#include "evc/OpusDec.hpp"
#include "evc/CallbackStylePortaudioEndpoint.hpp"

CallbackStyleAudioEndpoint & Factory::createCallbackStyleAudioEndpoint() {
    return (CallbackStyleAudioEndpoint &)(CallbackStylePortaudioEndpoint::get());
}


AudioEncoder &Factory::createAudioEncoder() {
    return (AudioEncoder &)(OpusEnc::get());
}

AudioDecoder &Factory::createAudioDecoder() {
    return (AudioDecoder &)(OpusDec::get());
}
