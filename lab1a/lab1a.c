/*
NAME: Atibhav Mittal
EMAIL: atibhav.mittal6@gmail.com
ID: 804598987
*/

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

extern int errno;

struct termios old_settings;

#define TRUE 1
#define FALSE 0

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

void createNewSettings(struct termios* settings, struct termios old_settings)
{
	*settings = old_settings;
	settings->c_iflag = ISTRIP;	/* only lower 7 bits	*/
	settings->c_oflag = 0;		/* no processing	*/
	settings->c_lflag = 0;		/* no processing	*/
}

void exitError(char* message)
{
	fprintf(stderr, "Error received: %s\r\n", message);
	tcsetattr(1, TCSANOW, &old_settings);
	exit(1);
}

// Process the input and output it
// if from_terminal argument is set to true, then forward the buffer to the FD referred to by fd_to_shell
// 
int processInputs(int shell_active, int fd_to_shell, char* buffer, int num_read, int from_terminal, int child_id)
{
	if(from_terminal)
	{
		int i;
		for(i = 0; i < num_read; i += 1)
		{
			if(buffer[i] == 3)
			{
				fprintf(stderr, "^C pressed, Killing child_id: %d\r\n", child_id);
				if(kill((pid_t) child_id, SIGINT) == -1)
				{	
					exitError("Error while sending SIGINT to shell");
				}
				else
				{
					fprintf(stderr, "At least one signal was sent\r\n");	
				}
				write(1, &buffer[i], 1);
				return 0; // continue with the processing
			}
			if(buffer[i] == 4) // ^D received
			{
				// end of file received
				write(1, "^D", 2);
				close(fd_to_shell);
				return 1;
			}
			else if(i+1 < num_read && buffer[i] == '\r' && buffer[i] == '\n')
			{
				write(1, "\r\n", 2);
				if(shell_active)
				{
					write(fd_to_shell, "\n", 1);
				}
				i = i + 1;
			}
			else if(buffer[i] == '\r' || buffer[i] == '\n')
			{
				write(1, "\r\n", 2);
				if(shell_active)
				{
					if(write(fd_to_shell, "\n", 1) == -1)
					{
						//fprintf(stderr, "Error: %s\n", strerror(errno));
						exitError(strerror(errno));
					}
				}
			}
			else
			{
				write(1, &buffer[i], 1);
				if(shell_active)
				{
					if(write(fd_to_shell, &buffer[i], 1) == -1)
					{
						exitError(strerror(errno));
					}
				}
			}
		}
	}
	else
	{
		int i;
		for(i = 0; i < num_read; i += 1)
		{
			/*if(buffer[i] == 4)
			{
				write(1, "\r\n", 2);
			}*/ 
			if(buffer[i] == '\r' || buffer[i] == '\n')
			{
				if(write(1, "\r\n", 2) == -1) 
				{
					exitError(strerror(errno));
				}
			}
			else
			{
				if(write(1, &buffer[i], 1) == -1)
				{
					exitError(strerror(errno));
				}
			}
		}
		//write(1, buffer, num_read);
	}
	return 0;
}

// 
void performIO(int shell_active, int fd_to_shell, int fd_from_shell, int child_id)
{
	char buff[10];
	char buff_shell[256];
	int eof_received = 0;

	struct pollfd two_inputs[2];

	two_inputs[0].fd = STDIN_FILENO; // input from the terminal
	two_inputs[0].events = POLLIN | POLLHUP | POLLERR; 

	if(shell_active) 
	{
		two_inputs[1].fd = fd_from_shell; // input from the shell (output of possible commands)
		two_inputs[1].events = POLLIN | POLLHUP | POLLERR;
	}

	signal(SIGPIPE, SIG_IGN);

	int poll_result;
	while(! eof_received)// && ! sigpipe_received) 
	{
		poll_result = poll(two_inputs, 2, 0);
		if(poll_result > 0)
		{
			if(two_inputs[0].revents & POLLIN)
			{
				// receiving input from the terminal
				int num_read = read(0, buff, 10);
				if(num_read == -1)
				{
					exitError(strerror(errno));
				}
				eof_received = processInputs(shell_active, fd_to_shell, buff, num_read, TRUE, child_id);
			}
			else if((two_inputs[0].revents & (POLLHUP | POLLERR)))
			{
				// received some kind of error from the terminal
				exitError(strerror(errno));
			}
			else if(shell_active && two_inputs[1].revents & POLLIN)
			{
				// receiving input from the shell descriptor
				int num_read = read(fd_from_shell, buff_shell, 256);
				if(num_read == -1)
				{
					exitError(strerror(errno));
				}
				eof_received = processInputs(shell_active, fd_to_shell, buff_shell, num_read, FALSE, child_id);
			}
			else if(shell_active && (two_inputs[1].revents & POLLHUP ))
			{
				// all file descriptors to the shell have been closed so exit after processing all read input fromt the shell
				fprintf(stderr, "POLLHUP received\r\n");
				break;
			}
			else if(shell_active && (two_inputs[1].revents & POLLERR))
			{
				// received error from the shell
				exitError(strerror(errno));
			}
		}
		else if(poll_result == 0)
		{
			continue;
		}
		else if(poll_result == -1)
		{
			exitError(strerror(errno));
		}
		//write(1, buff, num_read);
	}
	int num_read;
	while(1)
	{
		// read all remaining output from the shell
		num_read = read(fd_from_shell, buff_shell, 256);
		if(num_read == 0)
		{
			break;
		}
		else if(num_read == -1)
		{
			exitError(strerror(errno));
		}
		else
		{
			if(write(1, buff_shell, num_read) == -1)
			{
				exitError(strerror(errno));
			}
		}
	}
	int shell_status;
	if(waitpid(child_id, &shell_status, 0) == -1)
	{
		// error
		exitError("Error occured while exiting shell");
	}
	else
	{
		// successfully finished reading all of the shell's output
		fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", (shell_status & 0x007f), ((shell_status & 0xff00) >> 8));
	}
}

