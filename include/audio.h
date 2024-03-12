#ifndef AUDIO_H
#define AUDIO_H

#include <alsa/asoundlib.h>


typedef struct PPM_voice_s{
    int counter;
    int period;
    int waveform;
    unsigned char out;
}PPM_voice;

typedef struct PIM_channel_s{
    unsigned char out;
    int duty_cycle;
    int freqs[N_VOICES];
    PPM_voice voices[N_VOICES];
}PIM_channel;

typedef struct pcm_interface_s{
    snd_pcm_t *pcm;
    PIM_channel *channels[N_CHANNELS];
}pcm_interface;

snd_pcm_t* pcm_start(void);
void pcm_write(snd_pcm_t *pcm, unsigned char *sample);
void pcm_stop(snd_pcm_t *pcm);
PIM_channel* channel_start(int duty_cycle);
int toggle_PPM_voice(PIM_channel *channel, int status, int freq);
void *hardware_out(void *arg);

#endif