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
#include <sys/types.h>

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
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
	  cout<<"error"<<endl;
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
	int contentFillLevel = 0;
	char fileContents[300000];
	// run indefinitely
	bool isMidTransfer = false;
	char sendBuffer [1300];
	bzero(sendBuffer, 1300);
	while (true) {
	  char buffer[500];
	  
	  // read content into buffer from an incoming client
	  int len = recvfrom(sock, buffer, sizeof(buffer), 0,
			     (struct sockaddr *)&client_address,
			     &client_address_len);
	  if(len < 0){
	    continue;
	  }
	  cout<<"contents of buffer: "<<buffer<<endl;
	analysis:
	  if(strstr(buffer, "FILEREQUEST: ") != NULL){
	    int fileSize = getFileSize(buffer+13);
	    FILE * file;
	    file = fopen(buffer+13, "r");
	    if(file){
	      contentFillLevel = fread(fileContents, 1,sizeof(fileContents), file);
	    }
	    cout<<"contens of fileContents: "<<endl<<fileContents<<endl<<endl;
	    bzero(buffer, sizeof(buffer));
	    char sendingBuffer[100];
	    sprintf(sendingBuffer,"FILESIZE: %d", fileSize);
	    sendto(sock, sendingBuffer, strlen(sendingBuffer), 0,
		   (struct sockaddr*)&client_address, sizeof(client_address));
	    cout<<"sent filesize"<<endl;

	  }else if(strstr(buffer,"ACK") != NULL){
	    cout<<"got an ack"<<endl;
	  }else if(strstr(buffer, "START:") != NULL){
	    cout<<"got a start: "<<endl;
	    int start = atoi(buffer+6);
	    int end = atoi(strstr(buffer,"END")+4);
	    cout<<"value of start: "<<start<<endl;
	    cout<<"value of end: "<<end<<endl;
	    char dataBuffer[1112];
	    bzero(dataBuffer,1112);
	    strncpy(dataBuffer,fileContents+start,end-start+1);
	    strcat(dataBuffer,"\0");
	    cout<<"contentsof databuffer: "<<dataBuffer<<endl;
	    //	    strcat(dataBuffer,'\0');
	    int sendtoTest = sendto(sock, dataBuffer, 1112, 0,
				    (struct sockaddr*)&client_address, sizeof(client_address));
	    if(sendtoTest<0){
	      cout<<"an error has occured in send to\n";
	      exit(-1);
	    }
	  }else if(strstr(buffer, "FILE RECIEVED") != NULL){
	    cout<<"got a file has been recieved"<<endl;
	  }else{
	    cout<<"catcher"<<endl;
	  }
	    
	  // inet_ntoa prints user friendly representation of the
	  // ip address
	  /*	  buffer[len] = '\0';
	  printf("received: '%s' from client %s on port %d\n", buffer,
		 inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
		 char returnStr [500] = {'\0'};*/
/*
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
	    }*/
	}
	close(sock);
	return 0;
}
