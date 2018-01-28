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
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

extern int errno;
extern char* optarg;

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define TRUE 1
#define FALSE 0

#define BUFF_SIZE 256

void printError(char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(1);
}

void printUsageMessage()
{
	fprintf(stderr, "usage: ./lab1b-server --port=num [--compress]\n");
}

// pass in the port num on which to listen
// new socket file descriptor is passed in as reference so that data can be read from it
void setupSocket(int* newsockfd, int port_num)
{
	int sockfd;
	unsigned int clilen;
	struct sockaddr_in serv_addr, cli_addr;
	//int n;

	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(sockfd < 0)
		printError(strcat("Error while opening socket: ", strerror(errno)));

	memset((char *) &serv_addr, 0,  sizeof(serv_addr)); // initialize to 0s

	serv_addr.sin_family = AF_LOCAL;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_num);
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printError(strcat("Error while trying to bind socket: ", strerror(errno)));
	}
	if(listen(sockfd, 5) < 0)
	{
		printError(strcat("Error while trying to listen to socket", strerror(errno)));
	}
	clilen = sizeof(cli_addr);
	*newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if(*newsockfd < 0) 
	{
		printError(strcat("Error while trying to accept connection", strerror(errno)));
	}
	//char buff[BUFF_SIZE];
}

// newsockfd is the file descriptor for the socket
void processInput(int newsockfd)
{
	char buff[BUFF_SIZE];
	int num_read;
	while(1)
	{
		num_read = read(newsockfd, &buff, BUFF_SIZE);
		if(num_read < 0)
		{
			printError(strcat("Error while trying to read", strerror(errno)));
		}
		else if(num_read == 0)
		{
			//End of file received
			break;
		}
		if(write(STDOUT_FILENO, &buff, num_read) < 0)
		{
			printError(strcat("Error while trying to write to stdout", strerror(errno)));
		}
		/*else
		{
			fprintf(stderr, "Sucessfully read and Wrote input from socket!!!\n");
		}*/

	}
}

int main(int argc, char *argv[])
{
	/* code */
	static struct option long_options[] = 
			{
				{"port", required_argument, 0, 'p'},
				{"compress", no_argument, 0, 'c'},
				{0, 0, 0, 0}
			};

	int c;
	int port_set = FALSE;
	int port_num;
	int option_index;
	int compress_mode = FALSE;
	while(1)
	{
		c = getopt_long(argc, argv, "", long_options, &option_index);
		if(c == -1)
		{
			break;
		}
		switch(c)
		{
			case 'p': 	port_set = TRUE;
						port_num = atoi(optarg);
						break;
			case 'c':	compress_mode = TRUE;
						break;
			default:
						printUsageMessage();
						exit(1);
		}
	}

	if(!port_set)
	{
		printUsageMessage();
		exit(1);
	}
	
	int newsockfd;
	setupSocket(&newsockfd, port_num);
	processInput(newsockfd);
	return 0;
}