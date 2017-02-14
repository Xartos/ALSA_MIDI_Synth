#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <pthread.h>
#include <stdint.h>

#define OSC_TYPE_SINE 0
#define OSC_TYPE_SAW 1
#define OSC_TYPE_SQUARE 2
#define OSC_TYPE_TRIANGLE 3

typedef struct oscillator {
    //Internal variables
    int sync;
    
    //External variables
    int type; /* 0 = sine, 1 = saw, 2 = square */
    int freq;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t thread;
    
    short done;
    int16_t *buffer;
    
    short exit;
} oscillator;

oscillator* createOsc(int type, int freq);

void killThread(oscillator* osc);

void setFreq(oscillator* osc, int freq);

void setType(oscillator* osc, int type);
#endif
