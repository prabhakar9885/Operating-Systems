#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/*
 *		Function responsible for rendering a single file
 */
void displayFile(const char* fileName, int linesLimit){
	int fd = open(fileName, O_RDONLY), linesCount = 0, FLAG = 0, IS_FIRST_ITERATION=1;
	char ch;
	ssize_t readBytesCount = 0;

	while (1) {
		while( (readBytesCount = read(fd, &ch, 1)) >0 ){
			if(readBytesCount==0){
				return;
			}
			if (ch=='\n'){
				linesCount++;
			}
			if ( (IS_FIRST_ITERATION || FLAG) && ch=='\n' && linesCount == linesLimit){
				if(IS_FIRST_ITERATION){
					IS_FIRST_ITERATION = 0;
				}
				else
					printf("\n");
			}
			if ( linesCount == linesLimit )
				break;
			write(1, &ch, 1);
		}

		if(readBytesCount == 0)
			return;
		scanf("%c", &ch);

		if(ch=='q')		//	If 'q'key is pressed then, stop the excution of the command.
			exit(1);
		if(ch==10){		//	If 'return' key is pressed then, display the next line from the file
			FLAG = 0;
			linesCount -= 1;
		}
		if(ch==' '){	//	If space-bar is pressed then, display the next "linesLimit" number of lines from the file
			FLAG = 1;
			linesCount = 1;
		}
	}
}


/*
 *		Function responsible for getting each of the g iven files rendered
 */
void displayFileUsingMore(int argc, const char* argv[]){

	printf("%c\n", *argv[1]);

	for (int i = 2; i < argc; ++i)	{
		if(argc > 3)
			printf("\n****************************\n********* File: %s *********\n", argv[i]);
		displayFile(argv[i], atoi(argv[1]));
	}
}

int main(int argc, char const *argv[])
{
	if(argc < 3 || *argv[1]<'0' || *argv[1]>'9'){
		printf("Parameters missing \nSyntax: %s\n", "./a.out <#lines> <filename> [file ...] ");
		return 0;
	}
	displayFileUsingMore(argc, argv);
	return 0;
}