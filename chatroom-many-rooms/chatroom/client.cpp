#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include "Csocket.h"


#define MAX_EPOLL_EVENTS 64
#define READ_SIZE 1024
#define MAX_BUFF_SIZE 1024

using namespace std;


void printStartWindow()
{
	cout << "Commands:\n";
	cout << "JOIN -> join a room\n";
	cout << "SHOW_ROOMS -> show all rooms in this server\n";
	cout << "SWITCH -> switch to another rooms\n";
	cout << "CREATE -> create a new room\n";
	cout << "HELP -> print all commands\n";
	cout << "EXIT -> exit this application\n";
}

void handleNameReply(string buff, string& theName, string& theRoomName)
{
	if( buff.compare("ERROR") == 0)
	{
		cout << "Error: bad login name\n";
		exit(EXIT_FAILURE);
	}
	
	if( buff.compare("OK") == 0)
	{
		cout << "New client connected successfully\n";
		buff = "";
		cout << "===== WELCOME TO CHATROOM =====\n";
		cout << endl;
		printStartWindow();
		cout << endl;
	}
	
	if( (buff.compare("OK") != 0) && (buff.compare("ERROR") != 0) )
	{
		theRoomName = buff;
		cout << theName << "@" << theRoomName << ">" << flush;
	}
}

int checkIfRequest(string str)
{
	int state = 0;

	if(str.compare("JOIN") == 0)
	{
		cout << "JOIN pressed\n";
		cout << "> " << flush;
		state = 1;
	}
	else if (str.compare("SHOW_ROOMS") == 0)
	{
		cout << "SHOW_ROOMS pressed\n";
		cout << "> " << flush;
		state = 2;
	}	
	else if (str.compare("SWITCH") == 0)
	{
		cout << "SWITCH pressed\n";
		cout << "> " << flush;
		state = 3;
	}	
	else if (str.compare("CREATE") == 0)
	{
		cout << "CREATE pressed\n";
		cout << "> " << flush;
		state = 4;
	}	
	else if (str.compare("HELP") == 0)
	{
		cout << "HELP pressed\n";
		cout << "> " << flush;
		state = 5;
	}	
	else if (str.compare("EXIT") == 0)
	{
		cout << "EXIT pressed\n";
		cout << "> " << flush;
		state = 6;
	}

	return state;
}

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		cout << "Usage: " << argv[0] << " port\n";
		return EXIT_FAILURE;
	}
	int port = atoi(argv[1]);

	Csocket client;
	string inputStr, buffStr, currentRoom, userName;
	char inputChar[32] = {};
	char dataBuff[MAX_BUFF_SIZE] = {};
	int req;

	cout << "> Enter your login name: ";
	getline(cin, inputStr);
	userName = inputStr;
	strcpy(inputChar, inputStr.c_str());

	int sockfd = client.InitClient();
	cout << "Descriptor created: " << sockfd << endl;
	
	int err = client.ConnectClient(port);
	if(err == -1)
		cout << "Error\n";
	else
		cout << "Connection successful!\n";
	
	int num_ready;
        struct epoll_event events[MAX_EPOLL_EVENTS];
        int epfd;
        epfd = epoll_create(1);
        struct epoll_event event;
        event.events = EPOLLIN; //Append "|EPOLLOUT" for write events as well
        event.data.fd = 0;
        epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event);
        event.data.fd = sockfd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);

	
	if(client.SendMessage(sockfd, inputChar) == -1)
	{
		cout << "Error: sending name\n";
		return 0;
	}

	if(client.ReceiveMessage(sockfd, dataBuff) == -1)
	{
		cout << "Error: receiving name accept\n";
		return 0;
	}
	
	buffStr = dataBuff;
	bzero(dataBuff, MAX_BUFF_SIZE);
	handleNameReply(buffStr, userName, currentRoom);

	while(1)
	{
		num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 30000);
		//cout << "Num ready: " << num_ready << endl;
		for(int i = 0; i < num_ready; i++)
		{
			if(events[i].data.fd == sockfd)
			{
				// Receive events
			}
			else if(events[i].data.fd == 0)
			{
				// Input events
				cout << "> ";
				getline(cin, inputStr);
				
				req = checkIfRequest(inputStr);
				if(req)
				{
					switch(req)
					{
						case 1:
							//  Exit routine
						break;

						case 2:
							// JOIN routine
						break;

						case 3:
							// SHOW_ROOMS routine
						break;

						case 4:
							// CHANGE routine
						break;

						case 5:
							// HELP routine
						break;
					}
				}
				else
				{
					// TEXT FOR CHAT
				}
			}
		}
	}

	return 0;
}
