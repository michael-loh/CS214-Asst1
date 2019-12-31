
#include "mymalloc.h"
#include <sys/time.h>

// malloc() 1 byte and immediately free it - do this 150 times
void workload_a()
{
	int i;
	for(i = 0; i < 150; i++){
		char * testchar = malloc(1);
		free(testchar);
	}
}

//malloc() 1 byte, store the pointer in an array - do this 150 times.
//Once you've malloc()ed 50 byte chunks, then free() the 50 1 byte pointers one by on
void workload_b()
{
	int i;
	int j;
	char *arr[50];

	for(i = 0; i < 3; i++){
		for(j = 0; j < 50; j++){
			arr[j] = malloc(1);
		}
		for(j = 0; j < 50; j++){
			free(arr[j]);
		}
	}

}

/*
 * Randomly choose between a 1 byte malloc() or free()ing a 1 byte pointer
 *  > do this until you have allocated 50 times
 * Keep track of each operation so that you eventually malloc() 50 bytes, in total
 *  > if you have already allocated 50 times, disregard the random and just free() on each
 *    iteration
 * Keep track of each operation so that you eventually free() all pointers
 *  > don't allow a free() if you have no pointers to free()
 */
void workload_c()
{
	int mallocCount = 0;
	char * stack[50];
	int stackIndex = -1;

	while(mallocCount < 50){			//malloc 50 bytes
		int random = rand()%2;

		if(random == 1){				//if random == 1, malloc()
			stackIndex++;
			stack[stackIndex] = malloc(1);
			mallocCount++;
		}
		else{							//if random == 0, free()
			if(stackIndex == -1){			//if stack is empty, do not free
				continue;
			}
			free(stack[stackIndex]);
			stackIndex--;
		}

	}

	while(stackIndex >= 0){				//free all the malloc'ed memory
		free(stack[stackIndex]);
		stackIndex--;
	}

}

/*
 * Helper method for workload_d(). Used to get the size of the pointer that is being freed.
*/
int get_node_size(void * ptr)
{
	node_t * curr_node = ptr - sizeof(node_t);
	return curr_node->size + sizeof(node_t);
}

/*
 *  Randomly choose between a randomly-sized malloc() or free()ing a pointer – do this many
 *  times (see below)
 *  - Keep track of each malloc so that all mallocs do not exceed your total memory capacity
 *  - Keep track of each operation so that you eventually malloc() 50 times
 *  - Keep track of each operation so that you eventually free() all pointers
 *  - Choose a random allocation size between 1 and 64 bytes
 */
void workload_d()
{
	int mallocCount = 0;
	int stackIndex = -1;
	char * stack[50];

	int totalMemoryLeft = 4092;

	while(mallocCount < 50){
		int action = rand()%2;

//		printf("%d\n", totalMemoryLeft);
		if(action == 1){		//if action == 1, malloc()
			int bytes = rand()%64 + 1;		//get a random allocation size
			if(bytes + sizeof(node_t) > totalMemoryLeft){
				continue;
			}
			stackIndex++;
			stack[stackIndex] = malloc(bytes);
			totalMemoryLeft -= (bytes + sizeof(node_t));
			mallocCount++;
		}

		else{					//if action == 0, free()
			if(stackIndex == -1){
				continue;
			}
			totalMemoryLeft += get_node_size(stack[stackIndex]);
			free(stack[stackIndex]);
			stackIndex--;
		}
	}

	while(stackIndex >= 0){
//		printf("%d\n", totalMemoryLeft);
		totalMemoryLeft += get_node_size(stack[stackIndex]);
		free(stack[stackIndex]);
		stackIndex--;
	}

//	printf("%d\n", totalMemoryLeft);

}

void check_null_ptr(void *ptr) {
	if (ptr == NULL)
	{
		printf("Found NULL pointer!\n");
	}
}

/*
 * This workload attempt to randomly allocate amounts of memory between 0-4100 bytes, storing
 * the pointers in an array. Once malloc() has failed three times, free all the pointers. Finally,
 * free myblock - 1, first block of memory, and a NULL pointer.
 */
