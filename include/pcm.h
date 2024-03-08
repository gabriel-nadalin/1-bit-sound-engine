#ifndef PCM_H
#define PCM_H

#include <alsa/asoundlib.h>

snd_pcm_t* pcm_start(void);

void pcm_stop(snd_pcm_t *pcm);

#endif