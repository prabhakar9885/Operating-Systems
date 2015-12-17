#include <stdlib.h>
#include <sys/types.h>
#include "strings.h"
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "unistd.h"

#define RCVBUFSIZE 100

int main(int argc, char const *argv[])
{
	int sock_fd;

	if ( (sock_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("Socket creation failed\n");
		exit(1);
	}
	printf("Socket Created...\n");

	struct sockaddr_in serverAddr;
	struct hostent *server = gethostbyname(argv[1]);

	bzero((char *) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = inet_addr(server->h_addr);
	//serverAddr.sin_addr = *((struct in_addr *)server->h_addr);
	serverAddr.sin_port = htons(atoi(argv[2]));
	bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);

	if(server==NULL)
	{
		printf("Cant find the host\n");
		exit(1);
	}

	if (connect(sock_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		printf("connect() failed\n");
		exit(2);
	}
	printf("Connected...\n");

	char buffer[512];
	bzero(buffer, 512);
	int n;

	printf("Type string to send: ");
	fflush(stdout);
	n = read(0,buffer, 512);
	
	while(1){

		if( send(sock_fd, buffer, n, 0) < n )
			printf("send() error\n");
		else {
			size_t n1;
			int sum;
			if ( (n1=recv(sock_fd, &sum, sizeof(int), 0)) == sizeof(int) ){
				buffer[n1]=0;
				printf("Sum recieved: %d\n", sum);	
				bzero(buffer, 512);
				printf("Type string to send: ");	fflush(stdout);
				n = read(0,buffer, 512);
			}
			else{
				printf("recv Error\n");				
			}
		}
	}
	close(sock_fd);
}