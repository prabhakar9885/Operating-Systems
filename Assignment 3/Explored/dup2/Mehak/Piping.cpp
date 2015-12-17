#include "stdlib.h"
#include "stdio.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"
#include<errno.h>
extern int errno;

/**
 *	
 */
int parseCmdsFromPipe(char *cmds[], char const *argv[], int argc)
{
	int len=0, status;
	for (int i = 1; i < argc; ++i)
		len += strlen(argv[i]);

	char *cmd = (char*)malloc( sizeof(char) * len );
	for (int i = 1; i < argc; ++i){
		strcat(cmd, argv[i]);
		strcat(cmd, (char*)" ");
	}

	// char *cmds[32];
	int lenOfCmdBeforePipe = 0, indexOfCmdBeforePipe = 0, cmdCount = 0;
	for (int i = 0; i < len; ++i) {
		lenOfCmdBeforePipe = 0;
		for ( indexOfCmdBeforePipe = i; 
				indexOfCmdBeforePipe<len && cmd[indexOfCmdBeforePipe] != '|' ;
				 ++indexOfCmdBeforePipe) 
			lenOfCmdBeforePipe++;
				
		cmds[cmdCount] = (char*) malloc( sizeof(char) * (lenOfCmdBeforePipe+1) );
		bzero(cmds[cmdCount], sizeof(char) * (lenOfCmdBeforePipe+1) );
		for (int j = i; j < i+lenOfCmdBeforePipe; ++j)
			cmds[cmdCount][j-i] = cmd[j];

		cmds[lenOfCmdBeforePipe] = 0;

		//printf("%s\n", cmds[cmdCount]);

		//printf("%d\n", lenOfCmdBeforePipe);

		i += lenOfCmdBeforePipe;
		cmdCount++;
	}
	return cmdCount;
}



int main(int argc, char const *argv[])
{
	int cmdCount;
	char *cmdsInPipe[32];

	cmdCount = parseCmdsFromPipe(cmdsInPipe, argv, argc);


	int fd[2];
	pid_t pid = fork();

	if (pipe(fd) != 0){
		printf("pipe creation failed\n");
		exit(1);
	}

	if(pid < 0){
		write(1, "Fork error", 10);
		exit(1);
	}

	if(pid==0){
		/*	child process code		*/
		usleep(1000);
		close(fd[1]);
		//close(0);

		dup2(fd[0], 0);
		char ch;
		fprintf(stderr, "c1 error %d: %s\n", errno, strerror(errno));

		while( read(fd[0], &ch, 1 ) >0 )
			write(1, &ch, 1);
		//exit(0);
	}
	else{
		/*	parent process code   */
		close(fd[0]);
		//close(1);
		dup2(fd[1], 1);
		char *singleCmd[32];
		char *rest =cmdsInPipe[0], *token;
		fprintf(stderr, "p1 error %d: %s\n", errno, strerror(errno));
		int i = 0;
		printf("%s\n", cmdsInPipe[0]);
		while( token = strtok_r(rest, " ", &rest) ){
			/* Breaks the command into tokens. e.g., "ls -l"  -> ls, -l 	*/
			singleCmd[i] = (char*)malloc( sizeof(char) * 10 );
			strcpy(singleCmd[i++], token);;
		}
		fprintf(stderr, "p2 error %d: %s\n", errno, strerror(errno));

		fprintf(stderr, "singleCmd %s\n", singleCmd[0]);
		int j;
		for(j=0;j<i;j++){
			fprintf(stderr, "%d: %s\n", j, singleCmd[j]);

		}
		int r=execvp( singleCmd[0] , &singleCmd[0]);
		fprintf(stderr,"ret %d\n", r);

		close(fd[1]);
		close(1);

		//wait();
	}


	// for (int i = 0; i < cmdCount; ++i)
	// {
	// 	char *singleCmd[32];
	// 	printf("%s\n", cmdsInPipe[i]);
	// }
	
	return 0;
}
