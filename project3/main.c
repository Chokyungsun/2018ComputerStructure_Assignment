#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for getopt()

#define BYTES_PER_WORD 4
//#define DEBUG

/*
 * Cache structures
 */
int time = 0;

typedef struct {
	int age;
	int valid;
	int modified;
	uint32_t tag;
} cline;

typedef struct {
	cline *lines;
} cset;

typedef struct {
	int s;
	int E;
	int b;
	cset *sets;
} cache;

static int index_bit(int n) {
	int cnt = 0;
	while(n){
		cnt++;
		n = n >> 1;
	}
	return cnt-1;
}

cache build_cache(int way, int set, int blocksize) {

	int k, i;

	cset* c_set = (cset*)malloc(sizeof(cset)*set);
	for(int i = 0; i< set; i++){
		cline* c_lines = (cline*)malloc(sizeof(cline)*way);
		c_set[i].lines = c_lines;
	}

	cache* caches = (cache*)malloc(sizeof(cache));
	caches->s = index_bit(set);
	caches->E = way;
	caches->b = index_bit(blocksize);
	caches->sets = c_set;


	for(i = 0; i< set; i++){
		for(k=0; k < way;k++){
			caches->sets[i].lines[k].age = 0;
			caches->sets[i].lines[k].valid = 0;
			caches->sets[i].lines[k].tag = 0;
			caches->sets[i].lines[k].modified = 0;
		}
	}
	return *caches;
}

void access_cache(cache* caches, char* op, unsigned long addr, int *writeback, int *readhit, int *writehit, int *readmiss, int *writemiss) {

	int b = caches->b, s = caches->s;
	int way = caches->E;
	int E = index_bit(way);
	int oldest = 0;

	int tag = addr >> (s + b);
	int offset = addr % (1 << b);
	int index = (addr | (s + b)) >> b;
	index = index % (1 << s);

/*
	printf("add: %08x\n", addr);
	printf("tag: %08x\n", tag);
	printf("offset: %08x\n", offset);
	printf("index: %08x\n", index);*/
	int i=0;
	int done = 0;

	if(strcmp(op, "R")==0){
		while(i< way){

			if(caches->sets[index].lines[i].valid == 0){ //readmiss
				caches->sets[index].lines[i].age = 1;
				caches->sets[index].lines[i].valid = 1;
				caches->sets[index].lines[i].tag = tag;
				(*readmiss)++;	
				done =1;	
				break;
			}
			else{

				if(caches->sets[index].lines[i].tag == tag){ //readhit
					(*readhit)++;
					done =1;
					break;
				}
				else i++;
			}
		}


		//LRU
		if(done ==0){

			//find the oldest 
			for(i=0;i<way;i++){
				if(caches->sets[index].lines[oldest].age <  caches->sets[index].lines[i].age){
					oldest = i;
				}
			}

			if(caches->sets[index].lines[oldest].modified == 0){ 
				caches->sets[index].lines[oldest].age = 1;
				caches->sets[index].lines[oldest].valid = 1;
				caches->sets[index].lines[oldest].tag = tag;
				(*readmiss)++;		
			}
			else if(caches->sets[index].lines[oldest].modified == 1){
				caches->sets[index].lines[oldest].modified = 0;
				caches->sets[index].lines[oldest].age = 1;
				caches->sets[index].lines[oldest].valid = 1;
				caches->sets[index].lines[oldest].tag = tag;
				(*writeback)++;
			}

		}
	}

	else if(strcmp(op, "W")==0){

		i=0;
		while( i < way){
				if(caches->sets[index].lines[i].valid == 0){ //writemiss
					caches->sets[index].lines[i].age = 1;
					caches->sets[index].lines[i].valid = 1;
					caches->sets[index].lines[i].tag = tag;
					caches->sets[index].lines[i].modified = 1;
					(*writemiss)++;
					done=1;
					break;
				}
				else{
					if(caches->sets[index].lines[i].tag == tag){ //writehit
						(*writehit)++;
						done =1;
						break;
					}
					else i++;
				}			
			}	

		//LRU
		if(done ==0){

			//find oldest
			for(i=0;i<way;i++){
				if(caches->sets[index].lines[oldest].age <  caches->sets[index].lines[i].age){
					oldest = i;
				}
			}

			if(caches->sets[index].lines[oldest].modified == 0){ //writemiss
				caches->sets[index].lines[oldest].modified = 1;
				caches->sets[index].lines[oldest].age = 1;
				caches->sets[index].lines[oldest].valid = 1;
				caches->sets[index].lines[oldest].tag = tag;
				(*writemiss)++;		
			}
			else if(caches->sets[index].lines[oldest].modified == 1){ //writeback
				caches->sets[index].lines[oldest].age = 1;
				caches->sets[index].lines[oldest].valid = 1;
				caches->sets[index].lines[oldest].tag = tag;
				(*writeback)++;
			}
		}
	}


}

