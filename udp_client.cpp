#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>

#define PORT 8001
using namespace std;
struct octoLeg{
  int octoBlockID;
  int octoLegID;
  int start;
  int end;
  char legBuff[1111];
  unsigned char squenceCheck = 0x00;
};

struct octoBlock{
  int octoBID;
  int start;
  int end;
  char octoBuffer[8888];
  unsigned char sequenceChecker = 0x00;
  octoLeg octoLegs[8];
};

struct myFile{
  char fileName[100] = "";
  int fileSize = 0;
  int numOfOctoB;
  int currentOctoB = 0;
  octoBlock blocks[30];
};


int main(int argc, char ** argv) {
	const char* server_name = "localhost";//loopback
	const int server_port = PORT;

	if(argc != 2){
	  printf("Please enter a file name\n");
	  exit(-1);
	}
	
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// creates binary representation of server name
	// and stores it as sin_addr
	//inet_pton: convert IPv4 and IPv6 addresses from text to binary form

	inet_pton(AF_INET, server_name, &server_address.sin_addr);
	
	
	server_address.sin_port = htons(server_port);

	// open socket
	int sock;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("could not create socket\n");
		return 1;
	}
	printf("client socket created\n");
	// data that will be sent to the server
	const char* data_to_send = argv[1];

	// send data
	int len =
	    sendto(sock, data_to_send, strlen(data_to_send), 0,
	           (struct sockaddr*)&server_address, sizeof(server_address));
	printf("message has been sent to server\n");
	// received echoed data back
	char buffer[1111];
	int recv_bytes=recvfrom(sock, buffer, 1111, 0, NULL, NULL);
	printf("received bytes = %d\n",recv_bytes);
	buffer[1111] = '\0';
	printf("recieved: '%s'\n", buffer);
	
	//check if found or not and send ack
	myFile file;
	file.fileSize = atoi(buffer + 11);
	cout<<"filesize: "<<file.fileSize<<endl;
	
	if(strstr(buffer,"ERROR!!!") == NULL){
	  //file is found
	  cout<<"file found"<<endl;
	  char ack[100] = "ACK file size received\n";
	  sendto(sock, ack, 100, 0, (struct sockaddr*)&server_address, sizeof(server_address));

	  strcpy(file.fileName, argv[1]);
	}else{
	  //file is not found
	  cout<<"File was not found on server."<<endl;
	  exit(-1);
	}

	

	
	// close the socket
	close(sock);
	return 0;
}

