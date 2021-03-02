
#include "Csocket.h"
#include "CClient.h"
#include <list>
#include <sys/epoll.h>
#include <cstring>
#include <unistd.h>

#define MAX_EPOLL_EVENTS 64
#define RCV_BUFF_SIZE 512

/* GLOBAL VARIABLES */

using namespace std;

void handleClientName(Csocket &theServer, CClient* theClient, list<CClient*>& theOffList, list<CClient*>::iterator& it_offlist)
{
	char theName[32] = {};
	int state = 0;

	if( theServer.ReceiveMessage(theClient->getSockfd(), theName) == -1 )
		cout << "Error receiving client name\n";
		// Need to handle

	if( strlen(theName) <  2 || strlen(theName) >= 32-1)
        {
		cout << "Bad name, exit\n";
		// Need to handle 
        }
	else
	{
		//Check the offline list
		for(it_offlist = theOffList.begin(); it_offlist != theOffList.end(); ++it_offlist)
		{
			if( ((*it_offlist)->getClientName()).compare(theName) == 0)
			{
				cout << "The name is already registered!\n";
				// Copy info from offline to online	
				state = 1;
			}
		}

		// Also need to check online list, because of conflicts
	}

	if(!state)
	{
		theClient->setClientName(theName);
		// think about uid
	}
	else
	{
	}

	cout << theClient->getClientName() << " connected\n";
}


int main(int argc, char** argv)
{
	if(argc != 2)
	{
		cout << "Usage: " << argv[0] << " port\n";
		return EXIT_FAILURE;
	}
	int port = atoi(argv[1]);
	char rcv_buff[RCV_BUFF_SIZE] = {'\0'};
	string rcvStr;

	list<CClient*> OnlineClients;
	list<CClient*> OfflineClients;
	list<CClient*>::iterator it_on;
	list<CClient*>::iterator it_off;


	// Create and init the listen socket
	Csocket server;
	int listenfd = server.InitServer(port);	
	int connfd, receive;
	cout << "Descriptor created: " << listenfd << endl;
	
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


	while(1)
	{
		num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 30000);
		cout << "Num ready: " << num_ready << endl;
		for(int i = 0; i < num_ready; i++)
		{
			cout << "i =  " << i << endl;
			if(events[i].data.fd == listenfd)
			{
				// Connection events
				connfd = server.Accept();
				// here we need to limit client quantity and reject if necessary
				if(connfd == -1)
				{
					// Error	
				}
				else
				{
					// Allocate new client
					CClient* client = new CClient;
					client->setSockfd(connfd);
        				OnlineClients.push_back(client);

					event.data.fd = client->getSockfd();
					epoll_ctl(epfd, EPOLL_CTL_ADD, client->getSockfd(), &event);

					handleClientName(server, client, OfflineClients, it_off);
					
					// Send room names
					cout << "Connected\n";
				}
			}
			else if(events[i].data.fd == 0)
			{
				// Input events
				
			}
			else
			{
				// Receive events
				/*
				cout << "descriptor: " << events[i].data.fd << endl;
				cout << "Receiveign\n";
				char number = server.ReceiveDataAck(events[i].data.fd, rcv_buff); 
				cout << "Data received: " << rcv_buff << endl;
				bzero(rcv_buff, RCV_BUFF_SIZE);
				//close(events[i].data.fd);
				*/

				if (server.ReceiveMessage(events[i].data.fd, rcv_buff))
					cout << "Received data: " << rcv_buff << endl;
				
			}
		}
	}
	
	
	return 0;
}
