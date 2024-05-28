/**
 * Project 2
 * LC-2K Linker
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include	<stdbool.h>

#define MAXSIZE 500
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry {
	char label[7];
	char location;
	unsigned int offset;
};

struct RelocationTableEntry {
	unsigned int file;
	unsigned int offset;
	char inst[6];
	char label[7];
};

struct FileData {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	unsigned int textStartingLine; // in final executable
	unsigned int dataStartingLine; // in final executable
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	int text[MAXSIZE * MAXFILES];
	int data[MAXSIZE * MAXFILES];
	SymbolTableEntry symbolTable[MAXSIZE * MAXFILES];
	RelocationTableEntry relocTable[MAXSIZE * MAXFILES];
};

int main(int argc, char *argv[]) {
	char *inFileStr, *outFileStr;
	FILE *inFilePtr, *outFilePtr; 
	unsigned int i, j, totalTextSize=0, totalDataSize=0;

    if (argc <= 2 || argc > 8 ) {
        printf("error: usage: %s <MAIN-object-file> ... <object-file> ... <output-exe-file>, with at most 5 object files\n",
				argv[0]);
		exit(1);
	}

	outFileStr = argv[argc - 1];

	outFilePtr = fopen(outFileStr, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileStr);
		exit(1);
	}

	FileData files[MAXFILES];

  // read in all files and combine into a "master" file
	for (i = 0; i < argc - 2; ++i) {
		inFileStr = argv[i+1];

		inFilePtr = fopen(inFileStr, "r");
		printf("opening %s\n", inFileStr);

		if (inFilePtr == NULL) {
			printf("error in opening %s\n", inFileStr);
			exit(1);
		}

		char line[MAXLINELENGTH];
		unsigned int textSize, dataSize, symbolTableSize, relocationTableSize;

		// parse first line of file
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d",
				&textSize, &dataSize, &symbolTableSize, &relocationTableSize);
		totalDataSize+=dataSize;
		totalTextSize+=textSize;
		files[i].textSize = textSize;
		files[i].dataSize = dataSize;
		files[i].symbolTableSize = symbolTableSize;
		files[i].relocationTableSize = relocationTableSize;

		// read in text section
		int instr;
		for (j = 0; j < textSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = strtol(line, NULL, 0);
			files[i].text[j] = instr;
		}

		// read in data section
		int data;
		for (j = 0; j < dataSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = strtol(line, NULL, 0);
			files[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		unsigned int addr;
		for (j = 0; j < symbolTableSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d",
					label, &type, &addr);
			files[i].symbolTable[j].offset = addr;
			strcpy(files[i].symbolTable[j].label, label);
			files[i].symbolTable[j].location = type;
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < relocationTableSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s",
					&addr, opcode, label);
			files[i].relocTable[j].offset = addr;
			strcpy(files[i].relocTable[j].inst, opcode);
			strcpy(files[i].relocTable[j].label, label);
			files[i].relocTable[j].file	= i;
		}
		fclose(inFilePtr);
	} // end reading files

	// *** INSERT YOUR CODE BELOW ***
	//    Begin the linking process
	//    Happy coding!!!

	//------ERROR CHECKING------//
	char allGAddr[MAXLINELENGTH][MAXLINELENGTH];
	char allGType[MAXLINELENGTH];
	int gCount=0;
	for(i = 0; i < argc - 2; ++i){
		for (j = 0; j < files[i].symbolTableSize; ++j) {
			strcpy(allGAddr[gCount], files[i].symbolTable[j].label);
			allGType[gCount++]=files[i].symbolTable[j].location;
			//error: defining stack label
			if(!strcmp(files[i].symbolTable[j].label, "Stack") && files[i].symbolTable[j].location!='U')exit(1);
		}
	}
	//Check for undefined global address

	//Check for duplicate definiteion of global address
	for(int i=0;i<gCount-1;i++){
		for(int j=i+1;j<gCount;j++){
			if(!strcmp(allGAddr[i],allGAddr[j])){
				if(allGType[i]!='U' && allGType[j]!='U')exit(1);
			}
		}
	}
	//printf("done error checking\n");
	//------END OF ERROR CHECKING------//

	for(i=0;i<argc-2;i++){//for each file
		//looks at reloc
		for(j=0;j<files[i].relocationTableSize;j++){
			//in data section
			if(!strcmp(files[i].relocTable[j].inst, ".fill")){
				int newLoc;
				//local
				if('a'<=files[i].relocTable[j].label[0] &&files[i].relocTable[j].label[0] <='z'){
					newLoc=files[i].data[files[i].relocTable[j].offset];
					//label in text
					if(newLoc<files[i].textSize){
						for(int k=0; k < i ; k++)newLoc+=files[k].textSize;
					}
					else{
						newLoc-=files[i].textSize;
						newLoc+=totalTextSize;
						for(int k=0;k<i;k++)newLoc+=files[k].dataSize;
					}
					files[i].data[files[i].relocTable[j].offset]=newLoc;
				}
				//global
				else{
					//look for it from all the files
					bool found=false;
					int offset;
					if(!strcmp("Stack", files[i].relocTable[j].label)){
						offset=totalDataSize+totalTextSize;
					}
					else{
						for(int k=0;k<argc-2;k++){
							for(int k2=0; k2<files[k].symbolTableSize;k2++){
								if(!strcmp(files[i].relocTable[j].label, files[k].symbolTable[k2].label) && files[k].symbolTable[k2].location!='U'){
									if(found)exit(1);//duplicate
									found=true;
									offset=files[k].symbolTable[k2].offset;
									if(files[k].symbolTable[k2].location=='T'){
										for(int idk=0;idk<k;idk++)offset+=files[idk].textSize;
									}
									else{
										for(int idk=0;idk<k;idk++)offset+=files[idk].dataSize;
										offset+=totalTextSize;
									}
								}
							}
						}
						if(!found)exit(1);//undefined
					}
					files[i].data[files[i].relocTable[j].offset]=offset;
				}
			}
			
			else{//lw sw , aka text section
				int offSet=0;
				bool found=false;
				//handles global
				if('A'<=files[i].relocTable[j].label[0] && files[i].relocTable[j].label[0]<='Z'){
					if(!strcmp("Stack", files[i].relocTable[j].label)){
						offSet=totalDataSize+totalTextSize;
					}
					else{
						for(int k=0; k<argc-2; k++){
							for(int k2=0;k2<files[k].symbolTableSize;k2++){
								if(!strcmp(files[i].relocTable[j].label, files[k].symbolTable[k2].label) && files[k].symbolTable[k2].location!='U'){
									offSet=files[k].symbolTable[k2].offset;
									if(found)exit(1);//duplicate
									found=true;
									if(files[k].symbolTable[k2].location=='D'){
										for(int idk=0;idk<k;idk++)offSet+=files[idk].dataSize;
										offSet+=totalTextSize;
									}
									else{
										for(int idk=0;idk<k;idk++)offSet+=files[idk].textSize;
									}
								}
							}
						}
						if(!found)exit(1);//undefined label
					}
				}
				//handles local
				else{
					offSet=files[i].text[files[i].relocTable[j].offset]&0x00FFFF;
					files[i].text[files[i].relocTable[j].offset]-=offSet;
					//local label in text section
					if(offSet < files[i].textSize){
						for(int k=0;k<i;k++){//add all the data lines before this file
							offSet+=files[k].textSize;
						}
					}	
					//label in data section
					else{
						offSet-=files[i].textSize;//which line of local data
						offSet+=totalTextSize;
						for(int k=0; k < i ;k ++){
							offSet+=files[k].dataSize;
						}
					}
				}
				files[i].text[files[i].relocTable[j].offset]+=offSet;
			}
		}
	}

	//printf everything to the mc code
	for (i = 0; i < argc - 2; ++i) {
		for (j = 0; j < files[i].textSize; ++j) {
			//printf("%d\n", files[i].text[j]);
			fprintf(outFilePtr, "%d\n", files[i].text[j]);
		}
	}
	for (i = 0; i < argc - 2; ++i) {
		for (j = 0; j < files[i].dataSize; ++j) {
			//printf("%d\n", files[i].text[j]);
			fprintf(outFilePtr, "%d\n", files[i].data[j]);
		}
	}
	//end of printf

} // main
