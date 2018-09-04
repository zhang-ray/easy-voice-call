
#include <iostream>

#include <alsa/asoundlib.h>
#define error printf
static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;


static snd_pcm_t *handle;
static snd_pcm_t *defaultHandle = nullptr;


static void device_list(void)
{
    snd_ctl_t *handle;
    int card, err, dev, idx;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    card = -1;
    if (snd_card_next(&card) < 0 || card < 0) {
        return;
    }
    printf("**** List of %s Hardware Devices****\n",
           snd_pcm_stream_name(stream));
    while (card >= 0) {
        char name[32];
        sprintf(name, "hw:%d", card);
        if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
            error("control open (%i): %s", card, snd_strerror(err));
            goto next_card;
        }
        if ((err = snd_ctl_card_info(handle, info)) < 0) {
            error("control hardware info (%i): %s", card, snd_strerror(err));
            snd_ctl_close(handle);
            goto next_card;
        }


        dev = -1;
        while (1) {
            unsigned int count;
            if (snd_ctl_pcm_next_device(handle, &dev)<0)
                error("snd_ctl_pcm_next_device");
            if (dev < 0)
                break;
            snd_pcm_info_set_device(pcminfo, dev);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo, stream);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                if (err != -ENOENT)
                    error("control digital audio info (%i): %s", card, snd_strerror(err));
                continue;
            }
            printf("card %i: %s [%s], device %i: %s [%s]\n",
                card, snd_ctl_card_info_get_id(info), snd_ctl_card_info_get_name(info),
                dev,
                snd_pcm_info_get_id(pcminfo),
                snd_pcm_info_get_name(pcminfo));
            count = snd_pcm_info_get_subdevices_count(pcminfo);
            printf("  Subdevices: %i/%i\n",
                snd_pcm_info_get_subdevices_avail(pcminfo), count);
            for (idx = 0; idx < (int)count; idx++) {
                snd_pcm_info_set_subdevice(pcminfo, idx);
                if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                    error("control digital audio playback info (%i): %s", card, snd_strerror(err));
                } else {
                    printf("  Subdevice #%i: %s\n",
                        idx, snd_pcm_info_get_subdevice_name(pcminfo));
                }
            }
        }


        snd_ctl_close(handle);
    next_card:
        if (snd_card_next(&card) < 0) {
            error("snd_card_next");
            break;
        }
    }
}



int main(void){

    // init local device
    // call remote client
    // main loop : writei_func@aplay.c
    // bye

    device_list();
    auto err = snd_pcm_open(&defaultHandle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        error("audio open error: %s", snd_strerror(err));
        return 1;
    }
    snd_pcm_info_t *info;
    snd_pcm_info_alloca(&info);
    if ((err = snd_pcm_info(defaultHandle, info)) < 0) {
        error("info error: %s", snd_strerror(err));
        return 1;
    }


//snd_pcm_writei

    return 0;
}
