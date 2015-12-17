#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct Lines
{
	unsigned int firstByteOfLine[];
	int numberOfLines;
}Line;

int main(int argc, char const *argv[])
{
	int fd = open( argv[1] , O_RDONLY), byteOffset = 0, linesPerFile = 5;
	char buff[20];

	int linesCount = 0, count;

	Lines lines;
	int lineCount = 0;
	while((count = read(fd, buff, 512)) >0)
		for (int i = 0; i < count; ++i){
			if(buff[i]=='\n')
				lineCount++;
		}
	lines.numberOfLines = lineCount;

	int bytecount = 0, i = 0;
	lineCount = 0;

	int blockSize;
	while((blockSize=read(fd, buff, 512))>0){
		for(int i=0; i<blockSize; i++){
			bytecount++;
			if(buff[i]=='\n'){
				if(lineCount==0)
					lines.firstByteOfLine[lineCount] = 0;
				else
					lines.firstByteOfLine[lineCount] = bytecount+1;
				lineCount++;
			}
		}
	}

	char *buff2;

	int numberOfBytesInCurrentLine = lines.firstByteOfLine[i+1] - lines.firstByteOfLine[i];
	buff2 = (char*) malloc( sizeof(char)* numberOfBytesInCurrentLine);
	for(int j = 0; j<=lineCount; j++){
		for (int i = j*4+4-1; i >=j*4; --i) {
			if( i>lineCount)
				i = lineCount;
			lseek(fd, lines.firstByteOfLine[i], SEEK_SET );

			int len;
			numberOfBytesInCurrentLine = lines.firstByteOfLine[i+1] - lines.firstByteOfLine[i];
			buff2 = (char*) calloc( numberOfBytesInCurrentLine, sizeof(char) );

			if( (len=read(fd, buff2, numberOfBytesInCurrentLine ))>0 )
				write(1, buff2, len);
		}
	}


	return 0;
}