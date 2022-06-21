// First importing all the relevant libraries
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define LENGTH 2048

// variables which are global
int flag = 0;
int socketfd = 0;

// name of client
char name[32];


// if user enter ctrl+c on terminal in order to stop 
void ctrl_c_exit(int sig) 
{
    flag = 1;
}

void str_stdout_begin() 
{
  printf("%s", "-> ");		// at the beginning of every terminal line in each client 
  fflush(stdout);			// begin with a '->' symbol after which a client enters his message
}

void trim (char* arr, int length) 
{
  for (int i = 0; i < length; i++) 
  { 										
    if (arr[i] == '\n') 	// trimming new line char, that index char replaced by a null
	{
      arr[i] = '\0';
      break;
    }
  }
}

// handle message received
void message_recvd() 
{
  char message[LENGTH] = {};
  while (1) 
  {
	int rcvd = recv(socketfd, message, LENGTH, 0);
    if (rcvd > 0) 
	{
      printf("%s", message);
      str_stdout_begin();
    }
	else if (rcvd == 0) 
			break;

	memset(message, 0, sizeof(message));
  }
}

// handling message sent
void message_sent() 
{
  char message[LENGTH] = {};
	char resp[LENGTH + 64] = {};

  while(1) 
  {
  	str_stdout_begin();
    fgets(message, LENGTH, stdin);
    trim(message, LENGTH);

	// if msg is exit then break
    if (strcmp(message, "exit") == 0) 	 
			break;
    else 
	{
	  // here resp store name and message of any client
      sprintf(resp, "%s: %s\n", name, message);
      send(socketfd, resp, strlen(resp), 0);
    }

	bzero(message, LENGTH);
    bzero(resp, LENGTH + 32);
  }
  ctrl_c_exit(2);
}



int main(int argc, char **argv)
{
	if(argc != 2)
	{
		// port need to be specified in order to differentiate between each chatroom
		printf("Usage: %s 'port'\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	signal(SIGINT, ctrl_c_exit);

	printf("Enter your name: ");
  fgets(name, 32, stdin);
  trim(name, strlen(name));

	// Specify condition for valid user name
	if (strlen(name) > 32 || strlen(name) < 2)
	{
		printf("Name must be less than  and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	/* Socket settings */
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);


  // Connect to Server
  int err = connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) 
  {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(socketfd, name, 32, 0);
	//successful connection
	printf("		WELCOME TO THE CHATPLACE		\n");

	// create thread
	pthread_t message_send_thrd;
  if(pthread_create(&message_send_thrd, NULL, (void *) message_sent, NULL) != 0)
   {
	printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

	// create thread
	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) message_recvd, NULL) != 0)
   {
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1)
	{
		if(flag)
		{
			printf("\nGoodBye\n");
			break;
    	}
	}

	// closing 
	close(socketfd);

	return EXIT_SUCCESS;
}
