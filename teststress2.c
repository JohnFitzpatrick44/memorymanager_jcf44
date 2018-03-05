#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "dmm.h"

#ifdef HAVE_DRAND48
	#define RAND() (drand48())
	#define SEED(x) (srand48((x)))
#else
	#define RAND() ((double)random()/RAND_MAX)
	#define SEED(x) (srandom((x)))
#endif

#define BUFLEN (1000)

#define LOOPCNT (50000)

#define MAX_ALLOC_SIZE (MAX_HEAP_SIZE/100)

/* Set to 1 for non-deterministic seeding after each execution */
#define PSEUDO_RANDOM_SEED	1

#define ALLOC_CONST	0.5

int main(int argc, char *argv[]) {

	int size;
	int itr;
	void *ptr[BUFLEN];
	int i;
	double randvar;
	int fail = 0;
	
	clock_t begin, end;
	double time_spent;


	/* Set the PSEUDO_RANDOM_SEED for pseduo random seed initialization based on time, i.e.,
 	 * the random values changes after each execution 
 	 */
	if(PSEUDO_RANDOM_SEED)
		SEED(time(NULL));

	assert(MAX_HEAP_SIZE >= 1024*1024 && "MAX_HEAP_SIZE is too low; Recommended setting is at least 1MB for test_stress2");

	for(i=0; i < BUFLEN; i++) {
		ptr[i] = NULL;
	}

	begin = clock();

	for(i = 0; i < LOOPCNT; i++) {
		itr = (int)(RAND() * BUFLEN);
		//print_freelist();
		randvar = RAND();

		if(randvar < ALLOC_CONST && ptr[itr] == NULL) {
			size = (int)(RAND() * MAX_ALLOC_SIZE);
			if(size > 0) {
				printf("Mallocing size %d\n",size);
				ptr[itr] = dmalloc(size);
                //memset(ptr[itr], 0, size);
			}
			else
				continue;
			if(ptr[itr] == NULL) {
				printf("malloc at iteration %d failed for size %d\n", i,size);
				fflush(stderr);
				++fail;
			}
		} else if(randvar >= ALLOC_CONST && ptr[itr] != NULL) {
			printf("Freeing ptr[%d]\n", itr);
			dfree(ptr[itr]);
			ptr[itr] = NULL;
		}
	}

	/*
 	* now -- free them
 	* */
	for(i=0; i < BUFLEN; i++) {
		if(ptr[i] != NULL) {
			dfree(ptr[i]);
			ptr[i] = NULL;
		}
	}
	end = clock();

	print_freelist();
	printf("\n");
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	
	printf("Test case summary\n");
	printf("Loop count: %d, malloc successful: %d, malloc failed: %d, execution time: %g seconds\n\n", LOOPCNT, LOOPCNT-fail, fail, time_spent);

	printf("Stress testcases2 passed!\n");
	return 0;
}
