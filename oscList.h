#ifndef OSCLIST_H
#define OSCLIST_H

#include "oscillator.h"

typedef struct oscListElm {
    oscillator* osc;
    struct oscListElm* next;
} oscListElm;

typedef struct oscList {
    oscListElm* head;
    int length;
} oscList;

/* Creates/init a new list */
oscList* createOscList();

/* Add one oscillator */
void addOsc(oscList* list, oscillator* osc);

/* Create and add a oscillator */
void createAddOsc(oscList* list, int type, int freq);

/* Get element on index */
oscListElm* getOsc(oscList* list, int index);

/* Remove one oscillator */
int removeOsc(oscList* list, oscListElm* osc);

/* Remove all and free list */
void freeOscList(oscList* list);
#endif
