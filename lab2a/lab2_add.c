#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define NO_LOCK 0
#define MUTEX 1
#define SPIN_LOCK 2
#define COMPARE_AND_SWAP 3

#define TRUE 1
#define FALSE 0

extern char* optarg;
extern int errno;

long long counter = 0;
int lock_type = NO_LOCK;
int opt_yield = FALSE;

pthread_mutex_t lock;

void printUsageMessage()
{
	fprintf(stderr, "usage: ./lab2_add [--threads=num] [--iterations=num]\n");
}

void exitError()
{
	fprintf(stderr, "Error encountered: %s\n", strerror(errno));
	exit(1);
}

/*void exitError(char* message)
{
	fprintf(stderr, "%s\n", message);
	exit(1);
}*/

void printResult(char* test_name, int num_threads, int num_iterations, long long timeInNs)
{
	int total_num_operations = num_threads * num_iterations * 2;
	printf("%s,%d,%d,%d,%llu,%llu,%llu\n",test_name, num_threads, 
		num_iterations, total_num_operations ,timeInNs, timeInNs/ total_num_operations, counter);
}

void add(long long *pointer, long long value) {
   long long sum = *pointer + value;
	if (opt_yield)
	        sched_yield();
	*pointer = sum;
}

void cas_add(long long *ptr, long long value) {
	long long prev, newval;
	do {
		prev = *ptr;
		newval = prev + value;
		if(opt_yield)
			sched_yield();
	} while(__sync_val_compare_and_swap(ptr, prev, newval) != prev);
}

void* addNumIterations(void* num_iterations)
{
	int* num = (int *) num_iterations;
	int i;

	for(i = 0; i < *num; i+= 1)
	{
		switch(lock_type)
		{
			case NO_LOCK: add(&counter, 1);
						break;
			case MUTEX: 
						pthread_mutex_lock(&lock);
						add(&counter, 1);
						pthread_mutex_unlock(&lock);
						break;
			case SPIN_LOCK:
						while(__sync_lock_test_and_set(&counter, 1)); // spin
						add(&counter, 1); // change counter
						__sync_lock_release(&counter); // release the lock
						break;
			case COMPARE_AND_SWAP:
						// TODO
						cas_add(&counter, 1);
						break;	
			default:
				exitError();
		}
	}
	for(i = 0; i < *num; i+= 1)
	{
		switch(lock_type)
		{
			case NO_LOCK: add(&counter, -1);
						break;
			case MUTEX:
						pthread_mutex_lock(&lock);
						add(&counter, -1);
						pthread_mutex_unlock(&lock);
						break;
			case SPIN_LOCK:
						while(__sync_lock_test_and_set(&counter, 1));
						add(&counter, -1);
						__sync_lock_release(&counter);
						break;
			case COMPARE_AND_SWAP:
						// TODO
						cas_add(&counter, -1);
						break;
			default:
				 exitError();
		}
	}

	return NULL;
}


int main(int argc, char *argv[])
{
	/* code */
	int num_threads = 1;
	int num_iterations = 1;
	int synchronization = FALSE;

	char test_name[20] = "add-none";
	int lock_char;
	//int option_index; // useless variable for the getopt_long call

	struct timespec start,end; 

	static struct option long_options[] = {
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{"yield", no_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	char c = getopt_long(argc, argv, "", long_options, NULL);
	while(c != -1)
	{
		switch(c) {
			case 't': num_threads = atoi(optarg);
					break;
			case 'i': num_iterations = atoi(optarg);
					break;
			case 'y': opt_yield = TRUE;
					  strcpy(test_name, "add-yield-none");
					break;
			case 's': synchronization = TRUE;
			 		// TODO: Possibly check for length of optarg
					lock_char = (char) optarg[0];
					break;
			default:
				printUsageMessage();
				exit(1);
		}

		c = getopt_long(argc, argv, "", long_options, NULL);
	}

	if(synchronization)
	{
		//printf("Lock status: %c", lock_char );
		switch(lock_char)
		{
			case 's': lock_type = SPIN_LOCK;
					if(opt_yield)
					{
						strcpy(test_name, "add-yield-s");
					}
					else
					{
						strcpy(test_name, "add-s");
					}
					break;
			case 'm': lock_type = MUTEX;
					if(pthread_mutex_init(&lock, NULL) != 0)
					{
						exitError();
					}
					if(opt_yield)
					{
						strcpy(test_name, "add-yield-m");
					}
					else
					{
						strcpy(test_name, "add-m");
					}
					break;
			case 'c': lock_type = COMPARE_AND_SWAP;
					if(opt_yield)
					{
						strcpy(test_name, "add-yield-c");
					}
					else
					{
						strcpy(test_name, "add-c");
					}
					break;
			default:
				printUsageMessage();
				exit(1);
		}
	}
	

	pthread_t* threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));

	if( clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		exitError();
	}

	int i;
	for(i = 0; i < num_threads; i += 1)
	{
		if(pthread_create(&threads[i], NULL, &addNumIterations, (void *) &num_iterations) != 0)
		{
			exitError();
		}
	}

	for(i = 0; i < num_threads; i += 1)
	{
		if(pthread_join(threads[i], NULL) != 0)
		{
			exitError();
		}
	}

	if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
	{
		exitError();
	}

	free(threads);

	printResult(test_name, num_threads, num_iterations, (long long) end.tv_nsec - start.tv_nsec);

	return 0;
}