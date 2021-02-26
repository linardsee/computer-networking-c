
#include <iostream>
#include "Csocket.h"
#include <vector>

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
/*
	char test[100] = "test msg asda sdasd ";
	send(sockfd, test, 100, 0);
	
	for(int i=0; i<65000; i++){}

	char test2[100] = "test msg\0";
	send(sockfd, test2, 100, 0);
	cout << test2[7] << test2[8] << endl;

*/

	char test[32] = "hello world test";
	char test2[32] = "apelsins ir garsigs";
	char test3[32] = "jogur";

	vector <char*> vec_test;
/*	
	vec_test.push_back(test);
	vec_test.push_back(test2);
	vec_test.push_back(test3);
	char test_i = 2;
	for(char i = 0; i < vec_test.size(); i++)
	{	
		cout << "vector element " << vec_test[i] << endl;
		char ack_msg = client.SendDataAck(sockfd, vec_test[i], test_i);
		cout << "ACK number: " << (int)ack_msg << endl;
		test_i--;
	}
*/


	char ack_msg;
        ack_msg	= client.SendDataAck(sockfd, test, 3);
	cout << "ACK number: " << (int)ack_msg << endl;


 	ack_msg	= client.SendDataAck(sockfd, test2, 2);
	cout << "ACK number: " << (int)ack_msg << endl;
	

	ack_msg	= client.SendDataAck(sockfd, test3, 1);
	cout << "ACK number: " << (int)ack_msg << endl;



	return 0;
}
