// NAME: Atibhav Mittal
// EMAIL: atibhav.mittal6@gmail.com
// ID: 804598987

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <time.h>
#include <sys/time.h> 
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include <mraa.h>
#include <mraa/aio.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define TRUE 1
#define FALSE 0

#define AIO_TEMP 1

extern char* optarg;
extern int errno;
extern int optind;

int period = 1;
char scale = 'F';
int shutdown_temp = 0;
int reports_enabled = 1;

int logfile_fd = 0;
int logging_enabled = 0;

mraa_aio_context tempSensor;
//mraa_gpio_context button;

char buffered_string[1024];
int buffered_string_counter = 0;

unsigned int id = 0;
char host[80];
int port_num = 0;

void exitError()
{
	//TODO: Add error mess
	fprintf(stderr, "Error Occured: %s\n", strerror(errno));
	exit(2);
}

void printUsageMessage()
{
	fprintf(stderr, "usage: ./lab4b [--period=num] [--scale=C/F] --log=filename --host=hostname --id=ID port\n");
}


float readTemp()
{
	int reading = mraa_aio_read(tempSensor);

	int B = 4275; // B values of the thermistor

	float R0 = 100000.0; // R0 = 100k

	float R = 1023.0/reading-1.0;
    R = R0*R;

    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet (in celsius)

    if(scale == 'F')
    {
    	return ((temperature * 9) / 5) + 32;
    }
    else
    {
    	return temperature;
    }
}

void getSubstr(char* src, char* dest, int start_pos, int end_pos)
{
	int num_characters = end_pos - start_pos;
	memcpy(dest, src + start_pos, num_characters);
	dest[num_characters] = '\0';
}

void processCommand()
{
	// process the string in buffered_string
	if(strcmp(buffered_string, "OFF") == 0)
	{
		if(logging_enabled)
		{
			if(write(logfile_fd, buffered_string, buffered_string_counter) < 0 || write(logfile_fd, "\n", 1) < 0)
			{
				exitError();
			}
		}
		shutdown_temp = 1;
	}
	else if(strcmp(buffered_string, "START") == 0)
	{
		if(logging_enabled)
		{
			if(write(logfile_fd, buffered_string, buffered_string_counter) < 0 || write(logfile_fd, "\n", 1) < 0)
			{
				exitError();
			}
		}
		reports_enabled = 1;
	}
	else if (strcmp(buffered_string, "STOP") == 0)
	{
		if(logging_enabled)
		{
			if(write(logfile_fd, buffered_string, buffered_string_counter) < 0 || write(logfile_fd, "\n", 1) < 0)
			{
				exitError();
			}
		}
		reports_enabled = 0;
	}
	else
	{
		// could be change of scale, period, or a log command
		// length of the string is buffered_string_counter
		char substring[10];
		if(buffered_string[0] == 'L')
		{
			// check if valid command for logging
			if(buffered_string_counter < 3)
			{
				return;
			}

			getSubstr(buffered_string, substring, 0, 3);
			if(strcmp(substring, "LOG") == 0)
			{
				// write the entire buffered string to the logfile
				if(logging_enabled)
				{
					if(write(logfile_fd, buffered_string, buffered_string_counter) < 0 || write(logfile_fd, "\n", 1) < 0)
					{
						exitError();
					}
				}
			}

		}
		else if(buffered_string[0] == 'S')
		{
			// check if valid command for setting the scale
			if(buffered_string_counter == 7)
			{
				getSubstr(buffered_string, substring, 0, 5);
				if(strcmp(substring, "SCALE") == 0)
				{
					if(logging_enabled)
					{
						if(write(logfile_fd, buffered_string, buffered_string_counter) < 0 || write(logfile_fd, "\n", 1) < 0)
						{
							exitError();
						}
					}
					scale = buffered_string[7];
				}
			}

			
		}
		else if(buffered_string[0] == 'P')
		{
			// check if valid command for setting the period
			if(buffered_string_counter > 7)
			{
				getSubstr(buffered_string, substring, 0, 6);
				if(strcmp(substring, "PERIOD") == 0)
				{
					int j = 7;
					int new_period = 0;
					// start processing from the 8th character to get the number for period
					while(buffered_string[j] != '\0')
					{
						if(! isdigit(buffered_string[j]) )
						{
							return;
						}
						new_period = new_period * 10 + (buffered_string[j] - '0');
						j += 1;
					}
					if(logging_enabled)
					{
						if(write(logfile_fd, buffered_string, buffered_string_counter) < 0 || write(logfile_fd, "\n", 1) < 0)
						{
							exitError();
						}
					}
					
					period = new_period;
				}
			}
		}
		// invalid command so do nothing

	}
}

void processInput(char* input, int num)
{
	int i;
	for(i = 0; i < num; i += 1)
	{
		if(input[i] == '\n')
		{
			// received a backslash so flush the buffer with incomplete input
			buffered_string[buffered_string_counter] = '\0';
			processCommand();
			buffered_string_counter = 0;
			memset((void *)buffered_string, 0, 1024);
		}
		else
		{
			buffered_string[buffered_string_counter] = input[i];
			buffered_string_counter += 1;
		}
		
	}
}

