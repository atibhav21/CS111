#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>
#include <poll.h>

extern int errno;

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

// Process the input and output it
// if from_terminal argument is set to true, then forward the buffer to the FD referred to by fd_to_shell
// 
int processInputs(int shell_active, int fd_to_shell, char* buffer, int num_read, int from_terminal)
{
	if(from_terminal)
	{
		int i;
		for(i = 0; i < num_read; i += 1)
		{
			if(buffer[i] == 4)
			{
				// end of file received
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
						fprintf(stderr, "Error: %s\n", strerror(errno));
						return -1;
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
						return -1;
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
				write(1, "\r\n", 2); //TODO: Check if this needs to be changed to spaces or any other thing 
			}
			else
			{
				write(1, &buffer[i], 1);
			}
		}
		//write(1, buffer, num_read);
	}

	return 0;
}

// 
int performIO(int shell_active, int fd_to_shell, int fd_from_shell)
{
	char buff[10];
	char buff_shell[256];
	int eof_received = FALSE;

	struct pollfd two_inputs[2];

	two_inputs[0].fd = STDIN_FILENO; // input from the terminal
	two_inputs[0].events = POLLIN | POLLHUP | POLLERR; 

	if(shell_active) 
	{
		two_inputs[1].fd = fd_from_shell; // input from the shell (output of possible commands)
		two_inputs[1].events = POLLIN | POLLHUP | POLLERR;
	}


	while(eof_received == FALSE) 
	{
		if(poll(two_inputs, 2, 0) > 0)
		{
			if(two_inputs[0].revents & POLLIN)
			{
				// receiving input from the terminal
				int num_read = read(0, buff, 10);
				eof_received = processInputs(shell_active, fd_to_shell, buff, num_read, TRUE);
			}
			else if((two_inputs[0].revents & (POLLHUP | POLLERR)))
			{
				// received some kind of error from the terminal
			}
			else if(shell_active && two_inputs[1].revents & POLLIN)
			{
				// receiving input from the shell descriptor
				int num_read = read(fd_from_shell, buff_shell, 256);
				eof_received = processInputs(shell_active, fd_to_shell, buff_shell, num_read, FALSE);
			}
			else if(shell_active && (two_inputs[1].revents & (POLLHUP | POLLERR)) )
			{
				// received error from the shell
			}
		}
		else
		{
			continue;
		}
		/*int num_read = read(0, buff, 10);
		
		int i;

		if(shell_active)
		{
			// reads the output of the shell 
			int num_read_shell = read(fd_from_shell, buff_shell, 256);
			for(i = 0; i < num_read_shell; i+= 1)
			{
				// TODO: More iterative processing of the buffer
				write(1, &buff_shell, num_read_shell);
			}
		}
			
		for(i = 0; i < num_read; i += 1)
		{
			if(buff[i] == 4) // ^D ASCII representation is 4, so if it is received break out of input loop
			{
				eof_received = 1;
				break;
			}
			if( (i+1) < num_read && buff[i] == '\r' && buff[i+1] == '\n')
			{
				write(1, "\r\n", 2);
				i = i + 1;
				if(shell_active)
				{
					write(fd_to_shell, "\n", 1);
				}
			}
			// TODO: check if '\r\n' before checking for either character otherwise there will be double processing of some characters!!
			if(buff[i] == '\n' || buff[i] == '\r') 
			{
				write(1, "\r\n", 2);
				if(shell_active)
				{
					write(fd_to_shell, "\n", 1);
				}
			}
			else
			{
				write(1, &buff[i], 1);
				if(shell_active)
				{
					write(fd_to_shell, &buff[i], 1);
				}
			}
		}*/
		//write(1, buff, num_read);
	}
	if(eof_received == -1)
	{
		return -1;
	}
	else
	{
		// normal exit
		return 0;
	}
}

