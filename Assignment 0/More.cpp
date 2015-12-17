#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void displayFile(const char* fileName, int linesLimit){
	int fd = open(fileName, O_RDONLY), linesCount = 0;
	char ch;

	while( read(fd, &ch, 1) > 0 ){

		while(linesCount < linesLimit){
			if(ch=='\n')
				linesCount++;
			if(linesCount == linesLimit){
				//write(1, &ch, 1);
				break;
			}
			write(1, &ch, 1);
			if( read(fd, &ch, 1) <= 0 )
				break;
		}
		while(ch = getchar()){
			if(ch == 'q')
				exit(1);
			if (ch == ' ') {
				linesCount = 0;
				break;
			}
			if( ch == 10 ){
				linesCount--;
				break;
			}
		}
	}
}

void displayFileUsingMore(int argc, const char* argv[]){

	if(argc < 3){
		printf("Parameters missing \nSyntax: %s\n", "./more <#lines> <filename> [file ...] ");
		return;
	}

	for (int i = 2; i < argc; ++i)	{
		if(argc > 3)
			printf("********* File: %s *********\n", argv[i]);
		displayFile(argv[i], atoi(argv[1]));
	}
}

int main(int argc, char const *argv[])
{
	displayFileUsingMore(argc, argv);
	return 0;
}