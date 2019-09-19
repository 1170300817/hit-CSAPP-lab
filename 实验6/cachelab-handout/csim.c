#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"

//#define DEBUG_ON
#define ADDRESS_LENGTH 64

/* Type: Memory address */
typedef unsigned long long int mem_addr_t;

/* Type: Cache line
   LRU is a counter used to implement LRU replacement policy  */
typedef struct cache_line
{
    char valid;
    mem_addr_t tag;
    unsigned long long int lru;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

int verbosity = 0; /* print trace if set */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* associativity */
char* trace_file = NULL;

int S; /* number of sets */
int B; /* block size (bytes) */

int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 1;

cache_t cache;

void initCache();
void freeCache();
int getSet(mem_addr_t addr);
int getTag(mem_addr_t addr);
int isMiss(mem_addr_t addr);
void updateLruNumber(int setBits,int hitIndex);
int findMinLruNumber(int setBits);
int updateCache(mem_addr_t addr);
void accessData(mem_addr_t addr);

void initCache()
{
    int i,j;
    cache=(cache_set_t*)malloc(sizeof(cache_set_t)*S);
    for (i=0; i<S; i++)
    {
        cache[i]=(cache_line_t*)malloc(sizeof(cache_line_t)*E);
        for(j=0; j<E; j++)
        {
            cache[i][j].valid=0;
            cache[i][j].tag=0;
            cache[i][j].lru=0;
        }
    }
}

void freeCache()
{
    int i;
    for (i=0; i<S; i++)
    {
        free(cache[i]);
    }
    free(cache);
}

int getSet(mem_addr_t addr)
{
    addr = addr >> b;
    int mask =  (1<<s)-1;
    return addr &mask;
}
int getTag(mem_addr_t addr)
{
    int mask = s+b;
    return addr >> mask;
}
int isMiss(mem_addr_t addr)
{
    int j;
    int isMiss = 1;
    int setBits=getSet(addr);
    int tagBits=getTag(addr);
    for(j=0; j<E; j++)
    {
        if(cache[setBits][j].valid == 1 && cache[setBits][j].tag == tagBits)
        {
            isMiss = 0;
            updateLruNumber(setBits,j);
        }
    }
    return isMiss;
}
void updateLruNumber(int setBits,int hitIndex)
{
    cache[setBits][hitIndex].lru = 99999;
    int j;
    for(j=0; j<E; j++)
    {
        cache[setBits][j].lru--;
    }
}
int findMinLruNumber(int setBits)
{
    int j;
    int minIndex=0;
    int minLru = 99999;
    for(j=0; j<E; j++)
    {
        if(cache[setBits][j].lru < minLru)
        {
            minIndex = j;
            minLru = cache[setBits][j].lru;
        }
    }
    return minIndex;
}
int updateCache(mem_addr_t addr)
{
    int j;
    int isfull = 1;
    int setBits=getSet(addr);
    int tagBits=getTag(addr);
    for(j=0; j<E; j++)
    {
        if(cache[setBits][j].valid== 0)
        {
            isfull = 0;
            break;
        }
    }
    if(isfull == 0)
    {
        cache[setBits][j].valid= 1;
        cache[setBits][j].tag = tagBits;
        updateLruNumber(setBits,j);
    }
    else
    {
        int evictionIndex = findMinLruNumber(setBits);
        cache[setBits][evictionIndex].valid = 1;
        cache[setBits][evictionIndex].tag = tagBits;
        updateLruNumber(setBits,evictionIndex);
    }
    return isfull;
}

void accessData(mem_addr_t addr)
{
    if(isMiss(addr)==1)
    {
        miss_count++;
        if(verbosity == 1)
            printf("miss ");
        if(updateCache(addr) == 1)
        {
            eviction_count++;
            if(verbosity==1)
                printf("eviction ");
        }
    }
    else
    {
        hit_count++;
        if(verbosity == 1)
            printf("hit ");
    }
}
void replayTrace(char* trace_fn)
{
    char buf[1000];
    mem_addr_t addr=0;
    unsigned int len=0;
    FILE* trace_fp = fopen(trace_fn, "r");
    if(!trace_fp)
    {
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }
    while( fgets(buf, 1000, trace_fp) != NULL)
    {
        if(buf[1]=='S' || buf[1]=='L' || buf[1]=='M')
        {
            sscanf(buf+3, "%llx,%u", &addr, &len);
            if(verbosity)
                printf("%c %llx,%u ", buf[1], addr, len);
            accessData(addr);
            /* If the instruction is R/W then access again */
            if(buf[1]=='M')
                accessData(addr);
            if (verbosity)
                printf("\n");
        }
    }
    fclose(trace_fp);
}
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

int main(int argc, char* argv[])
{
    char c;
    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1)
    {
        switch(c)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace_file = optarg;
            break;
        case 'v':
            verbosity = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL)
    {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }
    S=pow(2,s);
    B=pow(2,b);

    initCache();

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
    printf("DEBUG: set_index_mask: %llu\n", set_index_mask);
#endif

    replayTrace(trace_file);
    freeCache();
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
