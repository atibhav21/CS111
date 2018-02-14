#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "SortedList.h"

#define TRUE 1
#define FALSE 0

int opt_yield = 0;


void printUsageMessage()
{
	fprintf(stderr, "usage: ./lab2_list [--threads=num] [--iterations=num]\n");
}

void exitError()
{
	fprintf(stderr, "Error encountered: %s\n", strerror(errno));
	exit(1);
}

void setYieldOpts(char* yieldopts)
{
	int i;
	for(i = 0; i < strlen(yieldopts); i+= 1)
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

int main(int argc, char *argv[])
{
	int num_threads;
	int num_iterations;
	int yield = FALSE;
	char yieldopts[4] = "";

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


	return 0;
}