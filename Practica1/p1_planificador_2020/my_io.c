//
// Created by jrivaden on 10/2/20.
//

#include "my_io.h"




int ticks_to_seconds(int ticks){
    int aux = ticks / QUANTUM_TICKS ;
    int seconds = QUANTUM_TIME * aux;
    return seconds;
}


int seconds_to_ticks (int seconds){
    if(seconds < 0){
        printf("ERROR\n");
        exit(-1);
    }
    double aux = seconds / QUANTUM_TIME;
    int ticks =(int) aux * QUANTUM_TICKS;
    return ticks;
}