int main(int argc, char *argv[])
{
	struct termios old_settings, new_settings;
	char buff[10];
	int option_index; 
	int in_shell_mode = FALSE;
	int debug_enabled = FALSE;

	int parent_to_child[2];
	int child_to_parent[2];

	static struct option long_options[] = 
				{
					{"shell", no_argument, 0, 's'},
					{"debug", no_argument, 0, 'd'},
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
			case 'd': debug_enabled = TRUE;
					  break;
			default:
				printf("Incorrect Option Passed, Exiting program now...\n");
				tcsetattr(1, TCSANOW, &old_settings); // reset to old settings
				exit(1);
		}

		c = getopt_long(argc, argv, "", long_options, &option_index);	
	}

	/*
	* parent_to_child[0] -> parent_to_child[1] is the pipe that takes data from user process and sends it to the shell
	* child_to_parent[0] -> child_to_parent[1] is the process that sends the output of the shell execution to the parent(lab1a.c) process
	*/
	if(in_shell_mode)
	{
		if(pipe(parent_to_child) != 0)
		{
			printf("Could not create pipe: %s. Exiting now...\n", strerror(errno));
			tcsetattr(1, TCSANOW, &old_settings); // reset to old settingss
			exit(1);
		}
		if(pipe(child_to_parent) != 0)
		{
			printf("Could not create pipe: %s. Exiting now...\n", strerror(errno));
			tcsetattr(1, TCSANOW, &old_settings); // reset to old settings
			exit(1);
		}
		/*if(debug_enabled)
		{
			fprintf(stderr, "Created pipe. Write End FD:%d, Read End FD:%d\n", parent_to_child[0], parent_to_child[1]);
			fprintf(stderr, "Created pipe. Write End FD:%d, Read End FD:%d\n", child_to_parent[0], child_to_parent[1]);
		}*/
	/*}

	if(in_shell_mode)
	{*/
		int rc = fork();
		if(rc < 0)
		{
			fprintf(stderr, "Fork failed\n");
			tcsetattr(1, TCSANOW, &old_settings); // reset to old settings
			exit(1);
		}
		else if(rc == 0)
		{
			//TODO: FIX THIS SHIT, doesn't do file redirection at any point!! (Probably)

			// Child process, do file redirection, and then call exec to run the shell
			//fprintf(stderr, "Child Process created with pid: %d\n", getpid()); // Need to use \n to flush the output buffer

			//close(0); // close stdin 
			close(parent_to_child[1]);
			if(close(0) != 0 || dup(parent_to_child[0]) != 0 || close(parent_to_child[0]) != 0)
			{
				fprintf(stderr, "Error in Duplicating pipe: %s\n", strerror(errno));
				tcsetattr(1, TCSANOW, &old_settings);
				exit(1);
			} // 
			
			//Need to redirect stdout, stderr of child process back to parents stdin
			close(child_to_parent[0]); // close read end

			close(1); // Close stdout
			dup(child_to_parent[1]);

			close(2); // close stderr
			dup(child_to_parent[1]);
			//close(child_to_parent[1]);

			
			/*int fd = open("abc.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
			if(fd >= 0)
			{
				close(1);
				dup(fd);
				close(fd);
			}*/

			char *args[]={"/bin/bash",NULL};
        	//execvp(args[0],args);

			if(execv(args[0], args) == -1)
			{
				// Error in creating exec, so print error message and exit
				fprintf(stderr, "Could not execute Shell: %s\n", strerror(errno));
				tcsetattr(1, TCSANOW, &old_settings); // reset to old settings
				exit(1);
			}
		}
		else
		{
			// parent process
			if(performIO(TRUE, parent_to_child[1], child_to_parent[0]) != 0)
			{
				tcsetattr(1, TCSANOW, &old_settings);
				exit(1);
			}
		}
	}
	
	else
	{
		// work in regular mode (non-shell)
		if(performIO(FALSE, parent_to_child[1], child_to_parent[0]) != 0)
		{
			tcsetattr(1, TCSANOW, &old_settings);
			exit(1);
		}
	}


	tcsetattr(1, TCSANOW, &old_settings); // reset to old settings
	return 0;
}