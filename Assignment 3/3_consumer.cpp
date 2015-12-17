#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/shm.h>

int main(int argc, char*argv[]){

	if(argc != 3){
		printf("Error\nCorrect format ./a.out <integer> <integer>\n");
		exit(2);
	}

	int shmKey = 12129;
	int shmId = shmget(shmKey, sizeof(int), IPC_CREAT|0666);
	if(shmId==-1){
		printf("Err");
		exit(1);
	}
	int *clientVar = (int*) shmat( shmId, NULL, 0);
	*clientVar = atoi(argv[1]);
	int oldVal= *clientVar;

	int range = atoi(argv[2]);
	//printf("%d ", *clientVar);

	for( int i=2; i< range; i+=2 ){
		while( *clientVar==oldVal )
			;
		printf("Received %d from consumer.\n", *clientVar);
		oldVal += *clientVar;
		*clientVar = oldVal;
		printf("Sending %d to producer.\n", *clientVar);
	}
	shmdt( clientVar );
	return 0;
}
