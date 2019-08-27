#include "OpusEnc.hpp"
#include "OpusDec.hpp"

AudioDecoder &AudioDecoder::create() {
    return (AudioDecoder &)(OpusDec::get());
}

AudioEncoder &AudioEncoder::create() {
    return (AudioEncoder &)(OpusEnc::get());
}