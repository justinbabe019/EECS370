// EECS 370 SP 23 - Lab 2
// MODIFY AND SUBMIT THIS FILE
#include "numHighBits.h"

// Takes in an integer as an argument, and returns the number of bits set high in its binary representation
int numHighBits(int input){
    int numHighBits=0;
    int size=sizeof(input)*8;
    for(int i=0;i<size;++i){
        if(input & (1<<i)){
            numHighBits++;
        }
    }
    return numHighBits;
}
