#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/epoll.h>

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define MAX_EPOLL_EVENTS 64


static _Atomic unsigned int cli_count = 0;
static int uid = 10;



/* Client structure */
typedef struct
{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void str_overwrite_stdout() 
{
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf (char* arr, int length) 
{
	int i;
  	for (i = 0; i < length; i++) 
	{ // trim \n
    		if (arr[i] == '\n') 
		{
      			arr[i] = '\0';
      			break;
    		}
  	}
}

void print_client_addr(struct sockaddr_in addr)
{
	printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

/* Add clients to queue */
void queue_add(client_t *cl)
{
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i)
	{
		if(!clients[i])
		{
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients to queue */
void queue_remove(int uid)
{
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i)
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

	pthread_mutex_unlock(&clients_mutex);
}

/* Send message to all clients except sender */
void send_message(char *s, int uid)
{
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i)
	{
		if(clients[i])
		{
			if(clients[i]->uid != uid)
			{
				if(write(clients[i]->sockfd, s, strlen(s)) < 0)
				{
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}


// Send message to all clients
void send_all(char *s)
{
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i)
	{
		if(clients[i])
		{
			if(write(clients[i]->sockfd, s, strlen(s)) < 0)
			{
				perror("ERROR: write to descriptor failed");
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}


/* Handle all communication with the client */
void *handle_client(void *arg)
{
	char buff_out[BUFFER_SZ];
	char name[32];
	int leave_flag = 0;
	int num_ready = 0;
	int receive;

	cli_count++;
	client_t *cli = (client_t *)arg;

	struct epoll_event events[MAX_EPOLL_EVENTS];

        int epfd = epoll_create(1);
        struct epoll_event event;
        event.events = EPOLLIN; //Append "|EPOLLOUT" for write events as well
        event.data.fd = cli->sockfd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, cli->sockfd, &event);

	// Name
	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1)
	{
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	} 
	else
	{
		strcpy(cli->name, name);
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		str_overwrite_stdout();
		send_message(buff_out, cli->uid);
	}

	bzero(buff_out, BUFFER_SZ);

	while(1)
	{
		if (leave_flag) 
		{
			break;
		}
		
		num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 10000); 
		//printf("Num ready: %d \n", num_ready);
		//printf("Socket %d \n", events[0].data.fd);

		for(int i = 0; i < num_ready; i++)
		{
			if(events[i].events & EPOLLIN)
			{
				receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
				if (receive > 0)
				{
					if(strlen(buff_out) > 0)
					{
						send_message(buff_out, cli->uid);
							
						str_trim_lf(buff_out, strlen(buff_out));
						printf("%s\n", buff_out);
						str_overwrite_stdout();
					}
				} 
				else if (receive == 0 || strcmp(buff_out, "exit") == 0)
				{
					sprintf(buff_out, "%s has left\n", cli->name);
					printf("%s", buff_out);
					str_overwrite_stdout();
					send_message(buff_out, cli->uid);
					leave_flag = 1;
				} 
				else 
				{
					printf("ERROR: -1\n");
					leave_flag = 1;
				}
			}
		}	

		bzero(buff_out, BUFFER_SZ);
	}

  	/* Delete client from queue and yield thread */
	close(cli->sockfd);
  	queue_remove(cli->uid);
  	free(cli);
  	cli_count--;
  	pthread_detach(pthread_self());

	return NULL;
}

void *send_msg_handler(void *arg)
{
        char message[BUFFER_SZ] = {""};
        char buffer[BUFFER_SZ + 32] = {""};

        client_t *cli = (client_t *)arg;

        while(1)
        {
                str_overwrite_stdout();
                fgets(message, BUFFER_SZ, stdin);
                str_trim_lf(message, BUFFER_SZ);

                if(strcmp(message, "exit") == 0)
                {
                        break;
                }
                else
                {
                        sprintf(buffer, "Server: %s\n", message);
                	send_all(buffer);
		}

                bzero(message, BUFFER_SZ);
                bzero(buffer, BUFFER_SZ + 32);
        }
}

int handle_client_name(client_t * client)
{
	char name[32];
	char buff_out[BUFFER_SZ];	
	int leave = 0;
		
	if(recv(client->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1)
	{
		printf("Didn't enter the name.\n");
		leave = 1;
	}
	else
	{
		strcpy(client->name, name);
		sprintf(buff_out, "%s has joined\n", client->name);
		printf("%s", buff_out);
		str_overwrite_stdout();
		send_message(buff_out, client->uid);
	}

	bzero(buff_out, BUFFER_SZ);
	
	return leave;
}

int main(int argc, char **argv)
{
	int leave_flag = 0, exit_while_flag = 0;
	char buff_out[BUFFER_SZ];
	int receive;

	if(argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);
	int option = 1;
	int listenfd = 0, connfd = 0;
  	struct sockaddr_in serv_addr;
  	struct sockaddr_in cli_addr;
  	pthread_t tid, send_thread;

  	/* Socket settings */
  	listenfd = socket(AF_INET, SOCK_STREAM, 0);
  	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_addr.s_addr = inet_addr(ip);
  	serv_addr.sin_port = htons(port);

	/* Create epoll instance*/
	int num_ready;
	struct epoll_event events[MAX_EPOLL_EVENTS];
        int epfd;

        epfd = epoll_create(1);

        struct epoll_event event;
       // struct epoll_event event_in;
//	struct epoll_event event_listen;
        event.events = EPOLLIN; //Append "|EPOLLOUT" for write events as well
//        event_in.events = EPOLLIN;
//	event_listen.events = EPOLLIN;

        event.data.fd = 0;
        epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event);
	event.data.fd = listenfd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);

  	/* Ignore pipe signals */
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
	{
		perror("ERROR: setsockopt failed");
    		return EXIT_FAILURE;
	}

	/* Bind */
  	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
	{
    		perror("ERROR: Socket binding failed");
    		return EXIT_FAILURE;
  	}

  	/* Listen */
  	if (listen(listenfd, 10) < 0) 
	{
    		perror("ERROR: Socket listening failed");
    		return EXIT_FAILURE;
	}

	printf("=== WELCOME TO THE CHATROOM ===\n");

		
	while(1)
	{
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

		/* Check if max clients is reached */
		if((cli_count + 1) == MAX_CLIENTS)
		{
			printf("Max clients reached. Rejected: ");
			print_client_addr(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
		}

		// Allocate new client
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli_count++;

		event.data.fd = cli->sockfd;
		epoll_ctl(epfd, EPOLL_CTL_ADD, cli->sockfd, &event);
			
		/* Add client to the queue and fork thread */
		queue_add(cli);
		leave_flag = handle_client_name(cli);
		
		while(1)
		{
			num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 10000);
			for(int i = 0; i < num_ready; i++)
			{
				printf("events[%d].data.fd = %d",i, events[i].data.fd);
				if(events[i].data.fd == cli->sockfd)
				{
					receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
					if (receive > 0)
					{
						if(strlen(buff_out) > 0)
						{
							send_message(buff_out, cli->uid);

							str_trim_lf(buff_out, strlen(buff_out));
							printf("%s\n", buff_out);
							str_overwrite_stdout();
						}
					}
					else if (receive == 0 || strcmp(buff_out, "exit") == 0)
					{
						sprintf(buff_out, "%s has left\n", cli->name);
						printf("%s", buff_out);
						str_overwrite_stdout();
						send_message(buff_out, cli->uid);
						leave_flag = 1;
					}
					else
					{
						printf("ERROR: -1\n");
						leave_flag = 1;
					}
				}
				if(events[i].data.fd == 0)
				{
				}
				if(events[i].data.fd == listenfd)
				{
					exit_while_flag = 1;
				}	
			}

			if(leave_flag)
			{
				leave_flag = 0;
				printf("exiting\n");
				close(cli->sockfd);
				queue_remove(cli->uid);
				free(cli);
				cli_count--;
			}

			if(exit_while_flag)
			{
				exit_while_flag = 0;
				break;
			}		
		}
	
		
		//pthread_create(&tid, NULL, &handle_client, (void*)cli);
		//pthread_create(&send_thread, NULL, &send_msg_handler, (void*)cli);

		/* Reduce CPU usage */
		sleep(1);
	}

	return EXIT_SUCCESS;
}
