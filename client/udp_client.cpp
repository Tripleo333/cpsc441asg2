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
#include <sys/types.h>
#include <fstream>
#define PORT 8001
#define retryLimit 5

using namespace std;
struct octoLeg{
  int octoBlockID = -1;
  int octoLegID = -1;
  int start;
  int end;
  char legBuff[1111];
  int retry = 0;
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
  int usedSize = 0;
  int currentOctoB = 0;
  octoBlock blocks[30];
};
char fileContents[300000];
int retryCounter = 0;
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

  //set socket recv option timer
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  
  
  printf("client socket created\n");
  // data that will be sent to the server
  char data_to_send [250];
  bzero(data_to_send, 250);
  strcat(data_to_send, "FILEREQUEST: ");
  strcat(data_to_send, argv[1]);
  cout<<"contents of data_to_send: "<<data_to_send<<endl;
  // send data
 retryCon:
  int len =
    sendto(sock, data_to_send, strlen(data_to_send), 0,
	   (struct sockaddr*)&server_address, sizeof(server_address));
  printf("message has been sent to server\n");
  // received echoed data back

  char buffer[1111];
  bzero(buffer, 1111);
  int recv_bytes=recvfrom(sock, buffer, 1111, 0, NULL, NULL);
  if (recv_bytes < 0){
    //timed out and retrying
    cout<<"connection time out... retrying..."<<endl;
    if(retryCounter > 5){
      cout<<"connection failed. Please try again later"<<endl;
      exit(-1);
    }else{
      retryCounter++;
      goto retryCon;
    }
  }else{
    retryCounter = 0;
  }
  printf("received bytes = %d\n",recv_bytes);
  buffer[1111] = '\0';
  printf("recieved: '%s'\n", buffer);
  
  //check if found or not and send ack
  myFile file;
  file.fileSize = atoi(buffer + 10);
  cout<<"filesize: "<<file.fileSize<<endl;
  
  if(strstr(buffer,"-1") == NULL){
    //file is found
    cout<<"file found"<<endl;
    char ack[100];
    bzero(ack, 100);
    strcat(ack,"ACK");
    sendto(sock, ack, 100, 0, (struct sockaddr*)&server_address, sizeof(server_address));    
    strcpy(file.fileName, argv[1]);
  }else{
    //file is not found
    cout<<"File was not found on server."<<endl;
    exit(-1);
  }
  
  //creating octoblocks
  int tempFileSize = file.fileSize;

  //create multiple full octoBlocks
  cout<<endl;
  double numOfBlocks = (double)file.fileSize/(double)8888;
  numOfBlocks = floor(numOfBlocks);
  for(int i = 0; i < numOfBlocks; i++){
    file.numOfOctoB++;
    file.blocks[i].octoBID = i;
    file.blocks[i].start = 8888 * i;
    //    file.blocks[i].end = 8888 * i + 8888;
    file.blocks[i].end = file.blocks[i].start + 8887;
    file.usedSize += 8888;
  }
  //creating partial octoblock
  if(file.fileSize>file.usedSize){
    //a single partial block is needed
    int thisBID = file.numOfOctoB;
    file.blocks[thisBID].octoBID = thisBID;
    file.numOfOctoB++;
    if(file.numOfOctoB <=1){
      file.blocks[thisBID].start = 0;
      file.blocks[thisBID].end = floor((double)file.fileSize/8.0) * 8 - 1;
    }else{
      //if there are other octolblocks before
      file.blocks[thisBID].start = file.usedSize;
      file.blocks[thisBID].end = floor((double)file.fileSize/8.0) * 8 - 1;
      //^last divisible by 8 size of file
    }
    //creating tiny block
    if(file.blocks[thisBID].end+1<file.fileSize){
      file.numOfOctoB++;
      thisBID++;
      file.blocks[thisBID].octoBID = thisBID;
      file.blocks[thisBID].start = file.blocks[thisBID - 1].end + 1;
      //      file.blocks[thisBID].start = file.blocks[thisBID - 1].end;
      file.blocks[thisBID].end = file.fileSize - 1;
    }
  }
  cout<<endl<<file.numOfOctoB<<endl;
  
  //creating octolegs in each octoblock
  for(int i = 0; i < file.numOfOctoB; i++){
    //will have to change range for more than one octoblock legs
    int legRange = (file.blocks[i].end - file.blocks[i].start)/8 + 1;
    //double legRange = ((double)file.blocks[i].end + 1.0 - (double)file.blocks[i].start )/8.0;
    //    cout<<"value of legRange: "<<legRange<<endl;
    if(legRange > 7){
      for(int j = 0; j < 8; j++){
	file.blocks[i].octoLegs[j].octoBlockID = file.blocks[i].octoBID;
	file.blocks[i].octoLegs[j].octoLegID = j;
	file.blocks[i].octoLegs[j].start = (legRange * j) + file.blocks[i].start ;
	file.blocks[i].octoLegs[j].end = file.blocks[i].octoLegs[j].start + legRange - 1;
	//file.blocks[i].octoLegs[j].octoBlockID = file.blocks[i].octoBID;
	file.blocks[i].octoLegs[j].sequenceCheck = 0x01;
	for (int m = 0; m < j; m++){
	  file.blocks[i].octoLegs[j].sequenceCheck =
	    file.blocks[i].octoLegs[j].sequenceCheck << 1;
	}
      }
    }else{
      legRange = (file.blocks[i].end + 1 + file.blocks[i].start) % 8;
      cout<<endl<<"leg range of tiny octoblock: "<<legRange<<endl;
      int j = 0;
      for(j = 0; j < legRange; j++){
	//((file.blocks[i].end + 1 + file.blocks[i].start) % 8); j++){
	cout<<endl;
	file.blocks[i].octoLegs[j].octoBlockID = file.blocks[i].octoBID;
	file.blocks[i].octoLegs[j].octoLegID = j;
	file.blocks[i].octoLegs[j].start =  j + file.blocks[i].start;
	file.blocks[i].octoLegs[j].end = file.blocks[i].octoLegs[j].start;
	file.blocks[i].octoLegs[j].sequenceCheck = 0x01;
	for(int m = 0; m < j; m++){
	  file.blocks[i].octoLegs[j].sequenceCheck =
	    file.blocks[i].octoLegs[j].sequenceCheck << 1;
	}
	//	cout<<"sequenceCheck of tiny block's legs with content: "<<file.blocks[i].octoLegs[j].sequenceCheck<<endl;
	//cout<<"start of tiny block's legs with content: "<<file.blocks[i].octoLegs[j].start<<endl;
	//cout<<"end of tiny block's legs with content: "<<file.blocks[i].octoLegs[j].end<<endl;
	//cout<<endl;
      }
      //      cout<<"padding tiny block's legs\n"<<endl;
      for(j; j < 8; j++){
	file.blocks[i].octoLegs[j].octoBlockID = file.blocks[i].octoBID;
	file.blocks[i].octoLegs[j].octoLegID = j;
	file.blocks[i].octoLegs[j].start = -1;
	file.blocks[i].octoLegs[j].end = -1;
	for(int m = 0; m < j; m++){
	  file.blocks[i].octoLegs[j].sequenceCheck =
	    file.blocks[i].octoLegs[j].sequenceCheck << 1;
	}
	//cout<<"sequenceCheck of tiny block's legs with content: "<<file.blocks[i].octoLegs[j].sequenceCheck<<endl;
	//cout<<"start of tiny block's legs with content: "<<file.blocks[i].octoLegs[j].start<<endl;
	//cout<<"end of tiny block's legs with content: "<<file.blocks[i].octoLegs[j].end<<endl;
	//cout<<endl;
	
	
      }
    }
  }
  
  char sendBuffer [150];
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
      bzero(sendBuffer, 150);
      sprintf(sendBuffer,"START: %d\nEND: %d\noctoLegID: %c", file.blocks[i].octoLegs[j].start,
	      file.blocks[i].octoLegs[j].end, file.blocks[i].octoLegs[j].octoLegID);
      cout<<"contents of sendBuffer: \n"<<sendBuffer<<endl;
      //      sleep(5);
      sendto(sock, sendBuffer, 150, 0, (struct sockaddr*)&server_address, sizeof(server_address));
      char dataBuffer[1200];
      bzero (dataBuffer, 1200);
    recvData:
      int len = recvfrom(sock, dataBuffer, 1200, 0,NULL,NULL);
      if(len < 0){
	if(retryCounter < 5){
	  retryCounter++;
	  cout<<"recvfrom didn't recvieve anything retrying.\n";
	  cout<<"retry number: "<<retryCounter<<endl;
	  goto recvData;
	}else{
	  cout<<"Transfer failed. Please try again later."<<endl;
	  exit(-1);
	}
      }
      retryCounter = 0;
      cout<<"content of dataBuffer"<<dataBuffer<<endl;
      bzero(file.blocks[i].octoLegs[j].legBuff, 1111);
      strncpy(file.blocks[i].octoLegs[j].legBuff,dataBuffer, strlen(dataBuffer));
      strncat(file.blocks[i].octoLegs[j].legBuff,"\0",1);
    }
    cout<<"-----------------------------------------------------------------"
	<<endl<<endl<<endl;
  }
  cout<<"number of blocks: "<<file.numOfOctoB<<endl;
  for(int i = 0; i < file.numOfOctoB;i++){
    for(int j = 0; j < 8; j++){
      if(file.blocks[i].octoLegs[j].start != -1){
	cout<<"before writing"<<endl;
	strcat(fileContents,file.blocks[i].octoLegs[j].legBuff);
	cout<<"after writing"<<endl;
      }
    }
  }
  ofstream myfile (argv[1]);
  myfile <<fileContents;
  myfile.close();
  char closeCon[150] = "FILE RECIEVED";
  int retries = 0;
 resendClose:
  sendto(sock,closeCon, 150, 0, (struct sockaddr*)&server_address, sizeof(server_address));
  char closeConFromServer[200];
  bzero(closeConFromServer, 200);
  int recvLen = recvfrom(sock, closeConFromServer, 200, 0, NULL,NULL);
  if (recvLen < 0){
    if(retries < 5){
      cout<<"closing handshake failed... retrying"<<endl;
      goto resendClose;
    }else{
      cout<<"closing connection failed multiple times."<<endl
	  <<"program terminating"<<endl;
      exit(-1);
    }
    
  }
  // close the socket
  close(sock);
  return 0;
}

