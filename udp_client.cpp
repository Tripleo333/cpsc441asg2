/*
 *check out
 *https://www.binarytides.com/raw-udp-sockets-c-linux/
 *for help on creating a raw UDP header
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <bitset>

#define PORT 8001
using namespace std;
struct octoLeg{
  int octoBlockID = -1;
  int octoLegID = -1;
  int start;
  int end;
  char legBuff[1111];
  unsigned char sequenceCheck = 0x00;
};

struct octoBlock{
  int octoBID = -1;
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

	//creating octoblocks
	int tempFileSize = file.fileSize;
	if(file.fileSize <= 8888){
	  //create single octoBlock and checking if it needs a tiny block
	  double tempLegSize = (double)tempFileSize / (double) 8;
	  tempLegSize = floor(tempLegSize);
	  int OB1Size = 8*tempLegSize;
	  if(OB1Size != file.fileSize){
	    //tinyOctoBlock creation
	    file.blocks[1].octoBID = 1;
	    file.blocks[1].start = OB1Size;
	    //it is supposed to be - 1 FUCKING KEEP IT THIS WAY AND DON'T THINK ABOUT IT AGAIN
	    file.blocks[1].end = file.fileSize - 1;
	    file.numOfOctoB++;
	  }
	  file.blocks[0].octoBID = 0;
	  file.blocks[0].start = 0;
	  file.blocks[0].end = OB1Size - 1;
	  file.numOfOctoB++;
	  
	  
	}else{
	  //create multiple octoBlocks
	  cout<<"in the else that creates multiple octoblocks"<<endl;
	}
	//creating octolegs in each octoblock
	for(int i = 0; i < file.numOfOctoB; i++){
	  //will have to change range for more than one octoblock legs
	  int legRange = (file.blocks[i].end - file.blocks[i].start+1)/8;
	  cout<<"value of legRange: "<<legRange<<endl;
	  for(int j = 0; j < 8; j++){
	    file.blocks[i].octoLegs[j].octoBlockID = file.blocks[i].octoBID;
	    file.blocks[i].octoLegs[j].octoLegID = j;
	    file.blocks[i].octoLegs[j].start = (legRange * j);
	    file.blocks[i].octoLegs[j].end = file.blocks[i].octoLegs[j].start + legRange - 1;
	    file.blocks[i].octoLegs[j].octoBlockID = file.blocks[i].octoBID;
	    file.blocks[i].octoLegs[j].sequenceCheck = 0x01;
	    for (int m = 0; m < j; m++){
	      file.blocks[i].octoLegs[j].sequenceCheck =
		file.blocks[i].octoLegs[j].sequenceCheck << 1;
	    }
	  }
	}

	for(int i = 0; i < file.numOfOctoB; i++){
	  cout<<"BID of each octoblock "<<file.blocks[i].octoBID<<endl;
	  cout<<"start for block "<<file.blocks[i].start<<endl;
	  cout<<"end of block "<<file.blocks[i].end<<endl;
	  cout<<endl;
	  for(int j = 0; j < 8; j++){
	    cout<<"octoleg BID: "<<file.blocks[i].octoLegs[j].octoBlockID
		<<". octoleg ID: "<<file.blocks[i].octoLegs[j].octoLegID<<endl;
	    cout<<"leg start: "<<file.blocks[i].octoLegs[j].start<<endl;
	    cout<<"leg end: "<<file.blocks[i].octoLegs[j].end<<endl;
	    cout<<"leg's sequenceChec: "<<bitset<8>(file.blocks[i].octoLegs[j].sequenceCheck)<<endl;
	  }
	  cout<<"-----------------------------------------------------------------"
	    <<endl<<endl<<endl;
	}
	

	
	// close the socket
	close(sock);
	return 0;
}

