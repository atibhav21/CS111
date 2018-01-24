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

#define TRUE 1
#define FALSE 0

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

struct termios old_settings;

void createNewSettings(struct termios* settings, struct termios old_settings)
{
	*settings = old_settings;
	settings->c_iflag = ISTRIP;	/* only lower 7 bits	*/
	settings->c_oflag = 0;		/* no processing	*/
	settings->c_lflag = 0;		/* no processing	*/
}

void signal_handler(int x)
{
	fprintf(stderr, "Received SIGPIPE: %d\r\n", x);
	tcsetattr(1, TCSANOW, &old_settings);
	exit(1);
}

// Process the input and output it
// if from_terminal argument is set to true, then forward the buffer to the FD referred to by fd_to_shell
// child_id is the process id of the child process created by the main executable program
void processInputs(int shell_active, int fd_to_shell, char* buffer, int num_read, int from_terminal, pid_t child_id)
{
	if(from_terminal)
	{
		int i;
		for(i = 0; i < num_read; i += 1)
		{
			if(buffer[i] == 3) // ^C received
			{
				// send a kill signal to the shell and return
				//fprintf(stderr, "Kill Called\r\n");
				if(kill(child_id, SIGINT) == -1 )
				{
					fprintf(stderr, "Error while sending kill signal: %s\r\n", strerror(errno));
					tcsetattr(1, TCSANOW, &old_settings);
					exit(1);
				}
				//return 1;
			}
			if(buffer[i] == 4) // ^D received
			{
				// end of file received
				if( close(fd_to_shell) == -1)
				{
					fprintf(stderr, "Error while closing shell descriptor: %s\r\n", strerror(errno));
					tcsetattr(1, TCSANOW, &old_settings);
					exit(1);
				}
				//return 1;
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
						fprintf(stderr, "Error: %s\n", strerror(errno));
						tcsetattr(1, TCSANOW, &old_settings);
						exit(1);
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
						fprintf(stderr, "Error: %s\n", strerror(errno));
						tcsetattr(1, TCSANOW, &old_settings);
						exit(1);
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
			if(buffer[i] == 4)
			{
				// EOF file received
				//write(1, "End of File received\n", 20);
				//write(1, "\r\n", 2);
				fprintf(stderr, "End of File received\r\n");
			}
			if(buffer[i] == '\r' || buffer[i] == '\n')
			{
				write(1, "\r\n", 2); //TODO: Check if this needs to be changed to spaces or any other thing 
			}
			else
			{
				write(1, &buffer[i], 1);
			}
		}
		//write(1, buffer, num_read);
	}

	//return 0;
}

// 
void performIO(int shell_active, int fd_to_shell, int fd_from_shell, int child_id)
{
	char buff[10];
	char buff_shell[256];

	struct pollfd two_inputs[2];

	two_inputs[0].fd = STDIN_FILENO; // input from the terminal
	two_inputs[0].events = POLLIN | POLLHUP | POLLERR; 

	if(shell_active) 
	{
		two_inputs[1].fd = fd_from_shell; // input from the shell (output of possible commands)
		two_inputs[1].events = POLLIN | POLLHUP | POLLERR;
	}


	while(1) 
	{
		if(poll(two_inputs, 2, 0) > 0)
		{
			signal(SIGPIPE, signal_handler);
			if(two_inputs[0].revents & POLLIN)
			{
				// receiving input from the terminal
				fprintf(stdout, "Hitting Point A\r\n");
				int num_read = read(0, buff, 10);
				processInputs(shell_active, fd_to_shell, buff, num_read, TRUE, child_id);
			}
			else if(two_inputs[0].revents & POLLHUP)
			{
				// received some kind of error from the terminal
				// TODO!!
				fprintf(stdout, "Hitting Point B\r\n");
				fprintf(stderr, "Something received in first event\r\n");
				break;
			}
			else if(two_inputs[0].revents & POLLERR)
			{
				fprintf(stderr, "Something error occured during polling: %s\r\n", strerror(errno));
				tcsetattr(1, TCSANOW, &old_settings);
				exit(1);
			}
			else if(shell_active && two_inputs[1].revents & POLLIN)
			{
				
				// receiving input from the shell descriptor
				fprintf(stdout, "Hitting Point C\r\n");
				int num_read = read(fd_from_shell, buff_shell, 256);
				processInputs(shell_active, fd_to_shell, buff_shell, num_read, FALSE, child_id);
			}
			else if(shell_active && (two_inputs[1].revents & POLLHUP))
			{
				//fprintf(stderr, "Does this get reached?\r\n");

				
				// received eof from the shell
				// close write pipes from shell back to main lab1a executable
				//close()
				fprintf(stdout, "Hitting Point D\r\n");
				int num_read;
				// keep doing a read  until it returns 0, since hangup has occured, after receiving 0, 
				// use wait pid to get shell's exit status and report it to stderr
				// TODO: Uncomment
				while((num_read = read(fd_from_shell, buff_shell, 256)) != 0)
				{
					if(num_read == -1)
					{
						fprintf(stderr, "Some error occured during read/write from shell: %s\n", strerror(errno));
						tcsetattr(1, TCSANOW, &old_settings);
						exit(1);
					}
					else
					{
						processInputs(shell_active, fd_to_shell, buff_shell, num_read, FALSE, child_id);
					}
				}

				int shell_status;
				if(waitpid(child_id, &shell_status, 0) == -1)
				{
					// error
					fprintf(stderr, "Error occured while exiting shell\n");
					tcsetattr(1, TCSANOW, &old_settings);
					exit(1);
				}
				else
				{
					// successfully finished reading all of the shell's output
					fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", (shell_status & 0x007f), (shell_status & 0xff00));
				}
				
				//fprintf(stderr, "Something received in second event\r\n" );
				break;
			}
			else if(shell_active && (two_inputs[1].revents & POLLERR))
			{
				fprintf(stdout, "Hitting Point E\r\n");
				int shell_status;
				if(waitpid(child_id, &shell_status, 0) == -1)
				{
					// error
					fprintf(stderr, "Error occured while exiting shell\r\n");
					tcsetattr(1, TCSANOW, &old_settings);
					exit(1);
				}
				else
				{
					// successfully finished reading all of the shell's output
					fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", (shell_status & 0x007f), (shell_status & 0xff00));
				}
				break;
			}
		}
		else
		{
			fprintf(stdout, "Hitting Point F\r\n");
			continue;
		}
	}
}

void exitError(char* message)
{
	fprintf(stderr, "%s: %s\r\n", message, strerror(errno) );
	tcsetattr(1, TCSANOW, &old_settings);
	exit(1);
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
				fprintf(stderr, "Usage: ./lab1a [--shell]\r\n");
				exitError("Incorrect Option Passed, Exiting program now...");
				break; // Will never reach this point
		}

		c = getopt_long(argc, argv, "", long_options, &option_index);	
	}

	/*
	* parent_to_child is the pipe that takes data from user process and sends it to the shell
	* child_to_parent is the pipe that sends the output of the shell execution to the parent(lab1a) process
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

		int child_id = fork();
		if(child_id < 0)
		{
			exitError("Fork failed");
		}
		else if(child_id == 0)
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
			// close read end of parent_to_child descriptor
			// close write end of child_to_parent descriptor
			if(close(parent_to_child[0]) != 0 || close(child_to_parent[1]) != 0)
			{
				exitError("Error occured while closing pipe ends");
			}
			performIO(TRUE, parent_to_child[1], child_to_parent[0], child_id);
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