/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */
/*                                                             */
/***************************************************************/
void cdump(int capacity, int assoc, int blocksize){

	printf("Cache Configuration:\n");
	printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", assoc);
	printf("Block Size: %dB\n", blocksize);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat		                           */
/*                                                             */
/***************************************************************/
void sdump(int total_reads, int total_writes, int write_backs,
	int reads_hits, int write_hits, int reads_misses, int write_misses) {
	printf("Cache Stat:\n");
	printf("-------------------------------------\n");
	printf("Total reads: %d\n", total_reads);
	printf("Total writes: %d\n", total_writes);
	printf("Write-backs: %d\n", write_backs);
	printf("Read hits: %d\n", reads_hits);
	printf("Write hits: %d\n", write_hits);
	printf("Read misses: %d\n", reads_misses);
	printf("Write misses: %d\n", write_misses);
	printf("\n");
}


/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */
/* 					                            		       */
/* Cache Design						                           */
/*  							                               */
/* 	    cache[set][assoc][word per block]		               */
/*                                						       */
/*      				                        		       */
/*       ----------------------------------------	           */
/*       I        I  way0  I  way1  I  way2  I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set0  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set1  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*                              						       */
/*                                                             */
/***************************************************************/
void xdump(cache* L)
{
	int i,j,k = 0;
	int b = L->b, s = L->s;
	int way = L->E, set = 1 << s;
	int E = index_bit(way);

	uint32_t line;

	printf("Cache Content:\n");
	printf("-------------------------------------\n");
	for(i = 0; i < way;i++)
	{
		if(i == 0)
		{
			printf("    ");
		}
		printf("      WAY[%d]",i);
	}
	printf("\n");

	for(i = 0 ; i < set;i++)
	{
		printf("SET[%d]:   ",i);
		for(j = 0; j < way;j++)
		{
			if(k != 0 && j == 0)
			{
				printf("          ");
			}
			if(L->sets[i].lines[j].valid){
				line = L->sets[i].lines[j].tag << (s+b);
				line = line|(i << b);
			}
			else{
				line = 0;
			}
			printf("0x%08x  ", line);
		}
		printf("\n");
	}
	printf("\n");
}




int main(int argc, char *argv[]) {
	int i, j, k;
	int capacity=1024;
	int way=8;
	int blocksize=8;
	int set;

	//cache
	cache simCache;

	// counts
	int read=0, write=0, writeback=0;
	int readhit=0, writehit=0;
	int readmiss=0, writemiss = 0;

	// Input option
	int opt = 0;
	char* token;
	int xflag = 0;

	// parse file
	char *trace_name = (char*)malloc(32);
	FILE *fp;
	char line[16];
	char *op;
	uint32_t addr;

    /* You can define any variables that you want */

	trace_name = argv[argc-1];
	if (argc < 3) {
		printf("Usage: %s -c cap:assoc:block_size [-x] input_trace \n",argv[0]);
		exit(1);
	}
	while((opt = getopt(argc, argv, "c:x")) != -1){
		switch(opt){
			case 'c':
                // extern char *optarg;
			token = strtok(optarg, ":");
			capacity = atoi(token);
			token = strtok(NULL, ":");
			way = atoi(token);
			token = strtok(NULL, ":");
			blocksize  = atoi(token);
			break;
			case 'x':
			xflag = 1;
			break;
			default:
			printf("Usage: %s -c cap:assoc:block_size [-x] input_trace \n",argv[0]);
			exit(1);

		}
	}

	// allocate
	set = capacity/way/blocksize;

    /* TODO: Define a cache based on the struct declaration */
	simCache = build_cache(way, set, blocksize);

	// simulate
	fp = fopen(trace_name, "r"); // read trace file
	if(fp == NULL){
		printf("\nInvalid trace file: %s\n", trace_name);
		return 1;
	}
	cdump(capacity, way, blocksize);

    /* TODO: Build an access function to load and store data from the file */
	while (fgets(line, sizeof(line), fp) != NULL) {
		op = strtok(line, " ");
		addr = strtoull(strtok(NULL, ","), NULL, 16);

#ifdef DEBUG
        // You can use #define DEBUG above for seeing traces of the file.
		fprintf(stderr, "op: %s\n", op);
		fprintf(stderr, "addr: %x\n", addr);
#endif
		if(strcmp(op, "R") == 0){
			read++;
		}
		else if (strcmp(op, "W") == 0){
			write++;
		}

		//aging
		for(int i=0; i<set ;i++){
			for(int j=0; j<way; j++){
				if(simCache.sets[i].lines[j].age != 0 && simCache.sets[i].lines[j].valid == 1){
						simCache.sets[i].lines[j].age++;
				}
			}
		}
        //printf("%08x\n", addr);

		access_cache(&simCache, op, addr, &writeback, &readhit, &writehit, &readmiss, &writemiss);
		//xdump(&simCache);
	}

    // test example
	sdump(read, write, writeback, readhit, writehit, readmiss, writemiss);
	if (xflag){
		xdump(&simCache);
	}

	return 0;
}
