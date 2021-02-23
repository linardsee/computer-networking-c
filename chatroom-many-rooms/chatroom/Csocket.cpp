
#include "Csocket.h"


int Csocket::InitServer(char* ip, int port)
{
	int descriptor = socket(AF_INET, SOCK_STREAM, 0);
        int option = 1;
	
	serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(ip);
        serv_addr.sin_port = htons(port);

	if(setsockopt(descriptor, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
        {
		std::cerr << "ERROR: setsockopt failed!";
               // perror("ERROR: setsockopt failed");
                return EXIT_FAILURE;
        }

        /* Bind */
        if(bind(descriptor, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        {
		std::cerr << "ERROR: Socket binding failed!";
               // perror("ERROR: Socket binding failed");
                return EXIT_FAILURE;
        }

        /* Listen */
        if (listen(descriptor, 10) < 0)
        {
		std::cerr << "ERROR: Socket listening failed!";
                // perror("ERROR: Socket listening failed");
                return EXIT_FAILURE;
        }

	socket_fd = descriptor;

	return descriptor;
}

//Overloading for loopback address case
int Csocket::InitServer(int port)
{
	char ip[] = "127.0.0.1";
	int descriptor = socket(AF_INET, SOCK_STREAM, 0);
        int option = 1;
	
	serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(ip);
        serv_addr.sin_port = htons(port);

	if(setsockopt(descriptor, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
        {
		std::cerr << "ERROR: setsockopt failed!";
               // perror("ERROR: setsockopt failed");
                return EXIT_FAILURE;
        }

        /* Bind */
        if(bind(descriptor, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        {
		std::cerr << "ERROR: Socket binding failed!";
               // perror("ERROR: Socket binding failed");
                return EXIT_FAILURE;
        }

        /* Listen */
        if (listen(descriptor, 10) < 0)
        {
		std::cerr << "ERROR: Socket listening failed!";
                // perror("ERROR: Socket listening failed");
                return EXIT_FAILURE;
        }
	
	socket_fd = descriptor;

	return descriptor;
}

int Csocket::InitClient()
{
	int descriptor = socket(AF_INET, SOCK_STREAM, 0);		      socket_fd = descriptor;

	return descriptor;
}

int Csocket::ConnectClient(char* ip, int port)
{
	serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(ip);
        serv_addr.sin_port = htons(port);

	int err = connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	return err;
}

//Overloading for loopback address case
int Csocket::ConnectClient(int port)
{
	char ip[] = "127.0.0.1";

	serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(ip);
        serv_addr.sin_port = htons(port);

	int err = connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	return err;
}

int Csocket::Accept()
{
	socklen_t clilen = sizeof(cli_addr);
	int connfd = accept(socket_fd, (struct sockaddr*)&cli_addr, &clilen);

	return connfd;
}
