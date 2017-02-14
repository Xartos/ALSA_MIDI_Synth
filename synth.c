
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <time.h>
#include "alsa-lib-1.1.3/include/asoundlib.h"
#ifndef _STRUCT_TIMEVAL
#  define _STRUCT_TIMEVAL
#endif
#include "synth.h"
#include "oscillator.h"
#include "oscList.h"
#include "notes.h"

snd_pcm_t* setupSnd(snd_pcm_t* handle){
    int err;

    if((err = snd_pcm_open(&handle,"default",SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(-1);
    }

    if ((err = snd_pcm_set_params(  handle,
                                    SND_PCM_FORMAT_S16_LE,
                                    SND_PCM_ACCESS_RW_INTERLEAVED,
                                    1,
                                    44100,
                                    1,
                                    500000)) < 0) {  
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    return handle;
}

int main(int argc, char* argv[]){

    int freq = notes[5][0];

    if(argc == 3){
        int o = atoi(argv[1]);
        int n = atoi(argv[2]);

        if(!(o < 0 || o > 8 || n < 0 || n > 11)){
            freq = notes[o][n];
        }
    }
    
    /* Start oscillators */
    oscList* list = createOscList();

    for(int i = 0; i < OSC_NR; i++){
        createAddOsc(list, OSC_TYPE_SINE, freq - (i * 392));
    }
    
    oscListElm* oscElem = getOsc(list, 0);
    oscillator** oscs = malloc(sizeof(oscillator*) * OSC_NR);;
    oscs[0] = oscElem->osc;
    oscs[1] = oscElem->next->osc;

    int16_t** oscBuffers = malloc(sizeof(int16_t*) * OSC_NR);
    int16_t*  mainBuffer = malloc(sizeof(int16_t) * BUFFERSIZE);

    if(oscBuffers == NULL || mainBuffer == NULL){
        perror("allocating memory for synth buffer list");
        exit(-1);
    }

    for(int i = 0; i < OSC_NR; i++){

        oscBuffers[i] = malloc(sizeof(int16_t) * BUFFERSIZE);

        if(oscBuffers[i] == NULL){
            perror("allocating memory for synth buffer");
            exit(-1);
        }
    }

    //Init sound
    snd_pcm_t *handle = NULL;
    snd_pcm_sframes_t frames;
    handle = setupSnd(handle);

    long dur = 0;

    printf("\n");
    printf("*****************\n");
    printf("*   Synth v1.0  *\n");
    printf("*****************\n");
    printf("\n");
    printf("Playing Blame It...\n");

    while(1){

        if(dur == 20){
            setFreq(oscs[0], notes[4][9]);
            setFreq(oscs[1], notes[2][9]);
        }else if(dur == 40){
            setFreq(oscs[0], 0);
            setFreq(oscs[1], 0);
        }else if(dur == 100){
            setFreq(oscs[0],  notes[4][9]);
            setFreq(oscs[1],  notes[2][9]);
        }else if(dur == 110){
            setFreq(oscs[0], notes[4][11]);
            setFreq(oscs[1], notes[2][11]);
        }else if(dur == 120){
            setFreq(oscs[0], notes[5][0]);
            setFreq(oscs[1], notes[3][0]);
        }else if(dur == 130){
            setFreq(oscs[0], notes[4][11]);
            setFreq(oscs[1], notes[2][11]);
        }else if(dur == 140){
            setFreq(oscs[0], notes[4][9]);
            setFreq(oscs[1], notes[2][9]);
        }else if(dur == 150){
            setFreq(oscs[0], notes[4][11]);
            setFreq(oscs[1], notes[2][11]);
        }else if(dur == 160){
            setFreq(oscs[0], notes[5][0]);
            setFreq(oscs[1], notes[3][0]);
            dur = 0;
        }
        
        //Clear main buffer
        for (int j = 0 ; j < BUFFERSIZE ; j++) {
            mainBuffer[j] = 0;
        }

        for(int i = 0; i < OSC_NR; i++){
            pthread_mutex_lock(&(oscs[i]->lock));
            
            while(!oscs[i]->done){
                pthread_cond_wait(&(oscs[i]->cond), &(oscs[i]->lock));
            }
            
            //Get buffer
            memcpy(oscBuffers[i], oscs[i]->buffer, BUFFERSIZE * sizeof(int16_t));
            oscs[i]->done = 0;
            pthread_cond_signal(&(oscs[i]->cond));
            pthread_mutex_unlock(&(oscs[i]->lock));
        }


        for (int j = 0 ; j < BUFFERSIZE ; j++) {
            int64_t out = 0;
            for(int i = 0; i < OSC_NR; i++){
                out += oscBuffers[i][j];
            }

            mainBuffer[j] = (int16_t)(out / OSC_NR);

            
            //printf("%d;\n", mainBuffer[j]);
        }

        //Send buffer
        frames = snd_pcm_writei(handle, mainBuffer, BUFFERSIZE);
        if (frames < 0){
            frames = snd_pcm_recover(handle, frames, 0);
        }
        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            break;
        }
        if (frames > 0 && frames < BUFFERSIZE){
            printf("Short write (expected %d, wrote %li)\n", BUFFERSIZE, frames);
        }

        dur++;
    }
    
    snd_pcm_close(handle);
    return 0;
}
