
#include "Csocket.h"


int Csocket::Init(char* ip, int port)
{
	int descriptor = socket(AF_INET, SOCK_STREAM, 0);
        int option = 1;
	
	serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(ip);
        serv_addr.sin_port = htons(port);

	if(setsockopt(descriptor, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
        {
		cerr << "ERROR: setsockopt failed!";
               // perror("ERROR: setsockopt failed");
                return EXIT_FAILURE;
        }

        /* Bind */
        if(bind(descriptor, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        {
		cerr << "ERROR: Socket binding failed!";
               // perror("ERROR: Socket binding failed");
                return EXIT_FAILURE;
        }

        /* Listen */
        if (listen(descriptor, 10) < 0)
        {
		cerr << "ERROR: Socket listening failed!";
                // perror("ERROR: Socket listening failed");
                return EXIT_FAILURE;
        }

	return descriptor;
}

