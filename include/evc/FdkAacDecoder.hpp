#pragma once

#include "ReturnType.hpp"
#include "evc/AudioDecoder.hpp"
#include <vector>

extern "C"{
#include <fdk-aac/aacdecoder_lib.h>
}

class FdkAacDecoder final : public AudioDecoder, public Singleton<FdkAacDecoder> {
private:
    HANDLE_AACDECODER decoder_ = nullptr;

public:
    virtual ReturnType reInit() override {
        AAC_DECODER_ERROR err;

        if (decoder_){
            aacDecoder_Close(decoder_);
        }

        decoder_ = aacDecoder_Open(TT_MP4_ADTS, 1);
        // if ((err = aacDecoder_ConfigRaw(decoder_, &avctx->extradata,
        //                                 &avctx->extradata_size)) != AAC_DEC_OK) {
        //     av_log(avctx, AV_LOG_ERROR, "Unable to set extradata\n");
        //     return AVERROR_INVALIDDATA;
        // }

        // if ((ret = get_stream_info(avctx)) < 0)
        //     goto end;

        if ((err = aacDecoder_SetParam(decoder_, AAC_CONCEAL_METHOD, 0)) != AAC_DEC_OK) {
            // av_log(avctx, AV_LOG_ERROR, "Unable to set error concealment method\n");
            return err;
        }
        return 0;
    }

    virtual ReturnType decode(const std::vector<char> &encodedData, std::vector<char> &pcmData) override {
        int ret;
        AAC_DECODER_ERROR err;
        uint valid = encodedData.size();
        
        auto buffers = encodedData.data();
        auto sizes = encodedData.size();
        printf("\nsize=%d\n", encodedData.size());
        err = aacDecoder_Fill(decoder_, (UCHAR**) (&buffers), (const UINT *) (&sizes), &valid);
        if (err != AAC_DEC_OK) {
            // av_log(avctx, AV_LOG_ERROR, "aacDecoder_Fill() failed: %x\n", err);

            std::cout << __FILE__ << ":" <<  __LINE__ << " err=" <<  err << std::endl;
            return err;
        }

        auto info = aacDecoder_GetStreamInfo(decoder_);

        // if (info->sampleRate <= 0) {
        //     av_log(avctx, AV_LOG_ERROR, "Stream info not initialized\n");
        //     return AVERROR_UNKNOWN;
        // }
        // avctx->sample_rate = info->sampleRate;
        // avctx->frame_size  = info->frameSize;

            std::cout << __FILE__ << ":" <<  __LINE__   << std::endl;
        std::cout << info->sampleRate << " " << info->frameSize << std::endl;

        uint8_t tmpData[1<<14];
        err = aacDecoder_DecodeFrame(decoder_, (INT_PCM*)tmpData, (1<<14) / sizeof(INT_PCM), 0);
        if (err == AAC_DEC_NOT_ENOUGH_BITS) {
            std::cout << __FILE__ << ":" <<  __LINE__ << " err=" <<  err << std::endl;
            return err;
        }
        if (err != AAC_DEC_OK) {
            // av_log(avctx, AV_LOG_ERROR, "aacDecoder_DecodeFrame() failed: %x\n", err);
            // ret = AVERROR_UNKNOWN;
            std::cout << __FILE__ << ":" <<  __LINE__ << " err=" <<  err << std::endl;
            return err;
        }

        // if ((ret = get_stream_info(avctx)) < 0)
        //     goto end;
        auto stream_info = aacDecoder_GetStreamInfo(decoder_);

        // if (info->sampleRate <= 0) {
        //     av_log(avctx, AV_LOG_ERROR, "Stream info not initialized\n");
        //     return AVERROR_UNKNOWN;
        // }
        // avctx->sample_rate = info->sampleRate;
        // avctx->frame_size  = info->frameSize;

            std::cout << __FILE__ << ":" <<  __LINE__   << std::endl;
        std::cout << stream_info->sampleRate << " " << info->frameSize << std::endl;

    //     frame->nb_samples = avctx->frame_size;

    //     if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
    //         goto end;

    //     memcpy(frame->extended_data[0], s->decoder_buffer,
    //         avctx->channels * avctx->frame_size *
    //         av_get_bytes_per_sample(avctx->sample_fmt));

    //     *got_frame_ptr = 1;
    //     ret = avpkt->size - valid;

    // end:
    //     return ret;
    return 0;
    }
    
    virtual ~FdkAacDecoder(){
        if (decoder_){
            aacDecoder_Close(decoder_);
        }
        
    }
};

