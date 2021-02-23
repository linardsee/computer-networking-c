#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
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
		//SendData();
		//ReceiveData();
	private:
		struct sockaddr_in serv_addr;
		struct sockaddr_in cli_addr;
		int socket_fd=0;
};
