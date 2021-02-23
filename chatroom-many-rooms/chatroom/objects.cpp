

#include <iostream>
#include <string>

using namespace std;

void printMenu(void);

class CRoom()
{
	public:
		CRoom();
		~CRoom();
		unsigned char getMaxParticipants(void);
		void setMaxParticipants(const unsigned char num);
		unsigned char getActiveMembers(void);
		void setActiveMembers(unsigned char num);
		string getOpName(void);
		void setOpName(const string name);
		string getRoomName(void);
		void setRoomName(const string name);

	private:
		unsigned char maxParticipants, activeMemebers;
		string opName, roomName;
};

class CClient()
{
	public:
		CClient();
		~CClient();
		int getUid(void);
		void setUid(const int userId);
		int getSockfd(void);
		void setSockfd(const int socketfd);
		string getClientName(void);
		void setClientName(const string theName);
		string getRoomName(void);
		void setRoomActive(const string theRoom);
		void setAddress(struct sockaddr_in theAddress);	

	private:
		int uid, sockfd;
		string clientName, roomActive;
		struct sockaddr_in address;
};

int main(int argc, char **argv)
{
	while(1)
	{
	}
	return 0;
}

void printMenu(void)
{
	cout << "============= WELCOME TO CHAT ROOM ===============\n";
	cout << "=============== CHOOSE AN OPTION =================\n";
	cout << "JOIN ROOM (1)\n";
	cout << "CREATE ROOM (2)\n";	
	cout << "SHOW AVAILABLE ROOMS (3)\n";	
	cout << "QUIT (4)\n";
	cout << "==================================================\n";
	cout << "==================================================\n";
}
