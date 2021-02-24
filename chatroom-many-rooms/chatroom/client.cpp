
#include <iostream>
#include "Csocket.h"

using namespace std;

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		cout << "Usage: " << argv[0] << " port\n";
		return EXIT_FAILURE;
	}
	int port = atoi(argv[1]);

	Csocket client;
	int sockfd = client.InitClient();
	
	cout << "Descriptor created: " << sockfd << endl;
	
	int err = client.ConnectClient(port);
	if(err == -1)
		cout << "Error\n";
	else
		cout << "Connection successful!\n";
	
	char name[32] = "pipis";
	send(sockfd, name, 32, 0);

	return 0;
}
