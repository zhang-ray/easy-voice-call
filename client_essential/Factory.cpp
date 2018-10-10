#include "evc/Factory.hpp"
#if 0
#include "evc/Alsa.hpp"


AudioDevice &Factory::create() {
    return (AudioDevice &)(Alsa::get());
}

#endif


#ifdef __linux__
#include "evc/Alsa.hpp"
AudioDevice &Factory::create() {
    return (AudioDevice &)(Alsa::get());
}
#else
#include "evc/PortAudio.hpp"
AudioDevice &Factory::create() {
    return (AudioDevice &)(PortAudio::get());
}
#endif

#include "evc/OpusEnc.hpp"
AudioEncoder &Factory::createAudioEncoder() {
    return (AudioEncoder &)(OpusEnc::get());
}

#include "evc/OpusDec.hpp"
AudioDecoder &Factory::createAudioDecoder() {
    return (AudioDecoder &)(OpusDec::get());
}
