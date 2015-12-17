#include "stdlib.h"
#include "stdio.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"

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


char** getSingleCommand(char *singleCmdInPipe) {
	char **singleCmd = (char**)malloc(sizeof(char*)*32);
	char *rest = singleCmdInPipe, *token;
	int i = 0;
	//printf("%s\n", singleCmdInPipe);
	while( token = strtok_r(rest, " ", &rest) ){
		/* Breaks the command into tokens. e.g., "ls -l"  -> ls, -l 	*/
		singleCmd[i] = (char*)malloc( sizeof(char) * 10 );
		strcpy(singleCmd[i++], token);;
	}

	return singleCmd;
}


int main(int argc, char const *argv[])
{
	int errno;
	int cmdCount, FROM_CHILD_PROCESS = 0;
	char *cmdsInPipe[32];

	cmdCount = parseCmdsFromPipe(cmdsInPipe, argv, argc);

	while(cmdCount){

		int fd[2];
		if (pipe(fd) != 0){
			printf("pipe creation failed\n");
			exit(1);
		}

		pid_t pid = fork();

		if(pid < 0){
			write(1, "Fork error", 10);
			exit(1);
		}

		if(pid==0){
			/*	child process code		*/
			FROM_CHILD_PROCESS = 1;
			
			if(cmdCount>1){
				close(fd[1]);
				close(0);
				dup2(fd[0], 0);
			}
			else if(cmdCount == 0)
				exit(0);

			char **singleCmd = getSingleCommand(cmdsInPipe[1]);

			if( execvp( singleCmd[0] , singleCmd)==-1 ) {
				//perror(singleCmd[0]);
				printf("%s: command not found\n", singleCmd[0]);
				exit(1);
			}
			cmdCount--;
		}
		else{
			/*	parent process code   */
			if( cmdCount > 1 ){
				close(fd[0]);
				dup2(fd[1], 1);
			}

			char **singleCmd = getSingleCommand(cmdsInPipe[0]);

			if( execvp( singleCmd[0] , singleCmd)==-1 ) {
				//perror(singleCmd[0]);
				close(fd[1]);
				open(1);
				printf("%s: command not found\n", singleCmd[0]);
				exit(1);
			}
			
			if( cmdCount > 0 ){
				close(fd[1]);
				close(1);
			}
			cmdCount--;

			wait();
		}
	}

	return 0;
}

