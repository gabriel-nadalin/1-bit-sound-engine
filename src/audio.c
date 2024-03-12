#include <stdio.h>
#include <alsa/asoundlib.h>
#include "../include/params.h"
#include "../include/audio.h"

snd_pcm_t *pcm_start(void){
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

void pcm_write(snd_pcm_t *pcm, unsigned char *sample){
    snd_pcm_writei(pcm, &sample, 1);
}

void pcm_stop(snd_pcm_t *pcm){
    snd_pcm_close(pcm);
}

PIM_channel *channel_start(int duty_cycle){
    PIM_channel *channel = malloc(sizeof(PIM_channel));
    channel->out = 0;
    channel->duty_cycle = duty_cycle;
    for(int i = 0; i < N_VOICES; i++){
        channel->freqs[i] = 0;
    }
    return channel;
}

void channel_stop(PIM_channel *channel){
    free(channel);
}

int toggle_PPM_voice(PIM_channel *channel, int status, int freq){
    int index = -1;
    if (status){
        for(int i = 0; i < N_VOICES; i++){
            if(channel->freqs[i] == freq){
                return -1;
            }
            if(channel->freqs[i] == 0){
                index = i;
            }
        }
        if(index == -1){
            return -2;
        }
        channel->freqs[index] = freq;
        PPM_voice *voice = &channel->voices[index];
        voice->counter = 0;
        voice->period = (int)(SAMPLE_RATE / freq);
        voice->waveform = (int)(voice->period * channel->duty_cycle / 1000);
        voice->out = 0;
    }
    else{
        for(int i = 0; i < N_VOICES; i++){
            if(channel->freqs[i] == freq){
                index = i;
            }
        }
        if(index == -1){
            return -3;
        }
        channel->freqs[index] = 0;
    }
    return 0;
}

void *hardware_out(void *arg){
    pcm_interface *args = (pcm_interface *)arg;
    snd_pcm_t *pcm = args->pcm;
    PIM_channel *channels = args->channels[0];
    unsigned char vals[N_VOICES] = {0};
    while(1){
        unsigned char val = 0;
        for(int i = 0; i < N_VOICES; i++){
            if(channels->freqs[i]){
                if(channels->voices[i].counter++ == channels->voices[i].period){
                    channels->voices[i].counter = 0;
                }
                else if(channels->voices[i].counter < channels->voices[i].waveform){
                    vals[i] = 1;
                }
                else{
                    vals[i] = 0;
                }
            }
            val = val | vals[i];
        }
        val = val * AMPLITUDE;
        snd_pcm_writei(pcm, &val, 1);
    }
}