
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

char Csocket::SendDataAck(int sockfd, char* buff, char seqNum)
{
	cout << "buff len = " << strlen(buff) << endl;
	int len = strlen(buff);
	buff[len] = seqNum; // Add message number in the end
	char ack[1];
	
	cout << "After adding buff len = " << strlen(buff) << endl;
	send(sockfd, buff, strlen(buff), 0);
	while(1)
	{
		if( recv(sockfd, ack, 1, 0))
		{
			if(ack[0] == seqNum)
			{
				cout << "Received ACK: " << (int)seqNum << endl;
			}
			else
				ack[0] = -1;
			break;
		}
	}

	return ack[0];
}

char Csocket::ReceiveDataAck(int sockfd, char* buff)
{
	char rcvData[RCV_BUFF_SIZE];
	char ack[1];	
	cout << "Size of rcvData = " << sizeof(rcvData) << endl;
	while(1)
	{
		if(recv(sockfd, rcvData, RCV_BUFF_SIZE, 0) )
		{
			cout << "Received data before strncpy: " << rcvData << endl;

			ack[0] = rcvData[strlen(rcvData)-1];	
			strncpy(buff, rcvData, strlen(rcvData));
			cout << "Received data: " << buff << endl;
			send(sockfd, ack, 1, 0);
			
			break;
		}
	}	

	return ack[0];
}
