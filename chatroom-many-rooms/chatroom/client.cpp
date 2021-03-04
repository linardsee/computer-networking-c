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

/* Global variables */
string currentRoom;


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
/*
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
*/


int handleRequest(string str, int sockfd)
{
        int state = 0;
        char buff[MAX_BUFF_SIZE] = {'\0'};
        string inputStr, buffStr;
        vector<char*> roomNames;

        if(str.compare("JOIN") == 0)
        {
                cout << "Enter the room to join: ";
                getline(cin, inputStr);

                // First send command
                stpcpy(buff, "JOIN");
                if (Csocket::SendMessage(sockfd, buff) == -1)
                {
                        cout << "Error: sending command JOIN\n";
                        return -1;
                }
                bzero(buff, MAX_BUFF_SIZE);
                // After that send argument
                stpcpy(buff, inputStr.c_str());
                if (Csocket::SendMessage(sockfd, buff) == -1)
                {
                        cout << "Error: sending argument JOIN\n";
                        return -1;
                }
                bzero(buff, MAX_BUFF_SIZE);

                if( Csocket::ReceiveMessage(sockfd, buff) == -1)
                {
                        cout << "Error: receiving reply JOIN\n";
                        return -1;
                }

                buffStr = buff;

                if( buffStr.compare("ERROR") == 0)
                {
                        cout << "No such room, please enter valid room\n";
                }
                else if( buffStr.compare("OK") == 0)
                {
                        currentRoom = inputStr;
                        state = 1;
                }
                else
                {
                        cout << "Error: unknown reply JOIN\n";
                        state = 2;
                }
        }
	else if(str.compare("CREATE") == 0)
        {
                cout << "Enter the room name to create: ";
                getline(cin, inputStr);

                stpcpy(buff, "CREATE");
                if (Csocket::SendMessage(sockfd, buff) == -1)
                {
                        cout << "Error: sending command CREATE\n";
                        return -1;
                }
                bzero(buff, MAX_BUFF_SIZE);
                // After that send argument
                stpcpy(buff, inputStr.c_str());
                if (Csocket::SendMessage(sockfd, buff) == -1)
                {
                        cout << "Error: sending argument CREATE\n";
                        return -1;
                }
                bzero(buff, MAX_BUFF_SIZE);

                if( Csocket::ReceiveMessage(sockfd, buff) == -1)
                {
                        cout << "Error: receiving reply CREATE\n";
                        return -1;
                }

                buffStr = buff;
		cout << "Buffer received: " << buffStr << endl;

                if( buffStr.compare("ERROR") == 0)
                {
                        cout << "Room already exists, please enter valid room name\n";
                }
                else if( buffStr.compare("OK") == 0)
                {
                        currentRoom = inputStr;
                        state = 1;
                }
                else
                {
                        cout << "Error: unknown reply CREATE\n";
                        state = 2;
                }
        }
        else if(str.compare("SHOW") == 0)
        {
                stpcpy(buff, "SHOW");
                if (Csocket::SendMessage(sockfd, buff) == -1)
                {
                        cout << "Error: sending command SHOW\n";
                        return -1;
                }
                bzero(buff, MAX_BUFF_SIZE);

                if( Csocket::ReceiveMessage(sockfd, buff) == -1)
                {
                        cout << "Error: receiving reply SHOW\n";
                        return -1;
                }

                for(int i = 0; i < int(buff[0]); i++)
                {
                        if( Csocket::ReceiveMessage(sockfd, buff) == -1)
                        {
                                cout << "Error: receiving list SHOW\n";
                                return -1;
                        }
                        roomNames.push_back(buff);
                        bzero(buff, MAX_BUFF_SIZE);
                }

                cout << endl;
                cout << "List of available rooms: \n";

                for(int i = 0; i < roomNames.size(); i++)
                {
                        cout << roomNames[0] << endl;
                }

                cout << endl;
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
				
				if (handleRequest(inputStr, sockfd))
				{
					cout << "Requests handled\n";
				}	
			}
		}
	}

	return 0;
}
