// Client side implementation of UDP client-server model
#include <string.h>
#include "tcpextra.c"	
#define PORT 8080
// Driver code
int main() {
	int sockfd;
	char *hello = convertToChar(1121);
	struct sockaddr_in	 servaddr;
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr("10.7.60.192");
	
	struct Connection c=createConnection(sockfd,(struct sockaddr *) &servaddr);	
	WindowSend(c);
	return 0;
}
