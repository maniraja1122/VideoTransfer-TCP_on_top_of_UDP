#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
struct Packet;
struct Connection;
#define SIZE 508 // Packet Size
#define n 10 // Window Size
#define MAX 4294967295
// General Functions
char* convertToChar(unsigned int value){
	char* buffer=(char*)malloc(4 * sizeof(char));;
	buffer[0] = (value >> 24) & 0xFF;
	buffer[1] = (value >> 16) & 0xFF;
	buffer[2] = (value >> 8) & 0xFF;
	buffer[3] = value & 0xFF;
    return buffer;
}	
unsigned int convertToInt(char* buffer){
	unsigned int ans=((buffer[0] & 0x000000FF) << 24)+((buffer[1] & 0x000000FF)<< 16)+((buffer[2] & 0x000000FF) << 8)+(buffer[3] & 0x000000FF);
	return ans;
}
// Struct's
struct Packet{
	unsigned int seq;
	unsigned int ack;
	bool isAck;
	char data[SIZE];	
};
struct Connection{
	int sockfd;
	struct sockaddr * address;
	struct Packet window[n];
	int pfirst;//Pointer number of first window
	int plast;
	int seqstart;
};
//Struct Functions
void SendPacket(struct Packet p,struct Connection c){
	int cd =sendto(c.sockfd, p.data, SIZE,
		MSG_CONFIRM, c.address,
			sizeof(*c.address));

}
struct Packet RecievePacket(struct Connection c){
	int num,len;
	char buffer[SIZE];
	struct Packet p;
	num=recvfrom(c.sockfd, (char *)buffer, SIZE,
				MSG_WAITALL, c.address,
				&len);
	if(num>0){
		char ack[4];
		char seq[4];
		char data[SIZE];
		for(int i=0;i<4;i++){
			ack[i]=buffer[i];
			seq[i]=buffer[i+4];
		}
		p.seq=convertToInt(seq);
		p.ack=convertToInt(ack);
		for(int i=0;i<SIZE;i++){
			p.data[i]=buffer[i];
		}
	}			
	return p;
}
struct Packet createPacket(unsigned int seq,unsigned int ack,char datapacket[SIZE-8]){
	struct Packet p1;
	p1.seq=seq;
	p1.ack=ack;
	char *ackarr=convertToChar(ack);
	char *seqarr=convertToChar(seq);
	char final[SIZE];
	for(int i=0;i<4;i++){
		final[i]=ackarr[i];
		final[i+4]=seqarr[i];
	}
	for(int i=0;i<SIZE-8;i++){
		final[i+8]=datapacket[i];
	}
	for(int i=0;i<SIZE;i++){
		p1.data[i]=final[i];
	}
	return p1;
}
void WindowSend(struct Connection c){
	//File 
	FILE *filePointer;
    filePointer = fopen("input.mp4", "rb");
	c.pfirst=0;
	c.plast=0;
	c.seqstart=0;
	while(1){
		if(c.plast-c.pfirst<n){
			char buffer[SIZE-8];
        	int bytesread=fread(buffer, 1,SIZE-8, filePointer);
			if(bytesread==0){
				printf("File Sent\n");
				break;
			}
			struct Packet p=createPacket(c.seqstart+c.plast,0,buffer);
			c.window[c.plast%n]=p;
			//Send Packet
			SendPacket(p,c);
			c.plast++;
		}
		else{
			struct Packet p=RecievePacket(c);
			printf("%d ack recieved\n",p.ack);
			if(p.ack-2==c.seqstart+c.pfirst){
				c.pfirst++;
			}
		}
	}
	//Closing File
	 if (filePointer != NULL)
    {
        fclose(filePointer);
    }
}
void WindowRecieve(struct Connection c){
	FILE *filePointer;
    filePointer = fopen("output.mp4", "wb");
	c.pfirst=0;
	c.plast=0;
	c.seqstart=0;
	while(1){
		char buffer[SIZE-8];
		if(c.plast-c.pfirst<n){
			struct Packet p=RecievePacket(c);
			if(p.seq>=c.pfirst && p.seq<=c.plast){
			printf("%d seq recieved\n",p.seq);
			c.window[p.seq%n]=p;
			//Send Ack
			struct Packet p1=createPacket(0,p.seq+1,buffer);
			printf("%d ack sent\n",p1.ack);
			SendPacket(p1,c);
			if(p.seq==c.pfirst){
				for(int k=0;k<SIZE-8;k++){
					buffer[k]=p.data[k+8];
				}
				fwrite(buffer,1,SIZE-8, filePointer);
				c.pfirst++;
				c.plast++;
			}
			}
		}
	}
	fclose(filePointer);
}
struct Connection InitializeClient(struct Connection c1){
	return c1;
}
struct Connection InitializeServer(struct Connection c1){
	return c1;
}
struct Connection createConnection(int sockfd,struct sockaddr *add){
	struct Connection c1;
	c1.address=add;
	c1.sockfd=sockfd;
	return c1;
}
