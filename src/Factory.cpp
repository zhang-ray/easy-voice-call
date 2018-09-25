#include "evc/Factory.hpp"
#if 0
#include "evc/Alsa.hpp"


AudioDevice &Factory::create() {
    return (AudioDevice &)(Alsa::get());
}

#endif

// use PortAudio backend
#include "evc/PortAudio.hpp"
AudioDevice &Factory::create() {
    return (AudioDevice &)(PortAudio::get());
}

#include "evc/FdkAacEncoder.hpp"
AudioEncoder &Factory::createAudioEncoder() {
    return (FdkAacEncoder &)(FdkAacEncoder::get());
}
