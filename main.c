#include <stdio.h>
#include <alsa/asoundlib.h>

#define SAMPLE_RATE 44100
#define AMPLITUDE 30

static snd_pcm_t *pcm = NULL;
static unsigned char buffer[SAMPLE_RATE * 5];
static unsigned char *ptr = buffer;

typedef struct voice_s{
    unsigned char val, status;
    int freq, off, period, counter;
    float duty_cycle;
}voice;

typedef struct channel_s{
    voice voices[3];
    int freqs[3];
}channel;

void toggle_voice(voice *voice, int status, int freq, float duty_cycle){
    voice->val = 0;
    voice->counter = 0;
    voice->status = status;
    voice->freq = freq;
    voice->duty_cycle = duty_cycle;
    voice->period = SAMPLE_RATE / freq;
    voice->off = (int)(voice->period * duty_cycle);
}

unsigned char *square_wave(unsigned char *buffer, int freq, int duration){
    int samples = (int)(SAMPLE_RATE * duration / 1000);
    unsigned char val = 0;
    int period = SAMPLE_RATE / 880;

    for (int i = 0; i < samples; i++){
        buffer[i] = val * AMPLITUDE;
        if (i % period == period - 1){
            val = !val;
        }
        printf("%d %d\n", buffer[i], i);
    }
}

void pulse_wave(int freq, float duty_cycle){

}

int main(void){
    int err;

    if ((err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    if ((err = snd_pcm_set_params(pcm,
                      SND_PCM_FORMAT_U8,
                      SND_PCM_ACCESS_RW_INTERLEAVED,
                      1,
                      SAMPLE_RATE,
                      1,
                      20000)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    channel channel;

    toggle_voice(&channel.voices[1], 1, 261, 0.07);
    toggle_voice(&channel.voices[0], 1, 329, 0.09);
    toggle_voice(&channel.voices[2], 1, 392, 0.11);
    printf("%d %d\n", channel.voices[0].period, channel.voices[0].off);
    unsigned char vals[3] = {0, 0, 0};
    while(1){
        unsigned char val = 0;
        for(int i = 0; i < 3; i++){
            if(channel.voices[i].counter++ < channel.voices[i].off){
                vals[i] = 1;
            }
            else if(channel.voices[i].counter < channel.voices[i].period){
                vals[i] = 0;
            }
            else{
                channel.voices[i].counter = 0;
            }
            val = val | vals[i];
        }
        printf("%d\n", val);
        val = val * AMPLITUDE;
        snd_pcm_writei(pcm, &val, 1);
    }

    unsigned char buffer[(int)(SAMPLE_RATE * 5)];
    square_wave(buffer, 440, 5000);
    printf("%d", sizeof(buffer));
    snd_pcm_writei(pcm, buffer, sizeof(buffer));
}