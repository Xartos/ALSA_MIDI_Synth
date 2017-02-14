#include "oscList.h"
#include "oscillator.h"
#include <stdlib.h>
#include <stdio.h>

oscList* createOscList(){
    //Allocate memory
    oscList* list = malloc(sizeof(oscList));
    
    //Error check
    if (list == NULL){
        perror("allocating memory for oscillator list");
        exit(-1);
    }
    
    //Init values
    list->head = NULL;
    list->length = 0;
    
    return list;
}

void addOsc(oscList* list, oscillator* osc){
    //Allocate memory
    oscListElm* newElement = malloc(sizeof(oscListElm));
    
    //Error check
    if (newElement == NULL){
        perror("allocating memory for new oscillator");
        exit(-1);
    }
    
    //Init values
    newElement->osc = osc;
    newElement->next = NULL;
    
    //Get last in list
    oscListElm* currElement = list->head;
    
    //No first element
    if(currElement == NULL){
        list->head = newElement;
    }else {
        
        //Go to last element
        while(currElement->next != NULL){
            currElement = currElement->next;
        }
        
        //Add element and increment length
        currElement->next = newElement;
    }
    
    list->length++;
}

void createAddOsc(oscList* list, int type, int freq){
    //Create oscillator
    oscillator* osc = createOsc(type, freq);
    
    //Add oscillator to list
    addOsc(list, osc);
}

oscListElm* getOsc(oscList* list, int index){
    //Check index for error values
    if(index < 0 || index >= list->length){
        return NULL;
    }
    
    //Get first element
    oscListElm* currElement = list->head;
    
    //Go <index> steps in list 
    for(int i = index; i > 0; i--){
        currElement = currElement->next;
    }
    
    return currElement;
}

int removeOsc(oscList* list, oscListElm* osc){
    int i = 0;
    
    //Empty list
    if (list->length <= 0){
        return -1;
    }
    
    //Get first element
    oscListElm* currElement = list->head;
    oscListElm* lastElement = NULL;
    
    //Go <index> steps in list 
    while(currElement->osc != osc->osc && currElement != NULL){
        //Save last element for future use
        lastElement = currElement;
        currElement = currElement->next;
        i++;
    }
    
    if(currElement == NULL){
        return -1;
    }
    
    if(i == 0){
        //Remove head
        list->head = currElement->next;
    } else if (currElement->next == NULL){
        //Remove tail
        lastElement->next = NULL;
    } else {
        //Remove middle
        lastElement->next = currElement->next;
    }
    
    killThread(currElement->osc);
    free(currElement->osc);
    free(currElement);
    return --list->length;
}

void freeOscList(oscList* list){
    //Get last in list
    oscListElm* currElement = list->head;
    
    //No elements to free
    if (currElement != NULL){
        //First check current element
        oscListElm* freeElement = currElement;
        
        //Free while it isn't the end
        while(freeElement != NULL){
            freeElement = currElement->next;
            free(currElement->osc);
            free(currElement);
            currElement = freeElement;
        }
    }
    
    free(list);
}