int main(int argc, char *argv[])
{
	struct termios new_settings;
	//char buff[10];
	int option_index; 
	int in_shell_mode = FALSE;
	//int debug_enabled = FALSE;

	int parent_to_child[2];
	int child_to_parent[2];


	static struct option long_options[] = 
				{
					{"shell", no_argument, 0, 's'},
					//{"debug", no_argument, 0, 'd'},
					{0, 0, 0, 0}
				};
	int c = getopt_long(argc, argv, "", long_options, &option_index);
	while(c != -1)
	{
		switch(c)
		{
			case 's': in_shell_mode = TRUE;
					  break;
			/*case 'd': debug_enabled = TRUE;
					  break;*/
			default:
				fprintf(stderr, "usage: ./lab1 [--shell]\n");
				exit(1);
		}

		c = getopt_long(argc, argv, "", long_options, &option_index);	
	}


	if(tcgetattr(1 , &old_settings) != 0)
	{
		// unsuccessful in getting the settings of the terminal, so print error message and quit
		printf("Error in getting terminal attributes: %s\n", strerror(errno));
		exit(1);
	}

	createNewSettings(&new_settings, old_settings);

	if(tcsetattr(1,TCSANOW, &new_settings) != 0)
	{
		printf("Error in setting terminal attributes: %s\n", strerror(errno));
		exit(1);
	}

	/*
	* parent_to_child[0] -> parent_to_child[1] is the pipe that takes data from user process and sends it to the shell
	* child_to_parent[0] -> child_to_parent[1] is the process that sends the output of the shell execution to the parent(lab1a.c) process
	*/
	if(in_shell_mode)
	{
		if(pipe(parent_to_child) != 0)
		{
			exitError("Could not create pipe");
		}
		if(pipe(child_to_parent) != 0)
		{
			exitError("Could not create pipe");
		}
		int rc = fork();
		if(rc < 0)
		{
			exitError("Fork Failed");
		}
		else if(rc == 0)
		{
			// Child process, do file redirection, and then call exec to run the shell
			if(close(child_to_parent[0]) != 0 ||  // close read end of the pipe from child to parent in the child process
						close(parent_to_child[1])) { // close write end of the pipe from parent to child in the child process
				exitError("Error in Closing the pipe");
			}
			if(close(0) != 0 || dup(parent_to_child[0]) == -1 || close(parent_to_child[0]) != 0) // close stdin and duplicate it
			{
				exitError("Error in Duplicating pipe");
			}

			if(close(1) != 0 || dup(child_to_parent[1]) == -1) // redirect stdout
			{
				exitError("Error in Duplicating pipe");
			}

			if(close(2) != 0 || dup(child_to_parent[1]) == -1  || close(child_to_parent[1]) != 0)
			{
				exitError("Error in Duplicating pipe");
			}

			char *args[]={"/bin/bash",NULL};
        	//execvp(args[0],args);

			if(execv(args[0], args) == -1)
			{
				// Error in creating exec, so print error message and exit
				exitError("Could not execute Shell");
			}
		}
		else
		{
			// parent process
			if(close(parent_to_child[0]) != 0 || close(child_to_parent[1]) != 0)
			{
				exitError("Error occured while closing pipe ends");
			}
			performIO(TRUE, parent_to_child[1], child_to_parent[0], rc);
		}
	}
	
	else
	{
		// work in regular mode (non-shell)
		performIO(FALSE, parent_to_child[1], child_to_parent[0], -1) ;
	}


	tcsetattr(1, TCSANOW, &old_settings); // reset to old settings
	return 0;
}