// NAME: Atibhav Mittal
// EMAIL: atibhav.mittal6@gmail.com
// ID: 804598987

#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

extern int errno;
extern char* optarg;

struct termios old_settings;

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define TRUE 1
#define FALSE 0

void printError(char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(1);
}

void printErrorAndReset(char *message)
{
	fprintf(stderr, "%s\n", message);
	tcsetattr(1, TCSANOW, &old_settings);
	exit(1);
}

void printUsageMessage()
{
	fprintf(stderr, "usage: ./lab1b --port=num --log=filename --compress\n");
}

void setTerminalSettings(struct termios* settings)
{
	settings->c_iflag = ISTRIP;	/* only lower 7 bits	*/
	settings->c_oflag = 0;		/* no processing	*/
	settings->c_lflag = 0;		/* no processing	*/

	if(tcsetattr(STDOUT_FILENO, TCSANOW, settings) == -1)
	{
		printError(strcat("Error in setting terminal settings: ", strerror(errno)));
	}

}

void processInput(int port_num, int log_fd)
{
	char buff[10];
	int num_read;
	int eof_received = FALSE; // TODO: remove
	while(! eof_received)
	{
		if((num_read = read(STDIN_FILENO, &buff, 10)) == -1)
		{
			printErrorAndReset(strcat("Error while reading input: ", strerror(errno)));
		}
		int i;
		for(i = 0; i < num_read; i+= 1)
		{
			// TODO: Change
			if(buff[i] == 4) // ^D received so exit as of now
			{
				eof_received = TRUE;
				break;
			}
			else if(buff[i] == '\r' || buff[i] == '\n')
			{
				if(write(STDOUT_FILENO, "\r\n", 2) == -1 || write(log_fd, "\r\n", 2) == -1)
				{
					printErrorAndReset(strcat("Error while trying to write: ", strerror(errno)));
				}
			}
			else
			{
				if(write(STDOUT_FILENO, &buff[i], 1) == -1 || write(log_fd, &buff[i], 1) == -1)
				{
					printErrorAndReset(strcat("Error while trying to write: ", strerror(errno)));
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	/* code */
	static struct option long_options[] = 
				{
					{"port", required_argument, 0, 'p'},
					{"log", required_argument, 0, 'l'},
					{"compress", no_argument, 0, 'c'},
					{0, 0, 0, 0}
				};

	int c;
	int option_index; 
	char* log_filename;
	int port_num;
	//int compress_mode = FALSE;
	while(1)
	{
		c = getopt_long(argc, argv, "", long_options, &option_index);
		if(c == -1)
		{
			break;
		}
		else
		{
			switch(c)
			{
				case 'p': port_num = atoi(optarg);
							break;
				case 'l': log_filename = optarg;
							break;
				/*case 'c': compress_mode = TRUE;
							break;*/
				default:
					printUsageMessage();
					exit(1);
			}
		}
	}
	// TODO: Check if preemptive exit if any argument is not passed!!

	if(tcgetattr(STDOUT_FILENO, &old_settings) == -1)
	{
		printError(strcat("Error in getting terminal settings: ", strerror(errno)));
	}


	struct termios new_settings = old_settings;
	setTerminalSettings(&new_settings); // set the terminal settings to non-canonical no-echo mode

	// TODO: take in input
	int log_fd = open(log_filename, O_CREAT | O_WRONLY| O_TRUNC, S_IRWXU);

	fprintf(stdout, "reached this point\r\n");
	if(log_fd == -1)
	{
		printErrorAndReset(strcat("Could not open logfile: ", strerror(errno)));
	}

	processInput(port_num, log_fd);


	if(close(log_fd) == -1)
	{
		printErrorAndReset(strcat("Could not close logfile: ", strerror(errno)));
	}
	// reset terminal before quitting!!
	if(tcsetattr(STDOUT_FILENO, TCSANOW, &old_settings) == -1)
	{
		printError(strcat("Error in setting terminal settings: ", strerror(errno)));
	}
	return 0;
}