void workload_e()
{
	int fails = 0;
	char * arr[819];	//max possible number of pointers malloc'ed
	int arrIndex = 0;

	//allocate memory until malloc fails 3 times
	while(fails < 3){
		int random = rand()%4100 + 1;
		arr[arrIndex] = malloc(random);

		//if an error occurs with malloc, the pointer will be NULL
		if(arr[arrIndex] == NULL){
			fails++;
			continue;
		}
		arrIndex++;
	}

	//free pointers
	int i;
	for(i = 0; i < arrIndex; i++){
		free(arr[i]);
	}

	//check free(myblock - 1). Error due to trying to free memory not in heap
	free(myblock - 1);

	//check free(myblock[0]). Error due to trying to free a free'd block
	free(arr[0]);

	//check free(NULL pointer). Error due to trying to free a NULL pointer
	char* null_ptr = NULL;
	free(null_ptr);
	
	//check free(non pointer). Error due to trying to free something that isn't a pointer
	free(arr[0] + 1);
}

/*
 * Malloc 512 blocks of of 4 bytes. That would fill up all the available memory.
 * Then free 50 random blocks of memory, and then malloc 50 more blocks of memory.
 * This test shows how we don't lose memory when we free.
 */
void workload_f()
{
	//create an array that can store the pointers
	char * arr[512];
	int i;
	//malloc 4 bytes to every pointer.
	for(i = 0; i < 512; i++){
		arr[i] = malloc(4);
	}

	//find the 50 random blocks that we want to free
	int freeBlocks[50];
	for(i = 0; i < 50; i++){
		int block = rand()%512;
		int j;

		//check for duplicates. If duplicate, find a different pointer index
		bool cont = false;
		for(j = 0; j < 50; j++){
			if(freeBlocks[j] == block){
				 cont = true;
				 break;
			}
		}
		if(cont){
			i--;
			continue;
		}
		freeBlocks[i] = block;
	}

	//free the 50 blocks
	for(i = 0; i < 50; i++){
		free(arr[freeBlocks[i]]);
	}

	//malloc 50 more blocks, and see if it runs out of memory (which it shouldn't)
	for(i = 0; i < 50; i++){
		malloc(4);
	}

	//free everything
	for(i = 0; i < 512; i++){
		free(arr[i]);
	}
}

/*
 * Executes a function and returns its runtime
 * @param (*func)() A workload that takes in no parameters
 * @return Runtime of the function
 */
double timed_execution(void (*func)())
{
	struct timeval start, end;
	double time_elapsed;

	gettimeofday(&start, NULL);
	func();
	gettimeofday(&end, NULL);

	time_elapsed = (end.tv_sec - start.tv_sec) * 1e6;
	return (time_elapsed + (end.tv_usec - start.tv_usec)) * 1e-6;
}

/*
 * Calculates the mean value of all elements of an array.
 * @param *num_arr Array of values
 * @param arr_size Number of elements to average together
 * @return Average of all elements of *num_arr
 */
double calculate_avg(double *num_arr, int arr_size)
{
	double total = 0;

	int i;
	for (i = 0; i < arr_size; i++)
	{
		total += num_arr[i];
	}

	return total / arr_size;
}

/*
 * Prints out an array of average times for a series of workloads.
 * @param *workload_avgs Array of mean times it took for workloads to run
 * @param num_workloads Amount of workloads, up to 26 assuming workloads are identified alphabetically
 */
void print_avg_times(double *workload_avgs, short num_workloads)
{
	short i;
	for (i = 0; i < num_workloads; i++)
	{
		printf("Mean runtime for Workload %c: %f seconds\n", 'A' + i, workload_avgs[i]);
	}
}

int main(int argc, char *argv[])
{
	void (*workload_ptr_arr[])() = {workload_a, workload_b, workload_c, workload_d, workload_e, workload_f};
	double workload_times[100];
	double workload_avgs[6];

	short i, j;
	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 100; j++)
		{
			workload_times[j] = timed_execution(workload_ptr_arr[i]);
		}
		workload_avgs[i] = calculate_avg(workload_times, 100);
	}

	print_avg_times(workload_avgs, 6);

	return 0;
}
