#ifndef SYNTH_H
#define SYNTH_H

#include "oscList.h"

#define RATE 44100
#define OSC_NR 2
#define BUFFERSIZE 1024

typedef struct synth {
    oscList* osc;
} synth;
#endif
