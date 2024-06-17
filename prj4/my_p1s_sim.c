/*
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 *
 * Make sure to NOT modify printState or any of the associated functions
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//DO NOT CHANGE THE FOLLOWING DEFINITIONS 

// Machine Definitions
#define MEMORYSIZE 65536 /* maximum number of words in memory (maximum number of lines in a given file)*/
#define NUMREGS 8 /*total number of machine registers [0,7]*/

// File Definitions
#define MAXLINELENGTH 1000 /* MAXLINELENGTH is the max number of characters we read */

typedef struct 
stateStruct {
    int pc;
    int mem[MEMORYSIZE];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);

static inline int convertNum(int32_t);

extern void cache_init(int blockSize, int numSets, int blocksPerSet);
extern int cache_access(int addr, int write_flag, int write_data);
extern void printStats();
static stateType state;
static int num_mem_accesses = 0;
int mem_access(int addr, int write_flag, int write_data) {
    ++num_mem_accesses;
    if (write_flag) {
        state.mem[addr] = write_data;
        if(state.numMemory <= addr) {
            state.numMemory = addr + 1;
        }
    }
    return state.mem[addr];
}
int get_num_mem_accesses(){
	return num_mem_accesses;
}

int 
main(int argc, char **argv)
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s , please ensure you are providing the correct path", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state.numMemory) {
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address  %d\n. Please ensure you are providing a machine code file.", state.numMemory);
            perror("sscanf");
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    //Your code starts here!
    //initialize
    state.pc=0;
    for(int i=0;i<8;i++)state.reg[i]=0;
    //simulating
    bool halted=false;
    int insNum=0;
    cache_init(4,2,1);
    while(state.pc<state.numMemory && !halted){
        insNum++;
        //printState(&state);
        //DECODE
        int op=state.mem[state.pc]>>22,f0=(state.mem[state.pc]>>19)&0b111, f1=(state.mem[state.pc]>>16)&0b111, f2;
        cache_access(state.pc, 0, 0);
        //SIMULATE
        switch (op)
        {
        case 0://add
            f2=state.mem[state.pc]&0b111;
            state.reg[f2]=state.reg[f0]+state.reg[f1];
            state.pc++;
            break;
        case 1://nor
            f2=state.mem[state.pc]&0b111;
            state.reg[f2]=state.reg[f0]|state.reg[f1];
            state.reg[f2]=~state.reg[f2];
            state.pc++;
            /* code */
            break;
        case 2://lw
            //TODO f2
            f2=convertNum(state.mem[state.pc]&0xFFFF);
            state.reg[f1]=cache_access(state.reg[f0]+f2, 0, 0);
            state.pc++;
            //lw 0 1 bob; r1=mem[r0+6]
            break;
        case 3://sw
            //TODO: f2
            f2=convertNum(state.mem[state.pc]&0xFFFF);
            cache_access(state.reg[f0]+f2, 1 ,state.reg[f1]);
            state.pc++;
            break;
        case 4://beq
            if(state.reg[f0]==state.reg[f1]){
                f2=convertNum(state.mem[state.pc]&0xFFFF);
                state.pc=state.pc+1+f2;
                printf("%d F2=\n",f2);
            }
            else
                state.pc++;
            break;
        case 5://jalr
            state.reg[f1]=state.pc+1;
            if(f0==f1)state.pc=state.pc+1;
            else
                state.pc=state.reg[f0];
            break;
        case 6://halt
            halted=true;
            state.pc++;
            break;
        case 7://noop
            state.pc++;
            break;
        default:
            exit(1);
            break;
        }
    }
    //end of simulator
    printf("machine halted\ntotal of %d instructions executed\nfinal state of machine:\n", insNum);
    printStats();
    printState(&state);
    //Your code ends here! 

    return(0);
}

/*
* DO NOT MODIFY ANY OF THE CODE BELOW. 
*/

void 
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) 
              printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) 
              printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    printf("end state\n");
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) 
{
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

/*
* Write any helper functions that you wish down here. 
*/
