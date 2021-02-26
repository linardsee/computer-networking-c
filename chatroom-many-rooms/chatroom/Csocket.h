#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>

#define RCV_BUFF_SIZE 512 

using namespace std;

class Csocket
{
	public:
		Csocket(){};
		~Csocket(){};
		int InitServer(char* ip, int port);
		int InitServer(int port);
		int InitClient();
	 	int ConnectClient(char* ip, int port);
		int ConnectClient(int port);
		int Accept();	
		//Disconnect();
		char SendDataAck(int sockfd, char* buff, char seqNum);
		char ReceiveDataAck(int sockfd, char* buff);
	private:
		struct sockaddr_in serv_addr;
		struct sockaddr_in cli_addr;
		int socket_fd=0;
};
