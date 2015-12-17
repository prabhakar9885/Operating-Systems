#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct Line 
{
	int startingOffset;
	int numberOfBytes;
}Line;

int main(int argc, char const *argv[])
{
	int fd = open( argv[1] , O_RDONLY), byteOffset = 0, linesPerFile = 5;
	char buff[20];

	int linesCount = 0;

	Line* lines = (Line*)malloc(sizeof(Line)*125);
	int k = 0;
	int lineCount = 0;
	int bytecount = 0, i = 0;

	while(read(fd, buff, 20)>0){
		for(int i=0; i<20; i++){
			bytecount++;
			if(buff[i]=='\n'){
				if(lineCount==0){
					lines[lineCount].startingOffset = -1;
					lines[lineCount].numberOfBytes = bytecount;
				}
				else{
					lines[lineCount].startingOffset = lines[lineCount-1].startingOffset + lines[lineCount-1].numberOfBytes;
					lines[lineCount].numberOfBytes = bytecount;
				}
				bytecount = 0;
				lineCount++;
			}
		}
	}

	char *buff2;


	buff2 = (char*) malloc( sizeof(char)* lines[0].numberOfBytes );
	for(int j = 0; j<=lineCount; j++){
		for (int i = j*4+4-1; i >=j*4; --i) {
			if( i>lineCount) 
				i = lineCount;
			lseek(fd, lines[i].startingOffset+1, SEEK_SET );
			int len;

			buff2 = (char*) calloc( lines[i].numberOfBytes, sizeof(char) );
			if( (len=read(fd, buff2, lines[i].numberOfBytes ))>0 )
				write(1, buff2, len);
			//delete(buff2);
		}
	}


	return 0;
}
