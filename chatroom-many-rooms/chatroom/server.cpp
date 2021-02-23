
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

	Csocket server;
	int listenfd = server.InitServer(port);
	
	cout << "Descriptor created: " << listenfd << endl;
	
	int connfd = server.Accept();
	
	if(connfd)
		cout << "Connection established\n";

	cout << "connfd = " << connfd << endl;
	
	return 0;
}
