#include "CallbackStylePortaudioEndpoint.hpp"
#include "CallbackStyleAudioEndpointStub.hpp"

CallbackStyleAudioEndpoint & CallbackStyleAudioEndpoint::create(
    bool realDevice
) {
    if (realDevice) {
        return CallbackStylePortaudioEndpoint::get();
    }
    else {
        return CallbackStyleAudioEndpointStub::get();
    }
}
