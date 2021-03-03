
#include "Csocket.h"
#include "CClient.h"
#include "CRoom.h"
#include <list>
#include <sys/epoll.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

#define MAX_EPOLL_EVENTS 64
#define MAX_BUFF_SIZE 1024

/* GLOBAL VARIABLES */
list<CClient*> OnlineClients;
list<CClient> OfflineClients;
list<CClient*>::iterator it_on;
list<CClient>::iterator it_off;
list<CRoom> Rooms;
list<CRoom>::iterator it_room;


using namespace std;

int handleReceiveMsg(string& recvStr, int sockfd)
{

	if( recvStr.compare("JOIN") == 0 )
	{
		if( server.ReceiveMessage(sockfd, rcv_buff) == -1 )
		{
			cout << "Error: receiving JOIN argument\n";
			return -1;
		}
		//Find this room in list if it is there
		//Increment this room member count
		//Decrement previous room member count
		//Change Client's active room
		//Send answer to the client
	}
	else if( recvStr.compare("SHOW_ROOMS") == 0 )
	{
		// send list of rooms
	}
	else if( recvStr.compare("CREATE") == 0 )
	{
		// Receive routine
		// Check if name is unique
		// Add to room list
		// Increment this room member count
		// Change clients active room
		// send answer to the client
	}
	else
	{
		// standard messages
		// broadcast to other clients
	}

}

int handleClientName(Csocket &theServer, CClient* theClient, list<CClient*>& theOnList, list<CClient*>::iterator& it_on, list<CClient>& theOffList, list<CClient>::iterator& it_off)
{
	char theName[32] = {};
	char sendBuff[MAX_BUFF_SIZE] = {'\0'};
	string nameStr;
	int state = 0;
	int receive = 0;
	
	// Get the name string
	while(receive == 0)
	{
		receive = theServer.ReceiveMessage(theClient->getSockfd(), theName);
		if( receive == -1 )
		{
			cout << "Error: receiving client's name\n";
			return -1;
		}	
	}
	nameStr = theName;
	
	// Check the contents of name string
	if( (nameStr.length() < 2) || (nameStr.length() >= 31) )
	{
		cout << "Login size is not valid\n";
		strcpy(sendBuff, "ERROR");
		if( theServer.SendMessage(theClient->getSockfd(), sendBuff) == -1 )
		{
			cout << "Error: sending error reply\n";
			return -1;
		}
		close(theClient->getSockfd());
		delete theClient;
		bzero(sendBuff, MAX_BUFF_SIZE);
		return 0;
	}
	else
	{
		// First check online list to avoid collision on same login names
		for(it_on = theOnList.begin(); it_on != theOnList.end(); ++it_on)
		{
			if( (*it_on)->getClientName().compare(nameStr) == 0 )
			{
				strcpy(sendBuff, "ERROR");
				if( theServer.SendMessage(theClient->getSockfd(), sendBuff) == -1 )
				{
					cout << "Error: sending error reply\n";
					return -1;
				}
				close(theClient->getSockfd());
				delete theClient;
				bzero(sendBuff, MAX_BUFF_SIZE);
				return 0;
			}
		}
	
		// Now check the offline list to check if login name is already registered 	
		for(it_off = theOffList.begin(); it_off != theOffList.end(); ++it_off)
		{
			if( (*it_off).getClientName().compare(nameStr) == 0 )
			{
				theOnList.push_back(theClient);
				it_on = --theOnList.end();
				(*it_on)->setClientName((*it_off).getClientName());
				(*it_on)->setLastRoom((*it_off).getLastRoom());
				// need to copy all parameters
				
				strcpy(sendBuff, (*it_off).getLastRoom().c_str());

				if(theServer.SendMessage(theClient->getSockfd(), sendBuff) == -1 ) 
				{
					cout << "Error: sending error reply\n";
					return -1;
				}
				
				bzero(sendBuff, MAX_BUFF_SIZE);
				return 1;	
			}
		}

		theOnList.push_back(theClient);
		theOffList.push_back(*theClient);
		
		strcpy(sendBuff, "OK");
		if(theServer.SendMessage(theClient->getSockfd(), sendBuff) == -1 ) 
		{
			cout << "Error: sending error reply\n";
			return -1;
		}
		
		bzero(sendBuff, MAX_BUFF_SIZE);

	}

	return 1;
}


int main(int argc, char** argv)
{
	if(argc != 2)
	{
		cout << "Usage: " << argv[0] << " port\n";
		return EXIT_FAILURE;
	}
	int port = atoi(argv[1]);
	char rcv_buff[MAX_BUFF_SIZE] = {};
	string rcvStr;
	
// TEST PURPOSE ONLY
	CClient client1;
	client1.setClientName("Linards");
	CClient client2;
	client2.setClientName("Apelsins");
	client2.setLastRoom("Hardware");

	OnlineClients.push_back(&client1);
	OfflineClients.push_back(client2);
// TEST PURPOSE ONLY ENDS

	// Create and init the listen socket
	Csocket server;
	int listenfd = server.InitServer(port);	
	int connfd, receive;
	
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

					event.data.fd = connfd; 
					epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);

					if(handleClientName(server, client, OnlineClients, it_on, OfflineClients, it_off) == -1 )
					{
						cout << "Error handling clients names\n";
					}
					else
					{
						cout << "Good\n";
					}
					
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
				if (server.ReceiveMessage(events[i].data.fd, rcv_buff) == -1)
				{
					cout << "Error: receiving  message\n";
					return -1;
				}
				rcvStr = rcv_buff;
				
				handleReceiveMsg(rcvStr);	
			}
		}
	}
	
	
	return 0;
}
