#pragma once


#include <vector>
#include <string>

#include "Singleton.hpp"
#include "AudioDevice.hpp"

#include <portaudio.h>
#include <iostream>

#ifdef WIN32
#pragma comment (lib, "portaudio_x86.lib")
#endif


class PortAudio : public AudioDevice, public Singleton<PortAudio> {
private:
    PaStream *stream_ = nullptr;
public:
    virtual ReturnType init() override{
        auto err = Pa_Initialize();
        if( err != paNoError ){
            return Pa_GetErrorText( err );
        }

        #define SAMPLE_RATE (48000)

        /* Open an audio I/O stream. */
        err = Pa_OpenDefaultStream(
            &stream_,
            1, 1,
            paInt16, SAMPLE_RATE,
            1<<7,
            nullptr, nullptr);
        if( err != paNoError ) {
            // error
            return err;
        }

        err = Pa_StartStream(stream_);
        if( err != paNoError ) {
            // error
            return err;
        }
        return 0;
    }
    virtual int read(char *pBuffer, int nBytes) override {
        auto err = Pa_ReadStream(stream_, (void*)pBuffer, nBytes);
        if (err != paNoError){
            std::cout << __FILE__ << ":" <<  __LINE__ << " :" << Pa_GetErrorText( err ) << std::endl;
            return err;
        }
        return nBytes;
    }
    virtual int write(const char *pBuffer, int nBytes) override {
        auto err = Pa_WriteStream(stream_, (void*)pBuffer, nBytes);
        if (err != paNoError){
            std::cout << __FILE__ << ":" <<  __LINE__ << " :" << Pa_GetErrorText( err ) << std::endl;
            return err;
        }
        return nBytes;
    }

    ~PortAudio(){
        auto err = Pa_StopStream(stream_);
        if( err != paNoError ){
            //printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        }
        err = Pa_Terminate();
        if( err != paNoError ){
            //printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        }
    }

};
