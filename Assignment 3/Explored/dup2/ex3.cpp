/*
	output redirection with dup2()
	send the output of a command to a file of the user's choice.

	Paul Krzyzanowski
	https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/dup2.html
*/ 


#include <stdlib.h> 
#include <stdio.h> 
#include <fcntl.h> 
#include "unistd.h"


int main(int argc, char const *argv[])
{
	int pid, status, newfd; /* new file descriptor */ 
	char *cmd[] = { (char*)"/bin/ls", (char*)"-al", (char*)"/", 0 }; 

	if (argc != 2) { 
		fprintf(stderr, "usage: %s output_file\n", argv[0]); exit(1); 
	} 

	if ((newfd = open(argv[1], O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) { 
		perror(argv[1]); /* open failed */ 
		exit(1); 
	} 

	printf("writing output of the command %s to \"%s\"\n", cmd[0], argv[1]); 

	/* this new file will become the standard output */ 
	/* standard output is file descriptor 1, so we use dup2 to */ 
	/* to copy the new file descriptor onto file descriptor 1 */ 
	/* dup2 will close the current standard output */ 

	dup2(newfd, 1); 
	
	/* now we run the command. It runs in this process and will have */ 
	/* this process' standard input, output, and error */ 

	execvp(cmd[0], cmd); 
	perror(cmd[0]); 

	/* execvp failed */
	return 0;
}