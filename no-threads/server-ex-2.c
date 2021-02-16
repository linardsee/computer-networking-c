#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
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
	for(int i=0; i < MAX_CLIENTS; ++i)
	{
		if(!clients[i])
		{
			clients[i] = cl;
			break;
		}
	}
}

/* Remove clients to queue */
void queue_remove(int uid)
{
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
}

/* Send message to all clients except sender */
void send_message(char *s, int uid)
{
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
}
sprintf(buffer, "Server: %s\n", message);
                        send_all(buffer);


// Send message to all clients
void send_all(char *s)
{
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
}

int handle_client_name(client_t * client)
{
        char name[32];
        char buff_out1[BUFFER_SZ];
        int leave;

        if(recv(client->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1)
        {
                printf("Didn't enter the name.\n");
                leave = 1;
        }
        else
        {
                strcpy(client->name, name);
                sprintf(buff_out1, "%s has joined\n", client->name);
                printf("%s", buff_out1);
                str_overwrite_stdout();
                //send_message(buff_out, client->uid);
        	send_all(buff_out1);
	}

        bzero(buff_out1, BUFFER_SZ);

        return leave;
}


int main(int argc, char **argv)
{
	int leave_flag = 0;
	client_t* cli = NULL;
	char buff_out[BUFFER_SZ];	
	char buff[BUFFER_SZ+32];

	if(argc != 2)
        {
                printf("Usage: %s <port>\n", argv[0]);
                return EXIT_FAILURE;
        }
	
	char *ip = "127.0.0.1";
        int port = atoi(argv[1]);
        int option = 1;
        struct sockaddr_in serv_addr;
        struct sockaddr_in cli_addr;
	
	/* Socket settings */
	int listenfd, connfd;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(ip);
        serv_addr.sin_port = htons(port);
	int receive;

	/* Create epoll instance */
	int num_ready;
        struct epoll_event events[MAX_EPOLL_EVENTS];
        int epfd;
	epfd = epoll_create(1);
	struct epoll_event event;
	event.events = EPOLLIN; //Append "|EPOLLOUT" for write events as well
	event.data.fd = 0;
        epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event);
        event.data.fd = listenfd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);


	/* Socket initialization */
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
		num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 30000);
		int iterator = 0;	
		for(int i = 0; i < num_ready; i++)
		{
			if(events[i].data.fd == listenfd)
			{
				// LISTEN EVENTS
				socklen_t clilen = sizeof(cli_addr);
        	        	connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

				if((cli_count + 1) == MAX_CLIENTS)
				{
					printf("Max clients reached. Rejected: ");
					print_client_addr(cli_addr);
					printf(":%d\n", cli_addr.sin_port);
					close(connfd);
				}

				// Allocate new client
				client_t* cli = (client_t *)malloc(sizeof(client_t));
                		cli->address = cli_addr;
                		cli->sockfd = connfd;
                		cli->uid = uid++;

				cli_count++;
				queue_add(cli);

				event.data.fd = cli->sockfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, cli->sockfd, &event);

				leave_flag = handle_client_name(cli);
			}
			else if(events[i].data.fd == 0)
			{
				// STD IN EVENTS
        		        fgets(buff_out, BUFFER_SZ, stdin);
	                	str_trim_lf(buff_out, BUFFER_SZ);
				sprintf(buff, "Server: %s\n", buff_out);
                        	send_all(buff);
				str_overwrite_stdout();
			
				bzero(buff_out, BUFFER_SZ);
				bzero(buff, BUFFER_SZ+32);
			}
			else
			{
				// RECEIVE EVENTS
				do
				{
					if(clients[iterator] == NULL)
						iterator++;
					else
					{
						if(clients[iterator]->sockfd == events[i].data.fd)
						{	
							break;
						}
						else
							iterator++;
					}

				}while(iterator < MAX_CLIENTS);
				
				//printf("DEBUG: events[i].data.fd = %d\n", events[i].data.fd);
				//printf("DEBUG: clients[iterator]->sockfd = %d\n", clients[iterator]->sockfd);
				
				if(iterator == MAX_CLIENTS)
				{
					printf("No client found!\n");
				}
				
				
				receive = recv(clients[iterator]->sockfd, buff_out, BUFFER_SZ, 0);

				if (receive > 0)
				{	
					if(strlen(buff_out) > 0)
					{
						if(strcmp(buff_out, "exit") == 0)
						{
							bzero(buff_out, BUFFER_SZ);
							//printf("DEBUG: exit pressed\n");
							sprintf(buff_out, "%s has left\n", clients[iterator]->name);
							printf("%s", buff_out);
							send_message(buff_out, clients[iterator]->uid);
							str_overwrite_stdout();
							leave_flag = 1;
						}
						else
						{
							send_message(buff_out, clients[iterator]->uid);
							str_trim_lf(buff_out, strlen(buff_out));
							printf("%s\n", buff_out);
							str_overwrite_stdout();
						}
					}
				}
				else if (receive == 0)
				{
					//printf("DEBUG: Receive == 0\n");
					sprintf(buff_out, "%s has left\n", clients[iterator]->name);
					printf("%s", buff_out);
					send_message(buff_out, clients[iterator]->uid);
					str_overwrite_stdout();
					leave_flag = 1;
				}
				else
				{
					printf("ERROR: -1\n");
					leave_flag = 1;
				}

				bzero(buff_out, BUFFER_SZ);
				
			}

			if(leave_flag == 1)
			{
				//printf("DEBUG: leave_flag = 1, exiting\n");
				leave_flag = 0;
				close(clients[iterator]->sockfd);
				//queue_remove(clients[iterator]->uid);
				//printf("Releasing %s", clients[iterator]->name);
				free(clients[iterator]);
				clients[iterator] = NULL;
				cli_count--;
			}
		}// for loop ends
	}// while 1 ends
	return EXIT_SUCCESS;
} // int main ends
