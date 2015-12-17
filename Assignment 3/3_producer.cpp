#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<errno.h>
#include<string.h>

int main(int argc, char*argv[]){

	if(argc !=3){
		printf("Error\nCorrect format ./a.out <integer> <integer>\n");
		exit(2);
	}

	int shmKey = 12129;
	int shmId = shmget(shmKey, sizeof(int), IPC_CREAT|0666);
	if(shmId==-1){
		perror("err");
		printf("err: %s", strerror(errno));
		exit(1);
	}
	int *serverVar = (int*) shmat( shmId, NULL, 0);
	int oldVal= atoi(argv[1]);
	*serverVar = oldVal;

	int range = atoi(argv[2]);

	for( int i=2; i< range; i+=2 ){
		while( *serverVar==oldVal )
			;
		printf("Received %d from consumer.\n", *serverVar);
		//printf("%d ", *serverVar + oldVal );
		*serverVar += oldVal;
		oldVal = *serverVar;
		printf("Sending %d to consumer.\n", *serverVar);
	}
	shmdt( serverVar  );
	return 0;
}
