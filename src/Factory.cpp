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
