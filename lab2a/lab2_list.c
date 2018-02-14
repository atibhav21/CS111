#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "SortedList.h"
#include <errno.h>
#include <time.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

int opt_yield = 0;
extern int errno;

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

void* threadStuff(void* arg)
{
	struct thread_info t = *((struct thread_info*) arg);

	// perform the required computations
	SortedList_t* list = t.list;
	int num_iterations = t.num_iterations;
	SortedListElement_t* elements = t.m_elements;

	int i = 0;
	for(i=0; i<num_iterations; i+= 1)
	{
		SortedList_insert(list, &elements[i]);
	}

	int list_length = SortedList_length(list);
	if(list_length < num_iterations)
	{
		fprintf(stderr, "Corrupted List, Do something here\n");
	}

	for(i=0; i<num_iterations; i+=1)
	{
		SortedListElement_t* m_elemt = SortedList_lookup(list, elements[i].key);

		if(SortedList_delete(m_elemt) == 1)
		{
			// list has been corrupted
			fprintf(stderr, "Corrupted List, Do something here\n");
			break;
		}
	}

	free(arg);
	return NULL;
}

void printResult()
{

}

int main(int argc, char *argv[])
{
	int num_threads = 1;
	int num_iterations = 1;
	char yieldopts[4] = "";

	struct timespec start,end;

	static struct option long_options[] = {
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{"yield", required_argument, 0, 'y'},
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
					strcpy(yieldopts, optarg);
					break;
			default:
				printUsageMessage();
				exit(1);
		}

		c = getopt_long(argc, argv, "", long_options, NULL);
	}

	if(opt_yield)
	{ 
		// set the opt_yield options
		opt_yield = 0;
		setYieldOpts(yieldopts);
	}

	SortedList_t* list = (SortedList_t*) malloc(sizeof(SortedList_t));
	list->prev = list;
	list->next = list;

	int number_of_elements = num_threads * num_iterations;
	SortedListElement_t* elements = (SortedListElement_t*) malloc(number_of_elements * sizeof(SortedListElement_t));


	// TODO: Assign Random keys to all elements, and initialize all pointers to null
	int i = 0;
	for(i = 0; i < number_of_elements; i+= 1)
	{
		elements[i].key = generateRandomKey();
	}

	pthread_t* threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));


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

	printf("Reached A\n");
	
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
	printf("Reached B\n");

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

	printResult();

	/*for(i= 0; i < number_of_elements; i+= 1)
	{
		free((void*) elements[i].key);
	}*/

	free(threads);
	free(list); // Free the list 
	free(elements); // Free the assigned elements
	return 0;
}