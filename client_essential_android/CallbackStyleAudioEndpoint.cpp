#include "CallbackStyleAudioEndpointStub.hpp"


#ifdef __ANDROID__
#include "CallbackStyleAAudioEndpoint.hpp"
#else
#include "CallbackStylePortaudioEndpoint.hpp"
#endif

CallbackStyleAudioEndpoint & CallbackStyleAudioEndpoint::create(
    bool realDevice
) {
    if (realDevice) {
#ifdef __ANDROID__
        return CallbackStyleAAudioEndpoint::get();
#else
        return CallbackStylePortaudioEndpoint::get();
#endif
    }
    else {
        return CallbackStyleAudioEndpointStub::get();
    }
}
