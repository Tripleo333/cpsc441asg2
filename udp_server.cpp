#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <string>
#include <unistd.h>
#include <math.h>

#define PORT 8001

using namespace std;

struct octoLeg{
  int octoBlockID;
  int octoLegID;
  int start;
  int end;
  char legBuff[1111];
  unsigned char sequenceCheck = 0x00;
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

int getFileSize(const char *name){
  ifstream ifile(name);
  int ret = -1;
  if((bool)ifile){
    ifstream in(name, std::ifstream::ate | std::ifstream::binary);
    ret = (int)in.tellg();
  }
  return ret;
}

int main(int argc, char *argv[]) {
	// port to start the server on
	int SERVER_PORT = PORT;

	// socket address used for the server
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(SERVER_PORT);

	// htons: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	// create a UDP socket, creation returns -1 on failure
	int sock;
	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("could not create socket\n");
		return 1;
	}
	printf("server socket created\n");
	// bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((bind(sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) {
		printf("could not bind socket\n");
		return 1;
	}
	printf("binding was successful\n");
	// socket address used to store client address
	struct sockaddr_in client_address;
	socklen_t client_address_len = sizeof(client_address);
        char client_name[100];
	// run indefinitely
	while (true) {
	  char buffer[500];
	  
	  // read content into buffer from an incoming client
	  int len = recvfrom(sock, buffer, sizeof(buffer), 0,
			     (struct sockaddr *)&client_address,
			     &client_address_len);
		
	  // inet_ntoa prints user friendly representation of the
	  // ip address
	  buffer[len] = '\0';
	  printf("received: '%s' from client %s on port %d\n", buffer,
		 inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
	  char returnStr [500] = {'\0'};
	  if(strstr(buffer, "ACK") != NULL){
	    cout<<buffer<<endl;
	  }else{
	    int fileSize = getFileSize(buffer);
	    if( fileSize != -1){
	      cout<<"existing file size: "<<fileSize<<endl;
	      sprintf(buffer, "File Size: %d\n", fileSize);
	    }else{
	      cout<<"File Doesn't exist\n"<<endl;
	      sprintf(buffer,"ERROR!!! File Doesn't exist.\n");
	    }
	    
	    int sent_len = sendto(sock, buffer, 150, 0, (struct sockaddr *)&client_address,
				  client_address_len);
	    printf("server sent back message:%d\n",sent_len);
	  }
	}
	close(sock);
	return 0;
}
