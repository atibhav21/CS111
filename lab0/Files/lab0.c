c/*
NAME: Atibhav Mittal
EMAIL: atibhav.mittal6@gmail.com
ID: 804598987
*/

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define STDIN_FILEDES 0
#define STDOUT_FILEDES 1

extern int errno ;
extern char* optarg;

void on_segfault(int x)
{
	fprintf(stderr, "Segfault caught and recieved\nError Code: %d\n", x);
	exit(4);
}

void create_segfault()
{
	char* temp = NULL;
	*temp = 'a'; // dereferencing a null pointer should cause a segfault
}

void doFileRedirection(int input_filedes, int output_filedes)
{
	char ch;
	while(read(input_filedes, &ch, 1) != 0) { // repeat until EOF is encountered 
		write(output_filedes, &ch, 1); // write to the output whatever is being read as input
	}
}

void printCorrectUsage()
{
	printf("usage: lab0 [options]\n");
	printf("options: --input=file1, Read from file1, --output=file2, Write to file2, --segfault Create a segfault, --catch Handle segfault\n");
}

int main(int argc, char *argv[])
{
	int c;
	static struct option long_options[] = 
			{
				{"input", required_argument, 0, 'i'},
				{"output", required_argument, 0, 'o'},
				{"segfault", no_argument, 0, 's'},
				{"catch", no_argument, 0, 'c'},
				{0, 0, 0, 0}
			};
	int option_index = 0;

	c = getopt_long(argc, argv, "", long_options, &option_index);	
	
	int segfault_needed = 0; // booleans to check if option is inputted to program or not
	int segfault_handler = 0;
	char* input_filename = NULL;
	char* output_filename = NULL;
	while(c != -1)
	{	

		switch(c)
		{
			case 'i': 
				//printf("option -i with value %s\n", optarg);
				input_filename = optarg;
				break;
			case 'o':
				//printf("option -o with value %s\n", optarg);
				output_filename = optarg;
				break;
			case 's':
				segfault_needed = 1;
				break;
			case 'c':
				segfault_handler = 1;
				break;
			default:
				// Other unrecognized option, exit with error
				printCorrectUsage();
				exit(1);
		}

		c = getopt_long(argc, argv, "", long_options, &option_index);	
	}
	if(output_filename) {
		// redirect stdout to output file

		int o_filedes = open(output_filename, O_CREAT | O_WRONLY, S_IRWXU); // create the output file specified with read, write, execute user permissions
		if(o_filedes < 0) {
			// error in opening the file
			fprintf( stderr, "--output caused the error\n");
			fprintf( stderr, "could not create file: %s\n", output_filename);
			fprintf( stderr, "Failed to create output file: %s\n", strerror(errno) );
			exit(3);
		}
		else if(o_filedes > 0) {
			// no error, so redirect stdout to output file
			if(close(1) < 0) 
			{
				fprintf(stderr, "Error closing stdout: %s\n", strerror(errno));
				exit(3);
			}
			if(dup(o_filedes) < 0)
			{
				fprintf(stderr, "Error duplicating file descriptor: %s\n", strerror(errno));
				exit(3);
			}
			
			if(close(o_filedes) < 0)
			{
				fprintf(stderr, "Error closing file descriptor for output file: %s\n", strerror(errno));
				exit(3);
			}
		}
		
	}
	if(input_filename) {
		// redirect stdin to input file
		int i_filedes = open(input_filename, O_RDONLY); // open a filedescriptor to the required input file
		if(i_filedes < 0) {
			// error in opening the file
			fprintf(stderr, "--input caused the error, file:%s\n", input_filename);
			fprintf(stderr, "Failed to open input file: %s\n", strerror(errno));
			exit(2);
		}
		else if(i_filedes > 0) {
			// no error so redirect stdin to input file
			if(close(0) < 0)
			{
				fprintf(stderr, "Error closing stdin: %s\n", strerror(errno));
				exit(2);
			}
			if(dup(i_filedes) < 0)
			{
				fprintf(stderr, "Error duplicating file descriptor: %s\n", strerror(errno));
				exit(2);
			}
			if(close(i_filedes) < 0)
			{
				fprintf(stderr, "Error closing file descriptor for input file: %s\n", strerror(errno));
				exit(2);
			}
		}
	}

	if(segfault_handler) {
		// Register signal handler
		signal(SIGSEGV, on_segfault);
	}
	if(segfault_needed) {
		// create a segfault since the user specified this option
		create_segfault();
	}
	else {
		// no error so do the file redirection from stdin to stdout
		doFileRedirection(STDIN_FILEDES, STDOUT_FILEDES);
	}

	
	return 0;
}