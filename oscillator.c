#include "oscillator.h"
#include "synth.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define M_PI 3.14159265358979323846264338327
#define TWO_PI (2.0 * M_PI)

static void* oscillatorWork(void* arg){
    oscillator* osc = (oscillator*) arg;
    int currType;
    int currFreq;
    
    pthread_mutex_lock(&(osc->lock));
    while(!osc->exit){
        //Wait for synth to empty buffer
        while(osc->done && !osc->exit){
            pthread_cond_wait(&(osc->cond), &(osc->lock));
        }
        
        if (osc->exit){
            break;
        }
        currType = osc->type;
        currFreq = osc->freq;
        pthread_mutex_unlock(&(osc->lock));

        int sync = osc->sync;
        int last;
        int sampPerPeriod = 0;
        int mod = 0;

        if(currFreq == 0){
            currType = -1;
        }else {
            sampPerPeriod = RATE / currFreq;
            mod = sampPerPeriod / 2;
        }
        
        /* Fill buffer with stuff */
        switch(currType){
            case OSC_TYPE_SINE:;
                double cost = TWO_PI * currFreq / (double)RATE;
                
                for (int i = 0 ; i < BUFFERSIZE ; i++) {
                    last = (i + sync);
                    osc->buffer[i] = (int16_t)(INT16_MAX * sin(last * cost));
                }
                
                osc->sync = (last + 1);
                break;
            case OSC_TYPE_SAW:
                for (int i = 0 ; i < BUFFERSIZE ; i++) {
                    last = (i + sync);
                    int inc = last % sampPerPeriod;

                    osc->buffer[i] = INT16_MIN + ((2 * inc * INT16_MAX)  / sampPerPeriod);
                }
                
                osc->sync = (last + 1) % sampPerPeriod;
                break;
            case OSC_TYPE_SQUARE:
                for (int i = 0 ; i < BUFFERSIZE ; i++) {
                    last = (i + sync);
                    osc->buffer[i] = ((last % sampPerPeriod) <= mod) ? (INT16_MAX - 100) : (INT16_MIN + 100);
                }
                osc->sync = (last + 1) % sampPerPeriod;
                break;
            case OSC_TYPE_TRIANGLE:
                for (int i = 0 ; i < BUFFERSIZE ; i++) {
                    last = (i + sync);
                    int inc = last % sampPerPeriod;
                    
                    osc->buffer[i] = INT16_MIN + abs(2 * (((2 * inc * INT16_MAX) / sampPerPeriod) - INT16_MAX));
                }
                osc->sync = (last + 1) % sampPerPeriod;
                break;
            default:
                for (int i = 0 ; i < BUFFERSIZE ; i++) {
                    osc->buffer[i] = 0;
                }
        }

        
        
        pthread_mutex_lock(&(osc->lock));
        osc->done = 1;
        pthread_cond_signal(&(osc->cond));
    }
    pthread_mutex_unlock(&(osc->lock));
    
    return NULL;
}

oscillator* createOsc(int type, int freq){
    //Allocate memory
    oscillator* osc = malloc(sizeof(oscillator));
    
    //Error check
    if (osc == NULL){
        perror("allocating memory for oscillator");
        exit(-1);
    }
    
    //Allocate memory for buffer
    osc->buffer = malloc(sizeof(int16_t) * BUFFERSIZE);
    
    //Error check
    if (osc->buffer == NULL){
        perror("allocating memory for oscillator buffer");
        exit(-1);
    }
    
    //Init values
    osc->freq = freq;
    osc->type = type;
    osc->done = 0;
    osc->sync = 0;
    pthread_mutex_init(&(osc->lock), NULL);

    //Start oscillator
    pthread_create(&(osc->thread), NULL, &oscillatorWork, osc);
    
    return osc;
}

void killThread(oscillator* osc){
    pthread_mutex_lock(&(osc->lock));
    osc->exit = 1;
    pthread_cond_signal(&(osc->cond));
    pthread_mutex_unlock(&(osc->lock));
    pthread_join(osc->thread, NULL);
}

void setFreq(oscillator* osc, int freq){
    pthread_mutex_lock(&(osc->lock));
    osc->freq = freq;
    pthread_mutex_unlock(&(osc->lock));
}

void setType(oscillator* osc, int type){
    pthread_mutex_lock(&(osc->lock));
    osc->type = type;
    pthread_mutex_unlock(&(osc->lock));
}
