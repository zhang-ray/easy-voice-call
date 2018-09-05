#include "evc/Factory.hpp"
#include "evc/Alsa.hpp"


AudioDevice &Factory::create() {
    return (AudioDevice &)(Alsa::get());
}
