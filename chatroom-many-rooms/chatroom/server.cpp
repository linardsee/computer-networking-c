
#include "Csocket.h"
#include "CClient.h"
#include "CRoom.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <list>
#include <cstdlib>
#include <algorithm>


#define MAX_EPOLL_EVENTS 64
#define MAX_BUFF_SIZE 1024

/* GLOBAL VARIABLES */
list<CClient*> OnlineClients;
list<CClient> OfflineClients;
list<CClient*>::iterator it_on;
list<CClient>::iterator it_off;
list<CRoom> Rooms;
list<CRoom>::iterator it_room;

CRoom room;

using namespace std;

// Functors
class FindByName
{
        string name;

        public:
                FindByName(const string& name) : name(name) {}
			
                bool operator()( CRoom room) const
                {
                        return !room.getRoomName().compare(name);
                }
};

class FindBySockfd
{
        int sockfd;

        public:
                FindBySockfd(const int& sockfd) : sockfd(sockfd) {}

                bool operator ()(CClient* client) const
                {
                        return client->getSockfd() == sockfd;
                }
};


int handleClientName(CClient* theClient)
{
	char theName[32] = {};
	char sendBuff[MAX_BUFF_SIZE] = {};
	string nameStr;
	int state = 0;
	int receive = 0;
	
	// Get the name string
	while(receive == 0)
	{
		receive = Csocket::ReceiveMessage(theClient->getSockfd(), theName);
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
		if( Csocket::SendMessage(theClient->getSockfd(), sendBuff) == -1 )
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
		for(it_on = OnlineClients.begin(); it_on != OnlineClients.end(); ++it_on)
		{
			if( (*it_on)->getClientName().compare(nameStr) == 0 )
			{
				strcpy(sendBuff, "ERROR");
				if( Csocket::SendMessage(theClient->getSockfd(), sendBuff) == -1 )
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
		for(it_off = OfflineClients.begin(); it_off != OfflineClients.end(); ++it_off)
		{
			if( (*it_off).getClientName().compare(nameStr) == 0 )
			{
				theClient->setClientName(nameStr);
				theClient->setLastRoom((*it_off).getLastRoom());
				OnlineClients.push_back(theClient);
				
				strcpy(sendBuff, (*it_off).getLastRoom().c_str());

				if( Csocket::SendMessage(theClient->getSockfd(), sendBuff) == -1 ) 
				{
					cout << "Error: sending error reply\n";
					return -1;
				}
				
				cout << nameStr << " connected\n";	
				return 1;	
			}
		}
		
		// If client is not present in both online and offline, then make new
		theClient->setClientName(nameStr);

		OnlineClients.push_back(theClient);
		OfflineClients.push_back(*theClient);
		cout << nameStr << " connected\n";

		strcpy(sendBuff, "OK");
		if( Csocket::SendMessage(theClient->getSockfd(), sendBuff) == -1 ) 
		{
			cout << "Error: sending error reply\n";
			return -1;
		}
	}

	return 1;
}

int broadcastMsg(char* buff, string room)
{
        for(it_on = OnlineClients.begin(); it_on != OnlineClients.end(); ++it_on)
        {
                if( room.compare( (*it_on)->getLastRoom() ) == 0)
                {
			Csocket::SendMessage((*it_on)->getSockfd(), buff);
                }
        }

	return 0;
}


int handleCreateCommand(int sockfd)
{
        char buff[MAX_BUFF_SIZE] = {'\0'};
        string buffStr;

        if (Csocket::ReceiveMessage(sockfd, buff) == -1)
        {
                cout << "Error: receiving  message in handleCreateCommand\n";
                return -1;
        }
        buffStr = buff;
        bzero(buff, MAX_BUFF_SIZE);
	
        it_room = find_if(Rooms.begin(), Rooms.end(), FindByName(buffStr));


        if(it_room == Rooms.end())
        {
	        Rooms.push_back(room);
                it_room = --Rooms.end();
		it_room->setRoomName(buffStr);
			
		it_on = OnlineClients.begin();


       		it_on = find_if(OnlineClients.begin(), OnlineClients.end(), FindBySockfd(sockfd));
	
                if( ((*it_on)->getLastRoom()) != "")
                {
                        (*it_on)->setLastRoom(buffStr);
                        ++(*it_room);

                        it_room = find_if(Rooms.begin(), Rooms.end(), FindByName((*it_on)->getLastRoom()));
                        --(*it_room);
                }
                else
                {
                        (*it_on)->setLastRoom(buffStr);
                        ++(*it_room);
                }

                stpcpy(buff, "OK");
		Csocket::SendMessage(sockfd, buff);
                cout << (*it_on)->getClientName() << " created " << buffStr << endl;
		
                return 1;
        }
        else
        {
                stpcpy(buff, "ERROR");
		Csocket::SendMessage(sockfd, buff);
                cout << "Invalid room name\n";

                return 2;
        }

}


int handleReceiveMsg(string rcvStr, int sockfd)
{
        if( rcvStr.compare("JOIN") == 0 )
        {
                //handleJoinCommand(sockfd);
        }
        else if( rcvStr.compare("CREATE") == 0 )
        {
                handleCreateCommand(sockfd);
        }
        else if( rcvStr.compare("CREATE") == 0 )
        {
                //handleShowCommand(sockfd);
        }
	else
	{
	}

	return 0;
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

					if(handleClientName(client) == -1 )
					{
						cout << "Error handling clients names\n";
					}
				}
			}
			else if(events[i].data.fd == 0)
			{
				// Input events
				
			}
			else
			{
				// Receive events
	        		epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &event);
				if (server.ReceiveMessage(events[i].data.fd, rcv_buff) == -1)
				{
					cout << "Error: receiving  message\n";
					return -1;
				}
				rcvStr = rcv_buff;
								
				if( handleReceiveMsg(rcvStr, events[i].data.fd) )
				{
					cout << "done\n" << endl;
				}	
				bzero(rcv_buff, MAX_BUFF_SIZE);
				epoll_ctl(epfd, EPOLL_CTL_ADD, events[i].data.fd, &event);

			}
		}
	}
	
	
	return 0;
}
