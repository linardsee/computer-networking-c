
#include "Csocket.h"
#include "CClient.h"
#include <list>
#include <sys/epoll.h>
#include <cstring>

#define MAX_EPOLL_EVENTS 64
#define RCV_BUFF_SIZE 512

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
	char rcv_buff[RCV_BUFF_SIZE];
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

					//handleClientName(client, OfflineClients, it_off);
					
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
				/*receive = recv(events[i].data.fd, rcv_buff, RCV_BUFF_SIZE, 0);
				if(receive > 0)
				{
					// Message received
					rcvStr = rcv_buff;
					cout << "The received message is: " << rcvStr << endl;
					cout << rcvStr.size() << endl;
					cout << rcvStr[rcvStr.size()-1] << endl;
					cout << sizeof(rcv_buff) << endl;
					cout << rcv_buff[rcvStr.size()-2] << " " << rcv_buff[rcvStr.size()-1] << endl; 
					if( rcvStr[rcvStr.size()-1] == '\0')
						cout << "Message wont continue\n";
					else
						cout << "This is not the end\n";					
				}
				bzero(rcv_buff, RCV_BUFF_SIZE);
				*/
				//if (server.ReceiveDataAck(events[i].data.fd) == 0 )
				//	cout << "Received a single message\n";
				//else
				//	cout << "Received message which must continue\n";
				
			/*	
				char number = 0;
				while(number >= 0)
				{
					number = server.ReceiveDataAck(events[i].data.fd, rcv_buff); 
					if( number >= 0)
					{
						cout << "Sequence number = " << (int)number << endl;
						cout << "Content of the message = " << rcv_buff << endl;
						cout << endl;
						if(!number)
							break;
						bzero(rcv_buff, RCV_BUFF_SIZE);
 
					}

				}
			*/
				cout << "descriptor: " << events[i].data.fd << endl;
				cout << "Receiveign\n";
				char number = server.ReceiveDataAck(events[i].data.fd, rcv_buff); 
				cout << "Data received: " << rcv_buff << endl;
				bzero(rcv_buff, RCV_BUFF_SIZE);
			}
		}
	}
	
	
	return 0;
}
