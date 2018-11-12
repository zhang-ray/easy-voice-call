#pragma once
#include <vector>
#include <string>

#include <alsa/asoundlib.h>

#include "Singleton.hpp"


class Alsa : public AudioDevice, public Singleton<Alsa>
{
    snd_pcm_t *handlerCapture_ = nullptr;
    snd_pcm_t *handlerPlay_ = nullptr;

    unsigned int channelsPerFrame_ = 1;
    unsigned int bitsPerChannel_  = 16;
    unsigned int bytesPerFrame_ = channelsPerFrame_ * bitsPerChannel_ / 8;

public:
    ~Alsa() {
        if(handlerCapture_){
            snd_pcm_close(handlerCapture_);
        }
        if(handlerPlay_){
            snd_pcm_close(handlerPlay_);
        }
    }

    ReturnType init(std::string &micInfo, std::string &spkInfo) override {
        auto configureDevice = [this](bool isInput) -> ReturnType {
            snd_pcm_t *handle = isInput ? handlerCapture_ : handlerPlay_;

            unsigned int nSampleRate = sampleRate;
            unsigned int nFragments  = 3;
            snd_pcm_uframes_t nFrames = blockSize / bytesPerFrame_ * nFragments;
            int err = 0;
            snd_pcm_hw_params_t *hw_params;

            if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
                return "snd_pcm_hw_params_malloc != 0";
            }

            if ((err = snd_pcm_hw_params_any(handle, hw_params)) < 0) {
                return "snd_pcm_hw_params_any != 0";
            }

            if ((err = snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
                return "snd_pcm_hw_params_set_access != 0";
            }

            snd_pcm_format_t format = bitsPerChannel_ == 16 ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_UNKNOWN;
            if ((err = snd_pcm_hw_params_set_format(handle, hw_params, format)) < 0) {
                return "snd_pcm_hw_params_set_format != 0";
            }
            if ((err = snd_pcm_hw_params_set_rate_near(handle, hw_params, &nSampleRate, 0)) < 0) {
                return "snd_pcm_hw_params_set_rate_near != 0";
            }
            if ((err = snd_pcm_hw_params_set_channels(handle, hw_params, channelsPerFrame_)) < 0) {
                return "snd_pcm_hw_params_set_channels != 0";
            }

            if ((err = snd_pcm_hw_params_set_periods_near(handle, hw_params, &nFragments, 0)) < 0) {
                return "snd_pcm_hw_params_set_periods_near != 0";
            }

            if ((err = snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &nFrames)) < 0) {
                return "snd_pcm_hw_params_set_buffer_size_near != 0";
            }

            if ((err = snd_pcm_hw_params(handle, hw_params)) < 0) {
                return "snd_pcm_hw_params != 0";
            }

            return 0;
        };


        const char *szDeviceID = "default";

        int err = 0;
        if ((err = snd_pcm_open(&handlerCapture_, szDeviceID, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
            return "snd_pcm_open != 0";
        }

        auto retInput = configureDevice(true);
        if (!retInput){
            return retInput;
        }

        if ((err = snd_pcm_open(&handlerPlay_, szDeviceID, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
            return "snd_pcm_open != 0";
        }


        auto retOutput = configureDevice(false);
        if (!retOutput) {
            return retOutput;
        }

        micInfo=szDeviceID;
        spkInfo=szDeviceID;

        return 0;
    }



    ReturnType read(std::vector<short> &buffer) override {
        auto pBuffer = (char *)(buffer.data());
        int nBytes = buffer.size() *2;
        int nFrames = nBytes / bytesPerFrame_;

        while (nFrames > 0) {
            int nRead = snd_pcm_readi(handlerCapture_, pBuffer, nFrames);

            if (nRead == -EAGAIN || (nRead >= 0 && nRead < nFrames)) {
                snd_pcm_wait(handlerCapture_, 1000);
            } else if (nRead == -EPIPE) {
                if(snd_pcm_prepare(handlerCapture_) < 0)
                    return -1;
            } else if (nRead == -ESTRPIPE) {
                int err;
                while ((err = snd_pcm_resume(handlerCapture_)) == -EAGAIN)
                    sleep(1);
                if (err < 0) {
                    if (snd_pcm_prepare(handlerCapture_) < 0) {
                        return -1;
                    }
                }
            } else if (nRead < 0) {
                return -1;
            }

            if (nRead > 0) {
                nFrames -= nRead;
                pBuffer += nRead * bytesPerFrame_;
            }

        }

        return 0;
    }


    ReturnType write(const std::vector<short> &buffer) override {
        auto pBuffer = (const char *)(buffer.data());
        int nBytes = buffer.size()*2;

        if(!handlerPlay_)
            return -1;

        int nFrames = nBytes / bytesPerFrame_;
        int nTotal = 0;
        while (nFrames > 0) {
            int nWritten = snd_pcm_writei(handlerPlay_, pBuffer, nFrames);

            if (nWritten == -EAGAIN || (nWritten >= 0 && nWritten < nFrames)) {
                snd_pcm_wait(handlerPlay_, 1000);
            }
            else if (nWritten == -EPIPE) {
                if(snd_pcm_prepare(handlerPlay_) < 0)
                    return -1;
            } else if (nWritten == -ESTRPIPE) {
                int err;
                while ((err = snd_pcm_resume(handlerPlay_)) == -EAGAIN)
                    sleep(1);
                if (err < 0) {
                    if (snd_pcm_prepare(handlerPlay_) < 0) {
                        return -1;
                    }
                }
            }
            else if (nWritten < 0) {
                return -1;
            }

            if (nWritten > 0) {
                nTotal += nWritten;
                nFrames -= nWritten;
                pBuffer += nWritten * bytesPerFrame_;
            }
        }

        return 0;
    }
};


