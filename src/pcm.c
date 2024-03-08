#include <stdio.h>
#include <alsa/asoundlib.h>
#include "../include/params.h"
#include "../include/pcm.h"

snd_pcm_t* pcm_start(void){
    snd_pcm_t *pcm;
    int err;

    if ((err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    if ((err = snd_pcm_set_params(pcm,
                    SND_PCM_FORMAT_U8,
                    SND_PCM_ACCESS_RW_INTERLEAVED,
                    N_CHANNELS,
                    SAMPLE_RATE,
                    1,
                    20000)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    return pcm;
}

void pcm_stop(snd_pcm_t *pcm){
    snd_pcm_close(pcm);
}

