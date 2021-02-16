#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define LENGTH 2048 
#define MAX_EPOLL_EVENTS 64


// Global variables

void str_overwrite_stdout() 
{
  printf("%s", "> ");
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

int main(int argc, char **argv)
{
	int flag = 0;
	char name[32];
	int num_ready;
	char message[LENGTH] = {};
        char buffer[LENGTH + 32] = {};


	if(argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	printf("Please enter your name: ");
  	fgets(name, 32, stdin);
  	str_trim_lf(name, strlen(name));


	if (strlen(name) > 32 || strlen(name) < 2)
	{
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	/* Socket settings */
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  	server_addr.sin_family = AF_INET;
  	server_addr.sin_addr.s_addr = inet_addr(ip);
  	server_addr.sin_port = htons(port);

	struct epoll_event events[MAX_EPOLL_EVENTS];
	int epfd;

        epfd = epoll_create(1);

        struct epoll_event event;
        struct epoll_event event_in;
	event.events = EPOLLIN; //Append "|EPOLLOUT" for write events as well
	event_in.events = EPOLLIN;

	event.data.fd = sockfd;
	event_in.data.fd = 0;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
	epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event_in);

  	// Connect to Server
  	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  	if (err == -1) 
	{
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(sockfd, name, 32, 0);

	printf("=== WELCOME TO THE CHATROOM ===\n");
	
	while (1)
	{
		num_ready = epoll_wait(epfd, events, 64, 10000);
		//num2_ready = epoll_wait(rfd, events, 64, 10000);
		//printf("File descriptor: %d", events[1].data.fd);

		for(int i = 0; i < num_ready; i++)
                {
			
		//	printf("File descriptor: %d\n", events[i].data.fd);
			if(events[i].data.fd == sockfd)
			{
				int receive = recv(sockfd, message, LENGTH, 0);
				if (receive > 0)
				{
					printf("%s", message);
					str_overwrite_stdout();
				}
				else if (receive == 0)
				{
					break;
				}
				else
				{
						// -1
				}
				memset(message, 0, sizeof(message));
			}

			if(events[i].data.fd == 0)
			{
				str_overwrite_stdout();
				fgets(message, LENGTH, stdin);
				str_trim_lf(message, LENGTH);

				if (strcmp(message, "exit") == 0)
				{
					flag = 1;
					sprintf(buffer, "%s", message);
					send(sockfd, buffer, strlen(buffer), 0);
					break;
				}
				else
				{
					sprintf(buffer, "%s: %s\n", name, message);
					send(sockfd, buffer, strlen(buffer), 0);
				}

				bzero(message, LENGTH);
				bzero(buffer, LENGTH + 32);
			}
                }
	
		if(flag)
		{
			printf("\nBye\n");
			break;
    		}
	}

	close(sockfd);

	return EXIT_SUCCESS;
}
