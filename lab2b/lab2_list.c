// NAME: Atibhav Mittal
// EMAIL: atibhav.mittal6@gmail.com
// ID: 804598987

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "SortedList.h"
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#define TRUE 1
#define FALSE 0

#define NO_LOCK 0
#define MUTEX 1
#define SPIN_LOCK 2

int opt_yield = 0;
extern int errno;
int lock_type = NO_LOCK;
pthread_mutex_t* locks;
int* spin_lock_flags;
int num_lists = 1;

pthread_t* threads;
SortedList_t* list; // Contains all the list header elements
SortedListElement_t* elements;
long long* total_thread_wait_time;
int number_of_elements;

struct thread_info {
	SortedListElement_t* m_elements;
	int num_iterations;
	//SortedList_t* list;
	int thread_num;
};

void printUsageMessage()
{
	fprintf(stderr, "usage: ./lab2_list [--threads=num] [--iterations=num]\n");
}

void printError()
{
	if(errno == 0)
	{
		fprintf(stderr, "Unknown Error Encountered\n");
	}
	else
	{
		fprintf(stderr, "Error encountered: %s\n", strerror(errno));
	}
	
}

// hash function found on stack overflow: https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long hash(const char* str)
{
	unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void freePointersAndExit()
{
	fprintf(stderr, "Corrupted List\n");
	int i;
	for(i= 0; i < number_of_elements; i+= 1)
	{
		free((void*) elements[i].key);
	}
	free(total_thread_wait_time);
	free(threads);
	free(list);
	free(elements);
	switch(lock_type)
	{
		case MUTEX: free(locks);
					break;
		case SPIN_LOCK: free(spin_lock_flags);
					break;
		default: break;
	}
	exit(2);
}

void freePointers()
{
	int i;
	for(i= 0; i < number_of_elements; i+= 1)
	{
		free((void*) elements[i].key);
	}
	free(total_thread_wait_time);
	free(list);
	free(elements);
	free(threads);
	switch(lock_type)
	{
		case MUTEX: free(locks);
					break;
		case SPIN_LOCK: free(spin_lock_flags);
					break;
		default: break;
	}
}

char* generateRandomKey()
{
	char* temp = malloc(1024 * sizeof(char));
	int j = 0;
	for(j = 0; j < 1023; j+= 1)
	{
		int random_number = rand() % 256;
		temp[j] = (char) random_number;
	}
	temp[1023] = '\0';
	return temp;
}

void setYieldOpts(char* yieldopts)
{
	int i;
	for(i = 0; i < (signed) strlen(yieldopts); i+= 1)
	{
		if(yieldopts[i] == 'i')
		{
			opt_yield = opt_yield | 0x01;
		}
		else if(yieldopts[i] == 'd')
		{
			opt_yield = opt_yield | 0x02;
		}
		else if(yieldopts[i] == 'l')
		{
			opt_yield = opt_yield | 0x04;
		} 
	}
}

void segfaultHandler(int x)
{
	fprintf(stderr, "Segmentation Fault occured: %d\n", x);
	freePointersAndExit();
	exit(2);
}

void* threadStuff(void* arg)
{
	struct thread_info t = *((struct thread_info*) arg);

	// perform the required computations
	//SortedList_t* list = t.list;
	int num_iterations = t.num_iterations;
	SortedListElement_t* elements = t.m_elements;
	int thread_num = t.thread_num;

	//printf("Lock ID: %d\n", lock_type);

	long long thread_time_in_ns = 0;
	struct timespec start,end;

	SortedList_t* list_to_work_with = NULL;

	int i = 0;
	for(i=0; i<num_iterations; i+= 1)
	{	
		int list_index = (int) (hash(elements[i].key) % num_lists);
		list_to_work_with = &list[list_index];
		switch(lock_type)
		{
			case NO_LOCK: SortedList_insert(list_to_work_with, &elements[i]);
						break;
			case MUTEX: //printf("Mutex section\n");
						if(clock_gettime(CLOCK_MONOTONIC, &start) != 0)
						{
							printError();
							freePointersAndExit();
						}
						pthread_mutex_lock(&locks[list_index]);
						if(clock_gettime(CLOCK_MONOTONIC, &end) != 0)
						{
							pthread_mutex_unlock(&locks[list_index]);
							printError();
							freePointersAndExit();
						}
						SortedList_insert(list_to_work_with, &elements[i]);
						pthread_mutex_unlock(&locks[list_index]);
						thread_time_in_ns += (end.tv_sec - start.tv_sec) * 1000000000 ;
						thread_time_in_ns += end.tv_nsec;
						thread_time_in_ns -= start.tv_nsec;
						break;
			case SPIN_LOCK: 
						//TODO: 
						//printf("Spin Lock\n");
						if(clock_gettime(CLOCK_MONOTONIC, &start) != 0)
						{
							printError();
							freePointersAndExit();
						}
						while(__sync_lock_test_and_set(&spin_lock_flags[list_index], 1)); // spin
						if(clock_gettime(CLOCK_MONOTONIC, &end) != 0)
						{
							__sync_lock_release(&spin_lock_flags[list_index]); // release the lock
							printError();
							freePointersAndExit();
						}
						SortedList_insert(list_to_work_with, &elements[i]);
						__sync_lock_release(&spin_lock_flags[list_index]); // release the lock
						thread_time_in_ns += (end.tv_sec - start.tv_sec) * 1000000000 ;
						thread_time_in_ns += end.tv_nsec;
						thread_time_in_ns -= start.tv_nsec;
						break;
		}
		
	}


	// TODO: CHANGE THE LOCK IMPLEMENTATIONS TO TRY AND ACQUIRE ALL THE LOCKS!!
	int list_length = 0;
	//int i;
	switch(lock_type)
	{
		case NO_LOCK: 
					for(i = 0; i < num_lists; i += 1)
					{
						list_length += SortedList_length(&list[i]);
					}
					//list_length = SortedList_length(list_to_work_with);
					break;
		case MUTEX: 
					for(i = 0; i < num_lists; i += 1)
					{
						if(clock_gettime(CLOCK_MONOTONIC, &start) != 0)
						{
							printError();
							freePointersAndExit();
						}
						pthread_mutex_lock(&locks[i]);
						if(clock_gettime(CLOCK_MONOTONIC, &end) != 0)
						{
							pthread_mutex_unlock(&locks[i]);
							printError();
							freePointersAndExit();
						}
						thread_time_in_ns += (end.tv_sec - start.tv_sec) * 1000000000 ;
						thread_time_in_ns += end.tv_nsec;
						thread_time_in_ns -= start.tv_nsec;
					}
					for(i = 0; i < num_lists; i += 1)
					{
						list_length += SortedList_length(&list[i]);
						pthread_mutex_unlock(&locks[i]);
					}
					
					break;
		case SPIN_LOCK: 
					//TODO: 
					for(i = 0; i < num_lists; i += 1)
					{
						if(clock_gettime(CLOCK_MONOTONIC, &start) != 0)
						{
							printError();
							freePointersAndExit();
						}
						while(__sync_lock_test_and_set(&spin_lock_flags[i], 1)); // spin
						if(clock_gettime(CLOCK_MONOTONIC, &end) != 0)
						{
							__sync_lock_release(&spin_lock_flags[i]); // release the lock
							printError();
							freePointersAndExit();
						}
						thread_time_in_ns += (end.tv_sec - start.tv_sec) * 1000000000 ;
						thread_time_in_ns += end.tv_nsec;
						thread_time_in_ns -= start.tv_nsec;
					}
					for(i = 0; i < num_lists; i+= 1)
					{
						list_length += SortedList_length(list_to_work_with);
						__sync_lock_release(&spin_lock_flags[i]); // release the lock
					}
					

					
					break;
	}




	//fprintf(stderr, "List Length: %d\n", list_length);
	//fprintf(stderr, "Num_Iterations: %d\n", num_iterations);

	if(list_length <= -1)
	{
		free(arg);	
		freePointersAndExit();
	}

	for(i=0; i<num_iterations; i+=1)
	{
		SortedListElement_t* m_elemt;
		int list_index = (int) (hash(elements[i].key) % num_lists);
		list_to_work_with = &list[list_index];
		switch(lock_type)
		{
			case NO_LOCK: m_elemt = SortedList_lookup(list_to_work_with, elements[i].key);
						if(SortedList_delete(m_elemt) == 1)
							{
								// list has been corrupted
								//fprintf(stderr, "Corrupted List, Do something 2\n");
								free(arg);
								freePointersAndExit();
								
							}
						break;
			case MUTEX: 
						if(clock_gettime(CLOCK_MONOTONIC, &start) != 0)
						{
							printError();
							freePointersAndExit();
						}
						pthread_mutex_lock(&locks[list_index]);
						if(clock_gettime(CLOCK_MONOTONIC, &end) != 0)
						{
							pthread_mutex_unlock(&locks[list_index]);
							printError();
							freePointersAndExit();
						}
						m_elemt = SortedList_lookup(list_to_work_with, elements[i].key);
						if(SortedList_delete(m_elemt) == 1)
						{
							// list has been corrupted
							//fprintf(stderr, "Corrupted List, Do something 3\n");
							free(arg);
							freePointersAndExit();
						}	
						pthread_mutex_unlock(&locks[list_index]);
						thread_time_in_ns += (end.tv_sec - start.tv_sec) * 1000000000 ;
						thread_time_in_ns += end.tv_nsec;
						thread_time_in_ns -= start.tv_nsec;
						break;
			case SPIN_LOCK: 
						//TODO: 
						if(clock_gettime(CLOCK_MONOTONIC, &start) != 0)
						{
							printError();
							freePointersAndExit();
						}
						while(__sync_lock_test_and_set(&spin_lock_flags[list_index], 1)); // spin
						if(clock_gettime(CLOCK_MONOTONIC, &end) != 0)
						{
							__sync_lock_release(&spin_lock_flags[list_index]); // release the lock
							printError();
							freePointersAndExit();
						}
						m_elemt = SortedList_lookup(list_to_work_with, elements[i].key);
						if(SortedList_delete(m_elemt) == 1)
						{
							// list has been corrupted
							//fprintf(stderr, "Corrupted List, Do something 4\n");
							free(arg);
							freePointersAndExit();
						}
						__sync_lock_release(&spin_lock_flags[list_index]); // release the lock
						thread_time_in_ns += (end.tv_sec - start.tv_sec) * 1000000000 ;
						thread_time_in_ns += end.tv_nsec;
						thread_time_in_ns -= start.tv_nsec;
						break;
		}

		
	}
	total_thread_wait_time[thread_num] = thread_time_in_ns;

	free(arg);
	return NULL;
}

void printResult(char* test_name, int threads, int iterations, int num_lists, long long time, long long wait_time)
{
	int num_operations = threads * iterations * 3;
	printf("%s,%d,%d,%d,%d,%llu,%llu,%llu\n", test_name, threads, iterations, num_lists, num_operations, time, time/num_operations, wait_time);
}

int main(int argc, char *argv[])
{
	int num_threads = 1;
	int num_iterations = 1;
	char yieldopts[5] = "none";

	struct timespec start,end;

	char test_name[20] = "list-none-none";

	char lock_name[5] = "none";

	static struct option long_options[] = {
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{"yield", required_argument, 0, 'y'},
		{"lists", required_argument, 0, 'l'},
		{"sync", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	int needToChangeTestName = FALSE;

	signal(SIGSEGV, segfaultHandler);

	char c = getopt_long(argc, argv, "", long_options, NULL);

	while(c != -1)
	{
		switch(c) {
			case 't': num_threads = atoi(optarg);
					break;
			case 'i': num_iterations = atoi(optarg);
					break;
			case 'y': 
					needToChangeTestName = TRUE;
					strcpy(yieldopts, optarg);
					break;
			case 's': 
					needToChangeTestName = TRUE;
					lock_name[0] = (char) optarg[0];
					lock_name[1] = '\0';
					if(lock_name[0] == 's')
					{
						lock_type = SPIN_LOCK;
					}
					else if(lock_name[0] == 'm')
					{
						lock_type = MUTEX;
					}
					break;
			case 'l':	
					num_lists = atoi(optarg);
					break;

			default:
				printUsageMessage();
				exit(1);
		}

		c = getopt_long(argc, argv, "", long_options, NULL);
	}

	if(needToChangeTestName)
	{ 
		// set the opt_yield options
		opt_yield = 0;
		strcpy(test_name, "list-");

		//test_name = strcat(strcat("list-", yieldopts), "-");
		strcpy(test_name, strcat(strcat(strcat(test_name, yieldopts), "-"), lock_name));
		setYieldOpts(yieldopts);

		switch(lock_type)
		{
			case MUTEX: 
				locks = (pthread_mutex_t*) malloc(num_lists * sizeof(pthread_mutex_t));
				int j;
				// Initialize all the locks for each thread!!
				for(j = 0; j < num_lists; j+= 1)
				{
					if(pthread_mutex_init(&locks[j], NULL) != 0)
					{
						free(locks);
						printError();
						exit(1);
					}
				}
				break;
			case SPIN_LOCK:
				spin_lock_flags = (int *) malloc(num_lists * sizeof(int));
				//int j;
				for(j = 0; j < num_lists; j += 1)
				{
					spin_lock_flags[j] = 0;
				}
				break;

		}
				

	}
	list = (SortedList_t*) malloc(sizeof(SortedList_t) * num_lists);


	number_of_elements = num_threads * num_iterations;
	elements = (SortedListElement_t*) malloc(number_of_elements * sizeof(SortedListElement_t));


	threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
	total_thread_wait_time = (long long*) malloc(num_threads * sizeof(long long));

	

	// TODO: Assign Random keys to all elements, and initialize all pointers to null
	int i = 0;
	for(i = 0; i < num_lists; i += 1)
	{
		list[i].prev = &list[i];
		list[i].next = &list[i];
	}
	for(i = 0; i < number_of_elements; i+= 1)
	{
		elements[i].key = generateRandomKey();
	}


	if( clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		printError();
		freePointers();
		exit(1);
	}

	for(i = 0; i < num_threads; i += 1)
	{
		struct thread_info* x = (struct thread_info* ) malloc(sizeof(struct thread_info));
		x->m_elements = &elements[i*num_iterations];
		x->num_iterations = num_iterations;
		//x->list = list;
		x->thread_num = i;
		if(pthread_create(&threads[i], NULL, &threadStuff, (void*) x) != 0)
		{
			printError();
			freePointers();
			exit(1);
		}
	}
	
	for(i = 0; i < num_threads; i+= 1)
	{
		if(pthread_join(threads[i], NULL) != 0) 
		{
			printError();
			freePointers();
			exit(1);
		}
	}
	//fprintf(stderr, "Reached Here\n");
	if(clock_gettime(CLOCK_MONOTONIC, &end) == -1)
	{
		printError();
		freePointers();
		exit(1);
	}

	if(list->next != list || list->prev != list)
	{
		freePointers(); // Free the assigned elements
		return 2;
	}

	long long my_elapsed_time_in_ns = (end.tv_sec - start.tv_sec) * 1000000000 ;
	my_elapsed_time_in_ns += end.tv_nsec;
	my_elapsed_time_in_ns -= start.tv_nsec;

	long long total_wait_time = 0;
	for(i = 0; i < num_threads; i+= 1)
	{
		total_wait_time += total_thread_wait_time[i];
	}


	printResult(test_name, num_threads, num_iterations, num_lists, my_elapsed_time_in_ns, total_wait_time);
	freePointers();

	return 0;
}