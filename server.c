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
#include <errno.h>

#define OUT_SIZE 2048
#define NO_OF_CLIENTS 100

// Structure 
typedef struct
{
	struct sockaddr_in address;			// sockadd_in stored in address
	int socketfd;
	int uid;
	char name[32];						// name of client
} client_t;

static int uid = 10;
static unsigned int client_cnt = 0; 		// counting clients

void trim (char* ch, int len) 
{
  for (int i = 0; i < len; i++) 
  { 
    if (ch[i] == '\n') 	// trimming new line char, that index char replaced by a null 
	{
	  ch[i] = '\0';
      break;
    }
  }
}

// initializing pthread mutex for clients
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// declaration with max no of clients
client_t *clients[NO_OF_CLIENTS];

// using queue for adding clients to it
void enqueue(client_t *client)
{
	// locking mutex to avoid same code sharing
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < NO_OF_CLIENTS; ++i)
	{
		if(!clients[i])
		{
			clients[i] = client;
			break;
		}
	}
	// now unlock
	pthread_mutex_unlock(&clients_mutex);
}

// removing clients from the queue
void dequeue(int uid)
{
	// locking to avoid same code sharing
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < NO_OF_CLIENTS; ++i)
	{
		if(clients[i])
		{
			if(clients[i]->uid == uid)
			{
				clients[i] = NULL;
				break;
			}
		}
	}
	// now unlock
	pthread_mutex_unlock(&clients_mutex);
}

// everyone receives message except the sender
void message_sent(char *msg, int uid)
{
	// locking to avoid same code sharing
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<NO_OF_CLIENTS; ++i)
	{
		if(clients[i])
		{
			// if id is not that of the sender
			if(clients[i]->uid != uid)
			{
				if(write(clients[i]->socketfd, msg, strlen(msg)) < 0)
				{
					perror("ERROR: failed to write");
					break;
				}
			}
		}
	}
	// now unlock
	pthread_mutex_unlock(&clients_mutex);
}

// Communication with the client
void *handle_connec(void *arg)
{
	char res[OUT_SIZE];			// buffer
	char name[32];				// name of client	
	int should_leave = 0;		// a flag variable to keep track when to leave

	client_cnt++;
	client_t *client = (client_t *)arg;

	// Name :
	if(recv(client->socketfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1)
	{
		printf("name not entered.\n");
		should_leave = 1;
	} 
	else
	{
		strcpy(client->name, name);
		// display who has joined
		sprintf(res, "%s has joined the chat\n", client->name);		
		printf("%s", res);
		// copying message to others
		message_sent(res, client->uid);
	}

	bzero(res, OUT_SIZE);

	while(1)
	{
		// if leave is true then break
		if (should_leave) 
			break;

		// received
		int rcvd = recv(client->socketfd, res, OUT_SIZE, 0);
		if (rcvd > 0)
		{	
			if(strlen(res) > 0)
			{
				message_sent(res, client->uid);
				trim(res, strlen(res));
				// displaying messages of clients on server
				printf("%s\n", res);
			}
		} 
		else if (rcvd == 0 || strcmp(res, "exit") == 0)		// if exit is entered
		{
			// specifying which user has left
			sprintf(res, "%s has left the chat\n", client->name);
			printf("%s", res);
			message_sent(res, client->uid);
			should_leave = 1;
		} 
		else 
		{
			printf("ERROR: -1\n");
			should_leave = 1;
		}

		bzero(res, OUT_SIZE);
	}

  // Delete client from queue and yield thread 
  close(client->socketfd);
  dequeue(client->uid);
  free(client);
  client_cnt--;
  pthread_detach(pthread_self());

	return NULL;
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
	int options = 1;
	int listenfd = 0, connfd = 0;
	// thread:
  	pthread_t tid;
	struct sockaddr_in server_address;
  	struct sockaddr_in client_address;
	// the port :
	int port = atoi(argv[1]);
	

  // Settings for the socket
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(ip);
  server_address.sin_port = htons(port);

  	// Ignore pipe signals 
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&options,sizeof(options)) < 0)
	{
		perror("ERROR: failed setsockopt");
    	return EXIT_FAILURE;
	}

	// now binding
  if(bind(listenfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) 
  {
    perror("ERROR: Socket binding failed");
    return EXIT_FAILURE;
  }

  // listening
  if (listen(listenfd, 10) < 0) 
  {
    perror("ERROR: listening failed");
    return EXIT_FAILURE;
  }

	// nothing failed hence successful connection
	
	printf("		WELCOME TO THE CHATPLACE		\n");

	// infinite so it keeps open
	while(1)
	{
		socklen_t clilen = sizeof(client_address);
		connfd = accept(listenfd, (struct sockaddr*)&client_address, &clilen);

		// Are maximum clients reached ?
		if((client_cnt + 1) == NO_OF_CLIENTS){
			printf("Max clients have been reached.");
			// closing connection
			close(connfd);
			continue;
		}

		// Settings for client
		client_t *client = (client_t *)malloc(sizeof(client_t));
		client->address = client_address;
		client->socketfd = connfd;
		client->uid = uid++;

		// Add client to the queue and create thread
		enqueue(client);
		pthread_create(&tid, NULL, &handle_connec, (void*)client);

		// To reduce CPU usage 
		sleep(1);
	}

	return EXIT_SUCCESS;
}
