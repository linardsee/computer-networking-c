
#include "Csocket.h"
#include "CClient.h"
#include <list>
#include <sys/epoll.h>
#include <cstring>

#define MAX_EPOLL_EVENTS 64

/* GLOBAL VARIABLES */

using namespace std;

void handleClientName(CClient* theClient, list<CClient*>& theOffList, list<CClient*>::iterator& it_offlist)
{
	char name[32];
	string theName;
	int state = 0;
	
	if(recv(theClient->getSockfd(), name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1)
        {
		// Handle error
        }
	else
	{
		theName = name;
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
	}

	if(!state)
	{
		theClient->setClientName(theName);
		//theClient->setUid(uid);
	}
	else
	{
	}
}


int main(int argc, char** argv)
{
	if(argc != 2)
	{
		cout << "Usage: " << argv[0] << " port\n";
		return EXIT_FAILURE;
	}
	int port = atoi(argv[1]);
	
	list<CClient*> OnlineClients;
	list<CClient*> OfflineClients;
	list<CClient*>::iterator it_on;
	list<CClient*>::iterator it_off;


	// Create and init the listen socket
	Csocket server;
	int listenfd = server.InitServer(port);	
	int connfd;
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

		for(int i = 0; i < num_ready; i++)
		{
			if(events[i].data.fd == listenfd)
			{
				// Connection events
				connfd = server.Accept();
				cout << "Connected\n";
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

					handleClientName(client, OfflineClients, it_off);
					
					// Send room names

				}
			}
			else if(events[i].data.fd == 0)
			{
				// Input events
			}
			else
			{
				// Receive events
			}
		}
	}
	
	
	return 0;
}
