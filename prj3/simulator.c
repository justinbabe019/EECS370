/*
 * EECS 370, University of Michigan, Fall 2023
 * Project 3: LC-2K Pipeline Simulator
 * Instructions are found in the project spec: https://eecs370.github.io/project_3_spec/
 * Make sure NOT to modify printState or any of the associated functions
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Machine Definitions
#define NUMMEMORY 65536 // maximum number of data words in memory
#define NUMREGS 8 // number of machine registers

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 // will not implemented for Project 3
#define HALT 6
#define NOOP 7

const char* opcode_to_str_map[] = {
    "add",
    "nor",
    "lw",
    "sw",
    "beq",
    "jalr",
    "halt",
    "noop"
};

#define NOOPINSTR (NOOP << 22)

typedef struct IFIDStruct {
    int instr;
	int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
    int instr;
	int pcPlus1;
	int valA;
	int valB;
	int offset;
} IDEXType;

typedef struct EXMEMStruct {
    int instr;
	int branchTarget;
    int eq;
	int aluResult;
	int valB;
} EXMEMType;

typedef struct MEMWBStruct {
    int instr;
	int writeData;
} MEMWBType;

typedef struct WBENDStruct {
    int instr;
	int writeData;
} WBENDType;

typedef struct stateStruct {
    unsigned int numMemory;
    unsigned int cycles; // number of cycles run so far
	int pc;
	int instrMem[NUMMEMORY];
	int dataMem[NUMMEMORY];
	int reg[NUMREGS];
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
} stateType;

static inline int opcode(int instruction) {
    return instruction>>22;
}

static inline int field0(int instruction) {
    return (instruction>>19) & 0x7;
}

static inline int field1(int instruction) {
    return (instruction>>16) & 0x7;
}

static inline int field2(int instruction) {
    return instruction & 0xFFFF;
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) {
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

void printState(stateType*);
void printInstruction(int);
void readMachineCode(stateType*, char*);


int main(int argc, char *argv[]) {

    /* Declare state and newState.
       Note these have static lifetime so that instrMem and
       dataMem are not allocated on the stack. */

    static stateType state, newState;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    readMachineCode(&state, argv[1]);

    /* ------------ Initialize State ------------ */
    state.pc=0;
    state.IFID.instr=0b111<<22;
    state.IFID.pcPlus1=0;
    state.IDEX.instr=0b111<<22;
    state.IDEX.offset=0;
    state.IDEX.pcPlus1=0;
    state.IDEX.valA=0;
    state.IDEX.valB=0;
    state.EXMEM.aluResult=0;
    state.EXMEM.branchTarget=0;
    state.EXMEM.eq=0;
    state.EXMEM.instr=0b111<<22;
    state.EXMEM.valB=0;
    state.MEMWB.instr=0b111<<22;
    state.MEMWB.writeData=0;
    state.WBEND.instr=0b111<<22;
    state.WBEND.writeData=0;

    int depreg=-1;
    /* ------------------- END ------------------ */

    newState = state;

    while (opcode(state.MEMWB.instr) != HALT) {
        printState(&state);

        newState.cycles += 1;

        /* ---------------------- IF stage --------------------- */
        newState.pc=state.pc+1;
        newState.IFID.pcPlus1=newState.pc;
        newState.IFID.instr=state.instrMem[state.pc];
        /* ---------------------- ID stage --------------------- */
        newState.IDEX.pcPlus1=state.IFID.pcPlus1;
        newState.IDEX.instr=state.IFID.instr;
        newState.IDEX.valA=state.reg[field0(state.IFID.instr)];
        newState.IDEX.valB=state.reg[field1(state.IFID.instr)];
        newState.IDEX.offset=convertNum(field2(state.IFID.instr));
        //detect data hazard
        bool been=false;
        if(depreg==field0(state.IFID.instr))been=true;
        if(depreg==field1(state.IFID.instr) && opcode(newState.IDEX.instr)!=LW)
            been=true;
        if(been){
            newState.IFID.instr=state.IFID.instr;
            newState.IDEX.instr=0b111<<22;
            newState.IFID.pcPlus1=state.IFID.pcPlus1;
            newState.pc=state.pc;
        }
        //increase and store dependencies
        if(opcode(newState.IDEX.instr)==LW)
            depreg=field1(state.IFID.instr);
        else
            depreg=-1;
        /* ---------------------- EX stage --------------------- */
        newState.EXMEM.instr=state.IDEX.instr;
        newState.EXMEM.branchTarget=state.IDEX.pcPlus1+state.IDEX.offset;
        //data hazard detect
        bool beenA=false, beenB=false;
        //dependent on prev 1
        if(field0(state.IDEX.instr)==field2(state.EXMEM.instr) && (opcode(state.EXMEM.instr)==ADD || opcode(state.EXMEM.instr)==NOR)){
            state.IDEX.valA=state.EXMEM.aluResult;
            beenA=true;
        }
        if(field1(state.IDEX.instr)==field2(state.EXMEM.instr) && (opcode(state.EXMEM.instr)==ADD || opcode(state.EXMEM.instr)==NOR)){
            state.IDEX.valB=state.EXMEM.aluResult;
            beenB=true;
        }
        //prev 2
        if(!beenA && field0(state.IDEX.instr)==field2(state.MEMWB.instr) && (opcode(state.MEMWB.instr)==ADD || opcode(state.MEMWB.instr)==NOR)){
            state.IDEX.valA=state.MEMWB.writeData;
            beenA=true;
        } 
        if(!beenA && field0(state.IDEX.instr)==field1(state.MEMWB.instr) && opcode(state.MEMWB.instr)==LW){
            state.IDEX.valA=state.MEMWB.writeData;
            beenA=true;
        }
        if(!beenB && field1(state.IDEX.instr)==field2(state.MEMWB.instr) && (opcode(state.MEMWB.instr)==ADD || opcode(state.MEMWB.instr)==NOR)){
            state.IDEX.valB=state.MEMWB.writeData;
            beenB=true;
        }
        if(!beenB && field1(state.IDEX.instr)==field1(state.MEMWB.instr) && opcode(state.MEMWB.instr)==LW){
            state.IDEX.valB=state.MEMWB.writeData;
            beenB=true;
        }
        //prev 3
        if(!beenA && field0(state.IDEX.instr)==field2(state.WBEND.instr) && (opcode(state.WBEND.instr)==ADD || opcode(state.WBEND.instr)==NOR)){
            state.IDEX.valA=state.WBEND.writeData;
            beenA=true;
        }
        if(!beenA && field0(state.IDEX.instr)==field1(state.WBEND.instr) && opcode(state.WBEND.instr)==LW){
            state.IDEX.valA=state.WBEND.writeData;
            beenA=true;
        }
        if(!beenB && field1(state.IDEX.instr)==field2(state.WBEND.instr) && (opcode(state.WBEND.instr)==ADD || opcode(state.WBEND.instr)==NOR)){
            state.IDEX.valB=state.WBEND.writeData;
            beenB=true;
        }
        if(!beenB && field1(state.IDEX.instr)==field1(state.WBEND.instr) && opcode(state.WBEND.instr)==LW){
            state.IDEX.valB=state.WBEND.writeData;
            beenB=true;
        }
        //end w data hazard detect
        switch(opcode(newState.EXMEM.instr)){
            case ADD:
                newState.EXMEM.aluResult=state.IDEX.valA+state.IDEX.valB;
                break;
            case NOR: 
                newState.EXMEM.aluResult=~(state.IDEX.valA|state.IDEX.valB);
                break;
            case LW: 
                newState.EXMEM.aluResult=state.IDEX.offset+state.IDEX.valA;
                break;
            case SW: 
                newState.EXMEM.aluResult=state.IDEX.offset+state.IDEX.valA;
                break;
            case BEQ: 
                newState.EXMEM.eq=(state.IDEX.valA==state.IDEX.valB);
                break;
            case JALR: 
                break;
            case HALT: 
                break;
            case NOOP:
                break;
        }
        newState.EXMEM.valB=state.IDEX.valB;
        /* --------------------- MEM stage --------------------- */
        newState.MEMWB.instr=state.EXMEM.instr;
        if(opcode(newState.MEMWB.instr)==SW)
            newState.dataMem[state.EXMEM.aluResult]=state.EXMEM.valB;
        else if(opcode(newState.MEMWB.instr)==LW)
            newState.MEMWB.writeData=state.dataMem[state.EXMEM.aluResult];
        else if(opcode(newState.MEMWB.instr)==ADD ||opcode(newState.MEMWB.instr)==NOR){
            newState.MEMWB.writeData=state.EXMEM.aluResult;
        }
        if(opcode(state.EXMEM.instr)==BEQ && state.EXMEM.eq){
            newState.IFID.instr=0b111<<22;
            newState.IDEX.instr=0b111<<22;
            newState.EXMEM.instr=0b111<<22;
            newState.pc=state.EXMEM.branchTarget;
        }
        /* ---------------------- WB stage --------------------- */
        newState.WBEND.instr=state.MEMWB.instr;
        newState.WBEND.writeData=state.MEMWB.writeData;
        int regB=field1(newState.WBEND.instr);
        int offset=field2(newState.WBEND.instr);
        if(opcode(state.MEMWB.instr)==LW)newState.reg[regB]=state.MEMWB.writeData;
        else if(opcode(state.MEMWB.instr)==ADD || opcode(state.MEMWB.instr)==NOR)newState.reg[offset]=state.MEMWB.writeData;
        /* ------------------------ END ------------------------ */
        state = newState; /* this is the last statement before end of the loop. It marks the end
        of the cycle and updates the current state with the values calculated in this cycle */
    }
    printf("Machine halted\n");
    printf("Total of %d cycles executed\n", state.cycles);
    printf("Final state of machine:\n");
    printState(&state);
}

