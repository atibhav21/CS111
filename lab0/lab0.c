#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define STDIN_FILEDES 0
#define STDOUT_FILEDES 1

extern int errno ;

void create_segfault()
{
	char* temp = NULL;
	*temp = 'a';
}

void doFileRedirection(int input_filedes, int output_filedes)
{
	char ch;
	while(read(input_filedes, &ch, 1) != 0) {
		write(output_filedes, &ch, 1);
	}
}

void printCorrectUsage()
{
	printf("Option not recognized\n");
	printf("usage: lab0 [options]\n");
	printf("options: --input=file1, Read from file1, --output=file2, Read from file2, --segfault Create a segfault, --catch Handle segfault\n");
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
	char* input_filename;
	char* output_filename;
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

	/*if(segfault_needed && ! segfault_handler) {
		create_segfault();
	}
	else if(segfault_needed && segfault_handler) {
		// handle the segfault
		// TODO: Figure out how segfault handlers work
		printf("Handle this segfault baby\n");
	}
	else {
		/*if(! input_filename && ! output_filename) {
			// redirect from stdin to stdout
			//doFileRedirection(STDIN_FILEDES, STDOUT_FILEDES);
		}
		else*/
	if(output_filename) {
		// redirect stdout to output file

		int o_filedes = open(output_filename, O_CREAT | O_WRONLY, S_IRWXU);
		if(o_filedes < 0) {
			//TODO: CHANGE TO fprintf
			fprintf( stderr, "Failed to create output file: %s\n", strerror(errno) );
			exit(3);
		}
		else if(o_filedes > 0) {
			close(1);
			dup(o_filedes);
			close(o_filedes);
			/*doFileRedirection(STDIN_FILEDES, o_filedes);
			if(close(o_filedes) < 0) {
				fprintf(stderr, "Failed to close output file: %s\n", strerror(errno) );
				exit(3);
			}*/
		}
		
	}
	if(input_filename) {
		// redirect stdin to input file
		int i_filedes = open(input_filename, O_RDONLY);
		if(i_filedes < 0) {
			fprintf(stderr, "Failed to open input file: %s\n", strerror(errno));
			exit(2);
		}
		else if(i_filedes > 0) {
			close(0);
			dup(i_filedes);
			close(i_filedes);
		}
		/*doFileRedirection(i_filedes, STDOUT_FILEDES);
		int close_error = close(i_filedes);
		if(close_error < 0) {
			fprintf(stderr, "Failed to close input file: %s\n", strerror(errno));
			exit(2);
		}*/
	}
	if(segfault_needed && ! segfault_handler) {
		create_segfault();
	}
	else if(segfault_needed && segfault_handler) {
		// handle the segfault
		// TODO: Figure out how segfault handlers work
		printf("Handle this segfault baby\n");
	}
	else {
		doFileRedirection(STDIN_FILEDES, STDOUT_FILEDES);
	}
		/*else{
			// both input and output files provided so do file redirection with those
			int i_filedes = open(input_filename, O_RDONLY);
			if(i_filedes < 0) {
				fprintf(stderr, "Failed to open input file: %s\n", strerror(errno));
				exit(2);
			}
			if(o_filedes < 0) {
				//TODO: CHANGE TO fprintf
				close(i_filedes);
				fprintf( stderr, "Failed to create output file: %s\n", strerror(errno) );
				exit(3);
			}
			int o_filedes = open(output_filename, O_CREAT | O_WRONLY, S_IRWXU);
			doFileRedirection(i_filedes, o_filedes);
			if(close(i_filedes) < 0) {
				fprintf(stderr, "Failed to close input file: %s\n", strerror(errno));
				exit(2);
			}
			if(close(o_filedes) < 0) {
				fprintf(stderr, "Failed to close output file: %s\n", strerror(errno) );
				exit(3);
			} 
		}*/

	
	return 0;
}