void setUpSocket(int* sockfd)
{
	struct sockaddr_in serv_addr;
	struct hostent* server;

	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(*sockfd < 0)
	{
		exitError();
	}
	server = gethostbyname(host);
	if(server == NULL)
	{
		exitError();
	}
	memset((char *)& serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char*)&serv_addr.sin_addr.s_addr, (char *) server->h_addr, server->h_length);

	serv_addr.sin_port = htons(port_num);

	
	if(connect(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{
		exitError();
	}


}



int main(int argc, char *argv[])
{

	/* code */
	static struct option long_options[] = {
		{"period", required_argument, 0, 'p'},
		{"scale", required_argument, 0, 's'},
		{"log", required_argument, 0, 'l'},
		{"id", required_argument, 0, 'i'},
		{"host", required_argument, 0, 'h'},
		{0,0,0,0}
	};

	int c;
	struct timeval clock;
	char stdout_buffer[128];
	char read_buffer[128];
	struct tm* now;
	time_t next_report_due = 0;
	struct pollfd stdin_poll;

	int sockfd;
	SSL* ssl = NULL;
	SSL_CTX* ctx = NULL;

	// TODO: Check if all the mandatory arguments are passed in
	int log_provided = FALSE, host_provided = FALSE, id_provided = FALSE, port_provided = FALSE;

	//c = getopt_long(argc, argv, "", long_options, NULL);
	while(optind < argc)
	{	
		if((c = getopt_long(argc, argv, "", long_options, NULL)) != -1)
		{
			switch(c)
			{
				case 'p': period = atoi(optarg);
						break;
				case 's': scale = optarg[0];
						if(scale != 'C' && scale != 'F')
						{
							printUsageMessage();
							exit(1);
						}
						break;
				case 'l': 
						log_provided = TRUE;
						logging_enabled = 1;
						logfile_fd = open(optarg, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
						if(logfile_fd < 0) {
							exitError();
						}
						break;
				case 'i': 
						id_provided = TRUE;
						id = (unsigned int) atoi(optarg);
						break;
				case 'h':
						host_provided = TRUE;
						strcpy(host, optarg);
						break;
				default:
						printUsageMessage();
						exit(1);
			}
		}
		else{
			// port number argument
			port_num = atoi(argv[optind]);
			port_provided = TRUE;
			optind += 1;
		}
		
	}

	if(!log_provided || ! host_provided || ! id_provided || ! port_provided)
	{
		printUsageMessage();
		exit(1);
	}

	tempSensor = mraa_aio_init(AIO_TEMP);
	//button = mraa_gpio_init(GPIO_BUTTON);
	//mraa_gpio_dir(button, MRAA_GPIO_IN);

	
	// Set up the SSL connection
	const SSL_METHOD* method;

	OpenSSL_add_all_algorithms();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();
	SSL_load_error_strings();

	//certbio = BIO_new(BIO_s_file());

	if(SSL_library_init() < 0)
	{
		fprintf(stderr, "Could Not initialize the OpenSSL library\n");
		exit(2);
	}

	method = SSLv23_client_method();

	if((ctx = SSL_CTX_new(method)) == NULL)
	{
		fprintf(stderr, "Could not create a new SSL context structure.\n");
		exit(2);
	}

	ssl = SSL_new(ctx);

	setUpSocket(&sockfd);

	SSL_set_fd(ssl, sockfd);

	if(SSL_connect(ssl) != 1)
	{
		fprintf(stderr, "Could not build a SSL session\n");
		exit(2);
	}

	// log the ID to the server
	char id_message[20];
	snprintf(id_message, 20, "ID=%d\n", id);

	if(SSL_write(ssl, id_message, strlen(id_message)) < 0)
	{
		exitError();
	}

	if(write(logfile_fd, id_message, strlen(id_message)) < 0)
	{
		exitError();
	}

	stdin_poll.fd = sockfd;
	stdin_poll.events = POLLIN;
	stdin_poll.revents = 0;

	int poll_result;

	while(! shutdown_temp)
	{
		poll_result = poll(&stdin_poll, 1, 0);

		if(reports_enabled && clock.tv_sec >= next_report_due)
		{
			now = localtime(&(clock.tv_sec));
			float temperature = readTemp();
			snprintf(stdout_buffer, sizeof(stdout_buffer), "%02d:%02d:%02d %.1f\n", now->tm_hour, now->tm_min, now->tm_sec, temperature);

			SSL_write(ssl, stdout_buffer, strlen(stdout_buffer));
			if(logging_enabled){
				write(logfile_fd, stdout_buffer, strlen(stdout_buffer));
			}
			next_report_due = clock.tv_sec + period;
		}

		if(poll_result > 0)
		{
			if(stdin_poll.revents & POLLIN)
			{
				// process the user input
				int num_read = SSL_read(ssl, read_buffer, 128);
				if(num_read < 0)
				{
					exitError();
				}
				else
				{
					processInput(read_buffer, num_read);
				}
			}
			else if (stdin_poll.revents & POLLERR)
			{
				exitError();
			}
		}


		gettimeofday(&clock, 0);

		

		/*if(mraa_gpio_read(button) == 1)
		{
			shutdown = 1;
		}*/
		
	}

	// log the shutdown message
	now = localtime(&(clock.tv_sec));
	snprintf(stdout_buffer, sizeof(stdout_buffer), "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
	SSL_write(ssl, stdout_buffer, strlen(stdout_buffer));
	if(logging_enabled){
		write(logfile_fd, stdout_buffer, strlen(stdout_buffer));
	}

	mraa_aio_close(tempSensor);
	//mraa_gpio_close(button);

	SSL_free(ssl);
  	//X509_free(cert);
  	SSL_CTX_free(ctx);

	exit(0);
}