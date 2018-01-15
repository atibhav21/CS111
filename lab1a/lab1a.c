#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern int errno;

void createNewSettings(struct termios* settings, struct termios old_settings)
{
	*settings = old_settings;
	settings->c_iflag = ISTRIP;	/* only lower 7 bits	*/
	settings->c_oflag = 0;		/* no processing	*/
	settings->c_lflag = 0;		/* no processing	*/
}

void processInputs()
{
	char buff[10];
	int eof_received = 0;
	while(! eof_received) 
	{
		int num_read = read(0, buff, 10);
		int i;
		for(i = 0; i < num_read; i += 1)
		{
			if(buff[i] == 4) // ^D ASCII representation is 4, so if it is received break out of input loop
			{
				eof_received = 1;
				break;
			}
			if(buff[i] == '\n' || buff[i] == '\r') 
			{
				write(1, "\r\n", 2);
			}
			else
			{
				write(1, &buff[i], 1);
			}
		}
		//write(1, buff, num_read);
	}
}

int main(int argc, char const *argv[])
{
	struct termios old_settings, new_settings;
	char buff[10];

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

	processInputs();

	tcsetattr(1, TCSANOW, &old_settings); // reset to old settings
	return 0;
}