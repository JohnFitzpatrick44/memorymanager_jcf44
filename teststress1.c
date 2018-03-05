#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "dmm.h"

int main(int argc, char *argv[])
{
	int size;
	void *ptr[10];
	int i;

	/*
 	 * try mallocing four pieces, each 1/4 of total size
 	 */
	size = MAX_HEAP_SIZE/ 4;

	ptr[0] = dmalloc(size);
	if(ptr[0] == NULL)
	{
		printf("malloc of ptr[0] failed for size %d\n", size);
		exit(1);
	}

	print_freelist();
	printf("\n");

	ptr[1] = dmalloc(size);
	if(ptr[1] == NULL)
	{
		printf("malloc of ptr[1] failed for size %d\n", size);
		exit(1);
	}

	print_freelist();
	printf("\n");

	ptr[2] = dmalloc(size);
	if(ptr[2] == NULL)
	{
		printf("malloc of ptr[2] failed for size %d\n", size);
		exit(1);
	}

	print_freelist();
	printf("\n");

	/*
  	 * this one should fail due to rounding
  	 */
	ptr[3] = dmalloc(size);
	if(ptr[3] == NULL)
	{
		printf("malloc of ptr[3] fails correctly for size %d\n", size);
	}

	print_freelist();
	printf("\n");

	/*
  	 * free the first block
  	 */
	dfree(ptr[0]);

	print_freelist();
	printf("\n");

	/*
  	 * free the third block
   	 */
	dfree(ptr[2]);

	print_freelist();
	printf("\n");


	/*
  	 * now free secoond block
   	 */
	dfree(ptr[1]);

	print_freelist();
	printf("\n");

	/*
 	 * re-malloc first pointer
  	 */
	ptr[0] = dmalloc(size);
	if(ptr[0] == NULL)
	{
		printf("re-malloc of ptr[0] failed for size %d\n", size);
		exit(1);
	}
	print_freelist();
	printf("\n");

	/*
  	 * try splitting the second block
  	 */
	ptr[1] = dmalloc(size/2);
	if(ptr[1] == NULL)
	{
		printf("split second block ptr[1] failed for size %d\n", size/2);
		exit(1);
	}
	print_freelist();
	printf("\n");

	/*
  	 * free first block and split of second
 	 */
	dfree(ptr[0]);
	dfree(ptr[1]);

	print_freelist();
	printf("\n");



	/*
 	 * try mallocing a little less to make sure no split occurs
   	 * first block from previous print should not be split
  	 */
	ptr[0] = dmalloc(size-1);
	if(ptr[0] == NULL)
	{
		printf("slightly smaller malloc of ptr[0] failed for size %d\n", size);
		exit(1);
	}

	/*
  	 * free it and make sure it comes back as the correct size
  	 */
	dfree(ptr[0]);
	
	print_freelist();
	printf("\n");

	/*
 	 * okay, now see if multiples work
  	 */
	for(i=0; i < 6; i++)
	{
		ptr[i] = dmalloc(100);
	}

	/*
 	 * free first block, third block, fifth block
  	 */
	dfree(ptr[0]);
	dfree(ptr[2]);
	dfree(ptr[4]);
	print_freelist();
	printf("\n");

	/*
 	 * now, free second block -- first, second, third blocks
   	 * should coalesce
	 */
	dfree(ptr[1]);
	print_freelist();
	printf("\n");

	/*
	 * free the sixth block and it shoudl merge with the last
  	 * block leaving two
 	 */
	dfree(ptr[5]);
	print_freelist();
	printf("\n");

	/*
  	 * now free fourth block and they should all be together
  	 */
	dfree(ptr[3]);
	print_freelist();
	printf("\n");

	printf("Stress testcases1 passed!\n");

	exit(0);
}
