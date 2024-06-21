/*
 * EECS 370, University of Michigan
 * Project 4: LC-2K Cache Simulator
 * Instructions are found in the project spec.
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256

// **Note** this is a preprocessor macro. This is not the same as a function.
// Powers of 2 have exactly one 1 and the rest 0's, and 0 isn't a power of 2.
#define is_power_of_2(val) (val && !(val & (val - 1)))


/*
 * Accesses 1 word of memory.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads and 1 for writes.
 * write_data is a word, and is only valid if write_flag is 1.
 * If write flag is 1, mem_access does: state.mem[addr] = write_data.
 * The return of mem_access is state.mem[addr].
 */
extern int mem_access(int addr, int write_flag, int write_data);

/*
 * Returns the number of times mem_access has been called.
 */
extern int get_num_mem_accesses(void);

//Use this when calling printAction. Do not modify the enumerated type below.
enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

/* You may add or remove variables from these structs */
typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE];
    int dirty;
    int lruLabel;
    int tag;
    int valid;
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_CACHE_SIZE];
    int blockSize;
    int numSets;
    int blocksPerSet;
    // add any variables for end-of-run stats
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;

void printAction(int, int, enum actionType);
void printCache(void);

/*
 * Set up the cache with given command line parameters. This is
 * called once in main(). You must implement this function.
 */
void cache_init(int blockSize, int numSets, int blocksPerSet)
{
    if (blockSize <= 0 || numSets <= 0 || blocksPerSet <= 0) {
        printf("error: input parameters must be positive numbers\n");
        exit(1);
    }
    if (blocksPerSet * numSets > MAX_CACHE_SIZE) {
        printf("error: cache must be no larger than %d blocks\n", MAX_CACHE_SIZE);
        exit(1);
    }
    if (blockSize > MAX_BLOCK_SIZE) {
        printf("error: blocks must be no larger than %d words\n", MAX_BLOCK_SIZE);
        exit(1);
    }
    if (!is_power_of_2(blockSize)) {
        printf("warning: blockSize %d is not a power of 2\n", blockSize);
    }
    if (!is_power_of_2(numSets)) {
        printf("warning: numSets %d is not a power of 2\n", numSets);
    }
    printf("Simulating a cache with %d total lines; each line has %d words\n",
        numSets * blocksPerSet, blockSize);
    printf("Each set in the cache contains %d lines; there are %d sets\n",
        blocksPerSet, numSets);

    /********************* Initialize Cache *********************/
    cache.blockSize=blockSize;
    cache.blocksPerSet=blocksPerSet;
    cache.numSets=numSets;
    for(int i=0;i<MAX_CACHE_SIZE;i++){
        cache.blocks[i].dirty=0;
        cache.blocks[i].valid=0;
        //LRU?
    }
    for(int i=0; i < cache.numSets; i++){
        for(int j=0;j<cache.blocksPerSet; j++)
            cache.blocks[i*cache.blocksPerSet+j].lruLabel=cache.blocksPerSet-1-j;
    }
    return;
}

//!! potentially write function that converts set, tag, into addr or the otherway round
int setNum(int memaddr){
    return memaddr/cache.blockSize%cache.numSets;
}
int cacheToMemAddr(int setIndex, int tag){
    return setIndex*cache.blockSize + tag*cache.numSets*cache.blockSize;
}
void updatesLRU(int cacheAddr, int setIndex){
    //printf("cache addr is %d, set index is %d\n", cacheAddr, setIndex);
    int temp=cache.blocks[cacheAddr].lruLabel;
    cache.blocks[cacheAddr].lruLabel=cache.blocksPerSet-1;
    int setStartAddrInCache=cache.blocksPerSet*setIndex;
    for(int i=0; i<cache.blocksPerSet; i++){
        if(setStartAddrInCache+i!=cacheAddr && cache.blocks[setStartAddrInCache+i].lruLabel>temp)
            cache.blocks[setStartAddrInCache+i].lruLabel--;
        //printf("lru of %d is %d\n", setStartAddrInCache+i, cache.blocks[setStartAddrInCache+i].lruLabel);
    }
}
/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should only call mem_access when absolutely necessary.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads (fetch/lw) and 1 for writes (sw).
 * write_data is a word, and is only valid if write_flag is 1.
 * The return of mem_access is undefined if write_flag is 1.
 * Thus the return of cache_access is undefined if write_flag is 1.
 */