/*
* DO NOT MODIFY ANY OF THE CODE BELOW.
*/

void printInstruction(int instr) {
    const char* instr_opcode_str;
    int instr_opcode = opcode(instr);
    if(ADD <= instr_opcode && instr_opcode <= NOOP) {
        instr_opcode_str = opcode_to_str_map[instr_opcode];
    }

    switch (instr_opcode) {
        case ADD:
        case NOR:
        case LW:
        case SW:
        case BEQ:
            printf("%s %d %d %d", instr_opcode_str, field0(instr), field1(instr), convertNum(field2(instr)));
            break;
        case JALR:
            printf("%s %d %d", instr_opcode_str, field0(instr), field1(instr));
            break;
        case HALT:
        case NOOP:
            printf("%s", instr_opcode_str);
            break;
        default:
            printf(".fill %d", instr);
            return;
    }
}

void printState(stateType *statePtr) {
    printf("\n@@@\n");
    printf("state before cycle %d starts:\n", statePtr->cycles);
    printf("\tpc = %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (int i=0; i<statePtr->numMemory; ++i) {
        printf("\t\tdataMem[ %d ] = %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (int i=0; i<NUMREGS; ++i) {
        printf("\t\treg[ %d ] = %d\n", i, statePtr->reg[i]);
    }

    // IF/ID
    printf("\tIF/ID pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->IFID.instr);
    printInstruction(statePtr->IFID.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IFID.pcPlus1);
    if(opcode(statePtr->IFID.instr) == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");

    // ID/EX
    int idexOp = opcode(statePtr->IDEX.instr);
    printf("\tID/EX pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->IDEX.instr);
    printInstruction(statePtr->IDEX.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IDEX.pcPlus1);
    if(idexOp == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegA = %d", statePtr->IDEX.valA);
    if (idexOp >= HALT || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegB = %d", statePtr->IDEX.valB);
    if(idexOp == LW || idexOp > BEQ || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\toffset = %d", statePtr->IDEX.offset);
    if (idexOp != LW && idexOp != SW && idexOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // EX/MEM
    int exmemOp = opcode(statePtr->EXMEM.instr);
    printf("\tEX/MEM pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->EXMEM.instr);
    printInstruction(statePtr->EXMEM.instr);
    printf(" )\n");
    printf("\t\tbranchTarget %d", statePtr->EXMEM.branchTarget);
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\teq ? %s", (statePtr->EXMEM.eq ? "True" : "False"));
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\taluResult = %d", statePtr->EXMEM.aluResult);
    if (exmemOp > SW || exmemOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegB = %d", statePtr->EXMEM.valB);
    if (exmemOp != SW) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // MEM/WB
	int memwbOp = opcode(statePtr->MEMWB.instr);
    printf("\tMEM/WB pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->MEMWB.instr);
    printInstruction(statePtr->MEMWB.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->MEMWB.writeData);
    if (memwbOp >= SW || memwbOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // WB/END
	int wbendOp = opcode(statePtr->WBEND.instr);
    printf("\tWB/END pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->WBEND.instr);
    printInstruction(statePtr->WBEND.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->WBEND.writeData);
    if (wbendOp >= SW || wbendOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");

    printf("end state\n");
    fflush(stdout);
}

// File
#define MAXLINELENGTH 1000 // MAXLINELENGTH is the max number of characters we read

void readMachineCode(stateType *state, char* filename) {
    char line[MAXLINELENGTH];
    FILE *filePtr = fopen(filename, "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", filename);
        exit(1);
    }

    printf("instruction memory:\n");
    for (state->numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state->numMemory) {
        if (sscanf(line, "%d", state->instrMem+state->numMemory) != 1) {
            printf("error in reading address %d\n", state->numMemory);
            exit(1);
        }
        printf("\tinstrMem[ %d ]\t= 0x%08x\t= %d\t= ", state->numMemory, 
            state->instrMem[state->numMemory], state->instrMem[state->numMemory]);
        printInstruction(state->dataMem[state->numMemory] = state->instrMem[state->numMemory]);
        printf("\n");
    }
}
