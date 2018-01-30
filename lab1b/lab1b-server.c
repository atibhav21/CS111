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
#include <sys/wait.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <zlib.h>

extern int errno;
extern char* optarg;

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define TRUE 1
#define FALSE 0

#define BUFF_SIZE 256

#define READ_END 0
#define WRITE_END 1

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

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		printError(strcat("Error while opening socket: ", strerror(errno)));

	memset((char *) &serv_addr, 0,  sizeof(serv_addr)); // initialize to 0s


	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_num);
	
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) // TODO: bind function creates junk files
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
void processInput(int newsockfd, int write_to_bash_fd, int read_from_bash_fd, int child_id)
{
	char buff[10]; // TODO: Check if this needs to made larger for the compress option (to 1kb)?
	char buff_shell[BUFF_SIZE];
	//int num_read;

	struct pollfd input_sources[2];

	input_sources[0].fd = newsockfd;
	input_sources[0].events = POLLIN | POLLHUP | POLLERR;

	input_sources[1].fd = read_from_bash_fd;
	input_sources[1].events = POLLIN | POLLHUP | POLLERR;

	int eof_received = FALSE;

	signal(SIGPIPE, SIG_IGN); // ignore the SIGPIPE status

	while(! eof_received)
	{
		int poll_rc = poll(input_sources, 2, 0);
		if(poll_rc > 0)
		{
			// some kind of input received
			if(input_sources[0].revents & POLLIN)
			{
				// input received from the client 
				// forward it to shell
				int num_read ;
				num_read = read(newsockfd, &buff, 10);
				if(num_read <= 0)
				{
					// Read error or EOF received from the network
					close(write_to_bash_fd);
					eof_received = TRUE;
					break;
					//printError("Could not read from socket");
				}
				int i;
				for(i = 0; i < num_read; i += 1)
				{
					if(buff[i] == 0x03) // ^C received TODO: CHANGE!!!!!!!!
					{
						/*eof_received = 1;
						break;*/
						// ^C received so send kill signal and continue writing to the shell
						if(kill(child_id, SIGINT) < 0)
						{
							printError(strcat("Error while sending SIGNINT to child: ", strerror(errno)));
						}
						//eof_received = 1;
						if(write(write_to_bash_fd, &buff[i], 1) < 0)
						{
							printError(strcat("Error while writing to bash shell: ", strerror(errno)));
						}
					}
					else if(buff[i] == 0x04) // ^D received
					{
						if(close(write_to_bash_fd) < 0)
						{
							printError(strcat("Error while trying to close fd: ", strerror(errno)));
						}
						eof_received = TRUE;
						break;
					}
					else if(buff[i] == '\r' || buff[i]=='\n')
					{
						if(write(write_to_bash_fd, "\n", 1) == -1)
						{
							printError(strcat("Error while writing to the bash shell: ", strerror(errno)));
						}
					}
					else
					{
						if(write(write_to_bash_fd, &buff[i], 1) == -1)
						{
							printError(strcat("Error while writing to the bash shell: ", strerror(errno)));
						}
					}
				}
			}
			else if(input_sources[0].revents & (POLLHUP | POLLERR))
			{
				// some kind of error occured during input from the client
				// TODO: Check if POLLHUP should also cause server to quit
				printError("Poll Hangup/Error from socket");
				break;
			}
			else if(input_sources[1].revents & POLLIN)
			{
				// input received from bash so forward it to the socket
				int num_read = read(read_from_bash_fd, &buff_shell, BUFF_SIZE);
				if(num_read < 0)
				{
					printError(strcat("Error while reading from shell: ", strerror(errno)));
				}
				if(write(newsockfd, &buff_shell, num_read) == -1)
				{
					printError(strcat("Error while writing to the socket: ", strerror(errno)));
				}
			}
			else if(input_sources[1].revents & POLLHUP)
			{
				// POLLHUP received from shell
				//printError("Poll Hangup/Error from socket");
				break;
			}
			else if(input_sources[1].revents & POLLERR)
			{
				// error occured so just exit
				printError("Poll Error from bash");
				break;
			}
		}
		else if(poll_rc < 0)
		{
			printError(strcat("Error occured during polling: ", strerror(errno)));
		}
	}

	// finish processing input from the shell
	while(1)
	{
		int num_read = read(read_from_bash_fd, &buff_shell, BUFF_SIZE);
		if(num_read < 0)
		{
			printError(strcat("Error while trying to process bash input", strerror(errno)));
		}
		else if(num_read == 0)
		{
			break;
		}
		else
		{
			if(write(newsockfd, &buff_shell, num_read) < 0)
			{
				printError(strcat("Error while trying to write to socket: ", strerror(errno)));
			}
		}
	}

	int shell_status;
	if(waitpid(child_id, &shell_status, 0) == -1)
	{
		// error
		printError(strcat("Error occured while waiting for shell", strerror(errno)));
	}
	else
	{
		// successfully finished reading all of the shell's output
		fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", (shell_status & 0x007f), ((shell_status & 0xff00) >> 8));
	}
}