int cache_access(int addr, int write_flag, int write_data)
{
    /* The next line is a placeholder to connect the simulator to
    memory with no cache. You will remove this line and implement
    a cache which interfaces between the simulator and memory. */

    //QUESTION: how do i know tag, set? How to implement LRU?

    int setIndex = addr/cache.blockSize % cache.numSets;
    int tempTag = addr/cache.blockSize/cache.numSets;//address size / blocksize /num sets
    //int blockStartAddr = setIndex*cache.blocksPerSet+tempTag*cache.blocksPerSet*cache.numSets;//cache to mem
    int setStartAddr = cache.blocksPerSet*setIndex;//*cache.blockSize;//the belonging set's addr in 256 blocks

    //search for block in set
    int invalidCount=0, invalidAddr, invalidTag;
    for(int i=0; i<cache.blocksPerSet; i++){
        //looks for empty spot if no match
        if(!cache.blocks[setStartAddr+i].valid && !invalidCount){
            invalidCount++;
            invalidAddr=setStartAddr+i;
            invalidTag=cache.blocks[setStartAddr+i].tag;//??
        }
        //if matches
        if(cache.blocks[setStartAddr+i].tag==tempTag && cache.blocks[setStartAddr+i].valid){
            updatesLRU(setStartAddr+i, setIndex);                                                                                                                                                                   
            //READ
            if(write_flag==0){
                printAction(addr, 1, cacheToProcessor);
                //or return mem_access(addr, write_flag, write_data);
                return cache.blocks[setStartAddr+i].data[addr-cacheToMemAddr(setIndex, tempTag)];//!!not sure if the data block is right
            }
            //STORE
            else{
                printAction(addr, 1, processorToCache);
                cache.blocks[setStartAddr+i].data[addr-cacheToMemAddr(setIndex, tempTag)]=write_data;
                cache.blocks[setStartAddr+i].dirty=1;
                //printf("%d %d %d\n", addr, setStartAddr+i,cache.blocks[setStartAddr+i].dirty);
                //do i need to print anything??
                return write_data;
            }
        }
    }
    //read data not in set/cache
    //if set does not have empty spot, EVICT
    int addrCache, addrMem = addr-addr%cache.blockSize;
    if(invalidCount) addrCache = invalidAddr;
    //no more empty spots, needs to evict
    if(!invalidCount){
        //printf("is there emptyspot?%d\n",invalidCount);
        int lruadd;
        int isDirty=0;
        //look for lru
        int smallest=cache.blockSize;
        for(int i=0;i<cache.blocksPerSet;i++){
            //printf("the current block addr is %d, and lru is %d\n", setStartAddr+i ,cache.blocks[setStartAddr+i].lruLabel);
            if(cache.blocks[setStartAddr+i].lruLabel==0){
                lruadd=cacheToMemAddr(setIndex, cache.blocks[setStartAddr+i].tag);
                //evict, overwrite mem if dirty
                if(cache.blocks[setStartAddr+i].dirty){
                    printAction(lruadd, cache.blockSize, cacheToMemory);
                    for(int j=0; j<cache.blockSize; j++){
                        mem_access(lruadd+j, 1, cache.blocks[setStartAddr+i].data[j]);
                    }
                }
                else
                    printAction(lruadd, cache.blockSize, cacheToNowhere);
                addrCache=setStartAddr+i;
                break;
            }
        }
    }
    //(set has EMPTY spot) overwrites value in cache
    if(write_flag==0){
        cache.blocks[addrCache].dirty=0;
    }
    else
        cache.blocks[addrCache].dirty=1;
    cache.blocks[addrCache].valid=1;
    cache.blocks[addrCache].tag=tempTag;
    updatesLRU(addrCache, setIndex);
    int ans=-1;
    for(int i=0; i<cache.blockSize; i++){
        cache.blocks[addrCache].data[i] = mem_access(addrMem+i, 0, 0);
        if(addrMem+i==addr)ans=cache.blocks[addrCache].data[i];
    }
    printAction(addrMem, cache.blockSize, memoryToCache);
    //end of overwriting
    if(!write_flag)printAction(addr, 1, cacheToProcessor);
    else{ 
        cache.blocks[addrCache].data[addr-cacheToMemAddr(setIndex, cache.blocks[addrCache].tag)]=write_data;
        printAction(addr, 1, processorToCache);
    }
    return ans;
}


/*
 * print end of run statistics like in the spec. **This is not required**,
 * but is very helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print $$$ in this function
 */
void printStats(void)
{
    printf("$$$ Main memory words accessed: %d\n", get_num_mem_accesses());
    printf("End of run statistics:\n");
    //printf("hits %d, misses %d, writebacks %d\n", );
    return;
}

/*
 * Log the specifics of each cache action.
 *
 *DO NOT modify the content below.
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(int address, int size, enum actionType type)
{
    printf("$$$ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
    else {
        printf("Error: unrecognized action\n");
        exit(1);
    }

}

/*
 * Prints the cache based on the configurations of the struct
 * This is for debugging only and is not graded, so you may
 * modify it, but that is not recommended.
 */
void printCache(void)
{
    printf("\ncache:\n");
    for (int set = 0; set < cache.numSets; ++set) {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block) {
            printf("\t\t[ %i ]: {", block);
            for (int index = 0; index < cache.blockSize; ++index) {
                printf(" %i", cache.blocks[set * cache.blocksPerSet + block].data[index]);
            }
            printf(" }\n");
        }
    }
    printf("end cache\n");
}
