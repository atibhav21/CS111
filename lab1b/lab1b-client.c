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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <zlib.h>

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
	fprintf(stderr, "usage: ./lab1b --port=num [--log=filename] [--compress]\n");
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

void setUpSocket(int* sockfd, int port_num)
{
	struct sockaddr_in serv_addr;
	struct hostent* server;

	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(*sockfd < 0)
	{
		printErrorAndReset(strcat("Error while setting up socket: ", strerror(errno)));
	}
	server = gethostbyname("localhost");
	if(server == NULL)
	{
		printErrorAndReset("Could not connect to localhost");
	}
	memset((char *)& serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *) server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);

	serv_addr.sin_port = htons(port_num);

	
	if(connect(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // TODO: This is causing a segfault
	{
		printErrorAndReset(strcat("Error connecting to server: ", strerror(errno)));
	}


}

void perform_writes(int log_specified, int log_fd, int sock_fd, char* buff, int num_bytes)
{
	if(buff[0] == '\r' || buff[0] =='\n')
	{
		if(write(STDOUT_FILENO, "\r\n", 2) == -1)
		{
			printErrorAndReset(strcat("Error while trying to write: ", strerror(errno)));
		}
	}
	else if(write(STDOUT_FILENO, &buff[0], num_bytes) == -1)
	{
		printErrorAndReset(strcat("Error while trying to write: ", strerror(errno)));
	}
	/*if(log_specified && write(log_fd, &buff[0], num_bytes) == -1)
	{
		/*char num_bytes_string[20];
		sprintf(num_bytes_string, "%d", num_bytes);
		char log_str [100];
		strcpy(log_str, strcat(strcat(strcat("SENT ", num_bytes_string), " bytes: "), buff));
		fprintf(stderr, "%s\n", log_str);

		printErrorAndReset(strcat("Error while trying to write to logfile: ", strerror(errno)));
	}*/
	if(write(sock_fd, &buff[0], num_bytes) == -1)
	{
		printErrorAndReset(strcat("Error while trying to write: ", strerror(errno)));
	}
	else
	{
		if(log_specified)
		{
			
			if(write(log_fd, &buff[0], 1) == -1)
			{
				printErrorAndReset(strcat("Error while trying to write to logfile: ", strerror(errno)));
			}
		}
	}
}


void processPollResult(struct pollfd input_sources[2], int log_specified, int log_fd, int sock_fd, int* eof_received)
{
	char buff[10];
	
	char buff_sock[256];
	if(input_sources[0].revents & POLLIN)
	{
		// input from the keyboard
		int num_read;
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
				*eof_received = TRUE;
				//break;
			}
			/*else if(buff[i] == '\r' || buff[i] == '\n')
			{
				perform_writes(log_specified, log_fd, sock_fd, "\r\n", 2);
			}*/
			else
			{
				perform_writes(log_specified, log_fd, sock_fd, &buff[i], 1);
			}
		}
	}
	else if(input_sources[0].revents & (POLLHUP | POLLERR))
	{
		// some error receiving input from the keyboard
		printErrorAndReset("Poll Hangup/Error occured on Keyboard file descriptor");
	}
	else if(input_sources[1].revents & POLLIN)
	{
		// input from the socket
		int num_read;
		if((num_read = read(sock_fd, &buff_sock, 256)) == -1)
		{
			printErrorAndReset(strcat("Error while reading socket input: ", strerror(errno)));
		}
		if(num_read == 0)
		{
			*eof_received = TRUE;
		}
		int i;
		for(i = 0; i < num_read; i += 1)
		{
			if(buff_sock[i] == '\r' || buff_sock[i] == '\n')
			{
				if(write(STDOUT_FILENO, "\r\n", 2) == -1)
				{
					printErrorAndReset(strcat("Error while writing to stdout: ", strerror(errno)));
				}
			}
			else
			{
				if(write(STDOUT_FILENO, &buff_sock[i], 1) == -1)
				{
					printErrorAndReset(strcat("Error while writing to stdout: ", strerror(errno)));
				}
			}
		}
	}
	else if(input_sources[1].revents & POLLHUP)
	{
		// hangup received from socket file descriptor
		//fprintf(stderr, "Received POLLHUP\r\n");
		*eof_received = TRUE; // TODO: Possibly change
		//break;
	}
	else if(input_sources[1].revents & POLLERR)
	{
		printErrorAndReset(strcat("Error received during polling: ", strerror(errno)));
	}
}

void processInput(int port_num, int log_specified, int log_fd)
{
	int eof_received = FALSE; // TODO: remove
	int sock_fd;

	struct pollfd input_sources[2]; // input_sources[0] is the keyboard, input_sources[1] is the socket fd

	

	setUpSocket(&sock_fd, port_num);

	input_sources[0].fd = STDIN_FILENO;
	input_sources[0].events = POLLIN | POLLHUP | POLLERR;

	input_sources[1].fd = sock_fd;
	input_sources[1].events = POLLIN | POLLHUP | POLLERR;

	int poll_result;
	while(! eof_received)
	{
		poll_result = poll(input_sources, 2, 0);
		if(poll_result > 0)
		{
			processPollResult(input_sources, log_specified, log_fd, sock_fd, &eof_received);

		}
		else if(poll_result < 0)
		{
			printErrorAndReset(strcat("Error received during polling: ", strerror(errno)));
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
	int port_set = FALSE;
	int log_specified = FALSE;
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
						  port_set = TRUE;
							break;
				case 'l': log_specified = TRUE;
							log_filename = optarg;
							break;
				/*case 'c': compress_mode = TRUE;
							break;*/
				default:
					printUsageMessage();
					exit(1);
			}
		}
	}
	
	if(port_set == FALSE)
	{
		printUsageMessage();
		exit(1);
	}

	if(tcgetattr(STDOUT_FILENO, &old_settings) == -1)
	{
		printError(strcat("Error in getting terminal settings: ", strerror(errno)));
	}


	struct termios new_settings = old_settings;
	setTerminalSettings(&new_settings); // set the terminal settings to non-canonical no-echo mode

	int log_fd = -1;

	if(log_specified)
	{
		log_fd = open(log_filename, O_CREAT | O_WRONLY| O_TRUNC, S_IRWXU);
		if(log_fd == -1)
		{
			printErrorAndReset(strcat("Could not open logfile: ", strerror(errno)));
		}
	} 


	processInput(port_num, log_specified, log_fd);

	
	if(log_specified && close(log_fd) == -1)
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