int createBashSession(int to_bash[2], int from_bash[2], int socketfd)
{
	int fork_rc = fork();
	if(fork_rc < 0)
	{
		printError("Could not fork");
	}
	else if(fork_rc == 0)
	{
		// child process, do file redirection and then exec the bash shell
		// close write end of the pipe that corresponds to data from server to shell in child process
		// close read end of the pipe that corresponds to data from shell to pipe in child process
		if( close(to_bash[WRITE_END]) == -1 || close(from_bash[READ_END]) == -1)
		{
			printError(strcat("Could not close ends of pipe: ", strerror(errno)));
		}
		// do the file redirection now
		// redirect read end to stdin
		if(	close(STDIN_FILENO) == -1 || dup(to_bash[READ_END]) == -1 || close(to_bash[READ_END]) == -1)
		{
			printError(strcat("Error closing file descriptors: ", strerror(errno)));
		}
		if(close(STDOUT_FILENO) == -1 || dup(from_bash[WRITE_END]) == -1)
		{
			printError(strcat("Error closing file descriptors: ", strerror(errno)));
		}
		if(close(STDERR_FILENO) == -1 || dup(from_bash[WRITE_END]) == -1 )
		{
			printError(strcat("Error closing file descriptors: ", strerror(errno)));
		}
		if(close(from_bash[WRITE_END]) == -1)
		{
			printError(strcat("Error closing file descriptors: ", strerror(errno)));
		}

		if(close(socketfd) == -1)
		{
			printError(strcat("Error closing file descriptors: ", strerror(errno)));
		}

		// no errors in file redirection so just exec the shell!!!
		char * args[] = {"/bin/bash", NULL};

		if(execv(args[0], args) == -1)
		{
			printError("Could not execute shell");
		}


	}
	else
	{
		// parent process, close the correct file descriptors and continue processing
		close(to_bash[READ_END]);
		close(from_bash[WRITE_END]);
		return fork_rc;
	}

	return -1; // Will never be executed
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
	setupSocket(&newsockfd, port_num); // function returns when an input has been 
	
	int to_bash[2]; // pipe from the server to the shell
	int from_bash[2]; // pipe from the shell back to the server

	if(pipe(to_bash) == -1)
	{
		printError(strcat("Could not create pipe: ", strerror(errno)));
	}
	if(pipe(from_bash) == -1)
	{
		printError(strcat("Could not create pipe: ", strerror(errno)));
	}

	int child_id = createBashSession(to_bash, from_bash, newsockfd); // the parent process now has the valid file descriptors to_bash[WRITE_END] and from_bash[READ_END]

	processInput(newsockfd, to_bash[WRITE_END], from_bash[READ_END], child_id);

	if(close(newsockfd) == -1)
	{
		printError("Error closing socket");
	}

	return 0;
}