#include <stdlib.h>
#include "string.h"
#include <sys/types.h>
#include "strings.h"
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "unistd.h"

#define RCVBUFSIZE 100



int getAsciiSum(char *echoBuffer){

	int len = strlen(echoBuffer), sum = 0;

	for (int i = 0; i < len-1; ++i){
		sum += echoBuffer[i];
	}

	return sum;
}


int main(int argc, char const *argv[])
{
	int sockfd;

	if ( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("Socket creation failed\n");
		exit(1);
	}
	printf("Socket Created...\n");

	struct sockaddr_in saddr;
	
	bzero((char *) &saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(atoi(argv[1]));

	if (bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
	{
		printf("Socket bind failed\n");
		exit(2);
	}
	printf("Socket binded...\n");

	while(1)
	{
		if (listen(sockfd, 1)<0)
		{
			printf("Listen failed\n");
			exit(3);
		}
		printf("Listening...\n");

		struct sockaddr_in caddr;
		int clientSock;
		socklen_t clen = sizeof(caddr);

		if ((clientSock=accept(sockfd, (struct sockaddr*)&caddr, &clen )) < 0)
		{
			printf("accept() failed.\n");
			exit(4);
		}
		printf("Accepted a connection...\n");

		int recvMsgSize=1;
		char *echoBuffer = (char*)malloc(RCVBUFSIZE*sizeof(char));

		while (recvMsgSize > 0) { /* zero indicates end of transmission */
			if ((recvMsgSize = recv(clientSock, echoBuffer, RCVBUFSIZE, 0)) < 0){
				printf("recv() failed");
				exit(7);
			}
			else{
				echoBuffer[recvMsgSize] = 0;
				printf("String received: %s", echoBuffer);
			}
			
			int sum = getAsciiSum(echoBuffer);
			if ( send(clientSock, &sum, sizeof(int), 0) != sizeof(int) ) {
				printf("send() failed");
				exit(6);
			}
			else{
				printf("Sum sent: %d\n", sum);
				fflush(stdout);
			}
		}
	}
	close(sockfd);
}