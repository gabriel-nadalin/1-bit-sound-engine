#include <stdio.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/params.h"
#include "../include/pcm.h"


static snd_pcm_t *pcm = NULL;


typedef struct voice_s{
    unsigned char val, status;
    int freq, off, period, counter;
    float duty_cycle;
}voice;

typedef struct channel_s{
    voice voices[N_VOICES];
}channel;

channel channel_ptr, chan;

void toggle_voice(channel *channel, int status, int freq, float duty_cycle){
    voice *voice_ptr;
    for (int i = 0; i < N_VOICES; i++){
        voice_ptr = &channel->voices[i];
        if(status == 0 && voice_ptr->freq == freq){
            voice_ptr->status = status;
            return;
        }
        else if(status == 1 && voice_ptr->status == 0){
            voice_ptr->val = 0;
            voice_ptr->counter = 0;
            voice_ptr->status = status;
            voice_ptr->freq = freq;
            voice_ptr->duty_cycle = duty_cycle;
            voice_ptr->period = SAMPLE_RATE / freq;
            voice_ptr->off = (int)(voice_ptr->period * duty_cycle);
            return;
        }
    }
}

void *play_channel_thread(void *arg){
    channel *channel_ptr = (channel *)arg;
    unsigned char vals[N_VOICES] = {0};
    while(1){
        unsigned char val = 0;
        for(int i = 0; i < N_VOICES; i++){
            if(channel_ptr->voices[i].status){
                if(channel_ptr->voices[i].counter++ < channel_ptr->voices[i].off){
                    vals[i] = 1;
                }
                else if(channel_ptr->voices[i].counter < channel_ptr->voices[i].period){
                    vals[i] = 0;
                }
                else{
                    channel_ptr->voices[i].counter = 0;
                }
            }
            val = val | vals[i];
        }
        // printf("%d\n", val);
        // printf("\b%d", val);
        val = val * AMPLITUDE;
        snd_pcm_writei(pcm, &val, 1);
    }
    return NULL;
}


struct args{
    FILE *file;
    channel *channel_ptr;
}args1, args2;

void *play_file(void *arg){
    struct args *args = (struct args*)arg;
    FILE *file = args->file;
    channel *chn = args->channel_ptr;
    int count = 0;

    while(1){
        int status, freq, delay;
        fscanf(file, "%d %d %d", &status, &freq, &delay);
        usleep(delay);
        if(status){ printf("%d %d %d %d\n", count, status, freq, delay);}
        toggle_voice(chn, status, freq, 0.15);
        count++;
    }
}

int main(void){
    pcm = pcm_start();

    channel_ptr.voices[0].status = 0;
    channel_ptr.voices[1].status = 0;
    channel_ptr.voices[2].status = 0;

    chan.voices[0].status = 0;
    chan.voices[1].status = 0;
    chan.voices[2].status = 0;

    pthread_t thread;

    if (pthread_create(&thread, NULL, play_channel_thread, &channel_ptr) != 0) {
        fprintf(stderr, "Error creating thread\n");
        exit(EXIT_FAILURE);
    }

    FILE *f1, *f2;
    f1 = fopen("../examples/badapple_nomico5.txt", "r");
    f2 = fopen("../examples/badapple_nomico3.txt", "r");

    args1.file = f1;
    args1.channel_ptr = &channel_ptr;
    args2.file = f2;
    args2.channel_ptr = &chan;
    pthread_t threads[2];

    play_file(&args1);
    // pthread_create(&threads[1], NULL, play_file, &args2);
    

    // toggle_voice(&channel_ptr, 1, 261, 0.1);
    // sleep(1);
    // toggle_voice(&channel_ptr, 1, 329, 0.12);
    // sleep(1);
    // toggle_voice(&channel_ptr, 1, 392, 0.14);

    pthread_join(thread, NULL); // Wait for the play_channel_thread to finish (which it never will)
    return 0;

}