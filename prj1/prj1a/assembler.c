/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

//defining variables
char labels[MAXLINELENGTH][MAXLINELENGTH];
short insNum=0;//number of instructions

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);

//MY HELPER Functions
//returns arg2 in number
int checkArg2(char * arg2,int pc, bool beq){
    if(!isNumber(arg2)){
        for(int i=0;i<insNum;i++){
            if(!strcmp(arg2, labels[i])){
                //offset+pc+1=i
                return beq?i-1-pc:i;
            }
        }
        exit(1);//ERROR: use of undefined labels
    }
    //printf("%s %d %d\n",arg2, atoi(arg2),pc);
    return atoi(arg2);
}

//check if arg's number/address is valid
int checkAddress(int arg){
    int t=arg;
    if(t<-32768 || t>32767)exit(1);//offsetFields that don’t fit in 16 bits
    t&=0xFFFF;
    return t;
}

//check if they are all numbers
void numCheck2(char *arg0, char*arg1){
    if(!isNumber(arg0)||!isNumber(arg1))exit(1);
}
void numCheck(char *arg0, char*arg1, char* arg2){
    if(!isNumber(arg0)||!isNumber(arg1)||!isNumber(arg2))exit(1);
}

//register number check
void regNumCheck(int arg0, int arg1, int arg2){
    if(arg0<0 || arg1<0 || arg2<0 || arg0>7 || arg1>7 || arg2>7)exit(1);
}

void regNumCheck2(int arg0, int arg1){
    if(arg0<0 || arg1<0 || arg0>7 || arg1>7)exit(1);
}



int main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    //read the labels
    while(readAndParse(inFilePtr,label,opcode,arg0,arg1,arg2)){
        strcpy(labels[insNum++],label);
    }
    rewind(inFilePtr);

    //ERROR CHECK DUPLICATE LABELS
    for(int i=0;i<insNum-1;i++){
        for(int j=i+1;j<insNum;j++){
            if(!strcmp(labels[i],labels[j]) && (strlen(labels[i])>0)){
                printf("lab i = %s , lab j = %s\n",labels[i], labels[j]);
                exit(1);
            }
        }
    }
    //do calculations
    int programCounter=0, instruction=0;
    while(readAndParse(inFilePtr,label,opcode,arg0,arg1,arg2)){
        //RTYPE
        if(!strcmp(opcode, "add")){
            numCheck(arg0,arg1,arg2);
            regNumCheck(atoi(arg0),atoi(arg1),atoi(arg2));
            instruction+=0b000<<22;
            instruction+=(atoi(arg0)<<19) + (atoi(arg1)<<16) + atoi(arg2);
        }
        else if(!strcmp(opcode, "nor")){
            numCheck(arg0,arg1,arg2);
            regNumCheck(atoi(arg0),atoi(arg1),atoi(arg2));
            instruction+=0b001<<22;
            instruction+= (atoi(arg0)<<19) + (atoi(arg1)<<16) + atoi(arg2);
        }
        //ITYPE
        else if(!strcmp(opcode, "lw")){
            numCheck2(arg0,arg1);
            regNumCheck2(atoi(arg0), atoi(arg1));
            int arg2temp=checkArg2(arg2,0,0);
            checkAddress(arg2temp);
            instruction+=0b010<<22;
            if(arg2temp<0)arg2temp&=0xFFFF;
            instruction+= (atoi(arg0)<<19) + (atoi(arg1)<<16) + arg2temp;
        }
        else if(!strcmp(opcode, "sw")){
            numCheck2(arg0,arg1);
            regNumCheck2(atoi(arg0), atoi(arg1));
            int arg2temp=checkArg2(arg2,0,0);
            checkAddress(arg2temp);
            instruction+=0b011<<22;
            if(arg2temp<0)arg2temp&=0xFFFF;
            instruction+= (atoi(arg0)<<19) + (atoi(arg1)<<16) + arg2temp;
        }
        else if(!strcmp(opcode, "beq")){
            numCheck2(arg0,arg1);
            regNumCheck2(atoi(arg0), atoi(arg1));
            int arg2temp=checkArg2(arg2, programCounter,1);
            checkAddress(arg2temp);
            instruction+= 0b100<<22;
            if(arg2temp<0)arg2temp&=0xFFFF;
            instruction+= (atoi(arg0)<<19) + (atoi(arg1)<<16) + arg2temp;
        }
        //JType
        else if(!strcmp(opcode, "jalr")){
            numCheck2(arg0, arg1);
            regNumCheck2(atoi(arg0), atoi(arg1));
            instruction+= 0b101<<22;
            instruction+= (atoi(arg0)<<19) + (atoi(arg1)<<16);
        }
        //OType
        else if(!strcmp(opcode, "halt")){
            instruction+= 0b110<<22;
        }
        else if(!strcmp(opcode, "noop")){
            instruction+= 0b111<<22;
            //continue;
        }
        else if(!strcmp(opcode,".fill")){
            if(isNumber(arg0))instruction=atoi(arg0);
            else
                instruction=checkArg2(arg0,0,0);
        }
        else{
            exit(1);
        }
        fprintf(outFilePtr,"%d\n",instruction);
        programCounter++;
        instruction=0;
    }
    return(0);
}

// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line) {
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(int line_idx=0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr) {
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for(int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address) {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH-1) {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if(lineIsBlank(line)) {
            if(!blank_line_encountered) {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        } else {
            if(blank_line_encountered) {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}


/*
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Ignore blank lines at the end of the file.
    if(lineIsBlank(line)) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);

    return(1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}

