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
pthread_mutex_t lock;
int spin_lock_flag = FALSE;

pthread_t* threads;
SortedList_t* list;
SortedListElement_t* elements;

struct thread_info {
	SortedListElement_t* m_elements;
	int num_iterations;
	SortedList_t* list;
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

void freePointersAndExit()
{
	fprintf(stderr, "Corrupted List\n");
	free(threads);
	free(list);
	free(elements);

	exit(2);
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
	SortedList_t* list = t.list;
	int num_iterations = t.num_iterations;
	SortedListElement_t* elements = t.m_elements;

	//printf("Lock ID: %d\n", lock_type);

	int i = 0;
	for(i=0; i<num_iterations; i+= 1)
	{
		switch(lock_type)
		{
			case NO_LOCK: SortedList_insert(list, &elements[i]);
						break;
			case MUTEX: //printf("Mutex section\n");
						pthread_mutex_lock(&lock);
						SortedList_insert(list, &elements[i]);
						pthread_mutex_unlock(&lock);
						break;
			case SPIN_LOCK: 
						//TODO: 
						//printf("Spin Lock\n");
						while(__sync_lock_test_and_set(&spin_lock_flag, 1)); // spin
						SortedList_insert(list, &elements[i]);
						__sync_lock_release(&spin_lock_flag); // release the lock
						break;
		}
		
	}

	int list_length;

	switch(lock_type)
	{
		case NO_LOCK: list_length = SortedList_length(list);
					break;
		case MUTEX: pthread_mutex_lock(&lock);
					list_length = SortedList_length(list);
					pthread_mutex_unlock(&lock);
					break;
		case SPIN_LOCK: 
					//TODO: 
					while(__sync_lock_test_and_set(&spin_lock_flag, 1)); // spin
					list_length = SortedList_length(list);
					__sync_lock_release(&spin_lock_flag); // release the lock
					break;
	}

	//fprintf(stderr, "List Length: %d\n", list_length);
	//fprintf(stderr, "Num_Iterations: %d\n", num_iterations);

	if(list_length == -1)
	{
		free(arg);	
		freePointersAndExit();
	}

	for(i=0; i<num_iterations; i+=1)
	{
		SortedListElement_t* m_elemt;

		switch(lock_type)
		{
			case NO_LOCK: m_elemt = SortedList_lookup(list, elements[i].key);
						if(SortedList_delete(m_elemt) == 1)
							{
								// list has been corrupted
								//fprintf(stderr, "Corrupted List, Do something 2\n");
								free(arg);
								freePointersAndExit();
								
							}
						break;
			case MUTEX: pthread_mutex_lock(&lock);
						m_elemt = SortedList_lookup(list, elements[i].key);
						if(SortedList_delete(m_elemt) == 1)
						{
							// list has been corrupted
							//fprintf(stderr, "Corrupted List, Do something 3\n");
							free(arg);
							freePointersAndExit();
						}	
						pthread_mutex_unlock(&lock);
						break;
			case SPIN_LOCK: 
						//TODO: 
						while(__sync_lock_test_and_set(&spin_lock_flag, 1)); // spin
						m_elemt = SortedList_lookup(list, elements[i].key);
						if(SortedList_delete(m_elemt) == 1)
						{
							// list has been corrupted
							//fprintf(stderr, "Corrupted List, Do something 4\n");
							free(arg);
							freePointersAndExit();
						}
						__sync_lock_release(&spin_lock_flag); // release the lock
						break;
		}

		

		/*if(m_elemt == NULL)
		{
			free(arg);
			freePointersAndExit();
		}

		switch(lock_type)
		{
			case NO_LOCK: if(SortedList_delete(m_elemt) == 1)
							{
								// list has been corrupted
								//fprintf(stderr, "Corrupted List, Do something 2\n");
								free(arg);
								freePointersAndExit();
								
							}
							break;
			case MUTEX: pthread_mutex_lock(&lock);
						if(SortedList_delete(m_elemt) == 1)
						{
							// list has been corrupted
							//fprintf(stderr, "Corrupted List, Do something 3\n");
							free(arg);
							freePointersAndExit();
						}	
						pthread_mutex_unlock(&lock);
						break;
			case SPIN_LOCK: 
						//TODO: 
						while(__sync_lock_test_and_set(&spin_lock_flag, 1)); // spin
						if(SortedList_delete(m_elemt) == 1)
						{
							// list has been corrupted
							//fprintf(stderr, "Corrupted List, Do something 4\n");
							free(arg);
							freePointersAndExit();
						}
						__sync_lock_release(&spin_lock_flag); // release the lock
						break;
		}*/

		
	}

	free(arg);
	return NULL;
}

void printResult(char* test_name, int threads, int iterations, int num_lists, long long time )
{
	int num_operations = threads * iterations * 3;
	printf("%s,%d,%d,%d,%d,%llu,%llu\n", test_name, threads, iterations, num_lists, num_operations, time, time/num_operations);
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

		if(pthread_mutex_init(&lock, NULL) != 0)
		{
			printError();
			exit(1);
		}
	}

	
	list = (SortedList_t*) malloc(sizeof(SortedList_t));
	list->prev = list;
	list->next = list;

	int number_of_elements = num_threads * num_iterations;
	elements = (SortedListElement_t*) malloc(number_of_elements * sizeof(SortedListElement_t));


	// TODO: Assign Random keys to all elements, and initialize all pointers to null
	int i = 0;
	for(i = 0; i < number_of_elements; i+= 1)
	{
		elements[i].key = generateRandomKey();
	}

	threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));


	if( clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		printError();
		free(list);
		free(elements);
		free(threads);
		exit(1);
	}

	for(i = 0; i < num_threads; i += 1)
	{
		struct thread_info* x = (struct thread_info* ) malloc(sizeof(struct thread_info));
		x->m_elements = &elements[i*num_iterations];
		x->num_iterations = num_iterations;
		x->list = list;
		if(pthread_create(&threads[i], NULL, &threadStuff, (void*) x) != 0)
		{
			printError();
			free(list);
			free(elements);
			free(threads);
			exit(1);
		}
	}
	
	for(i = 0; i < num_threads; i+= 1)
	{
		if(pthread_join(threads[i], NULL) != 0) 
		{
			printError();
			free(list);
			free(elements);
			free(threads);
			exit(1);
		}
	}
	//fprintf(stderr, "Reached Here\n");
	if(clock_gettime(CLOCK_MONOTONIC, &end) == -1)
	{
		printError();
		free(list);
		free(elements);
		exit(1);
	}

	if(list->next != list || list->prev != list)
	{
		free(threads);
		free(list); // Free the list 
		free(elements); // Free the assigned elements
		return 2;
	}

	long long my_elapsed_time_in_ns = (end.tv_sec - start.tv_sec) * 1000000000 ;
	my_elapsed_time_in_ns += end.tv_nsec;
	my_elapsed_time_in_ns -= start.tv_nsec;

	printResult(test_name, num_threads, num_iterations, 1, my_elapsed_time_in_ns);

	for(i= 0; i < number_of_elements; i+= 1)
	{
		free((void*) elements[i].key);
	}

	free(threads);
	free(list); // Free the list 
	free(elements); // Free the assigned elements
	return 0;
}