#include "iostream"
#include <dirent.h> 
#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//#define DEBUG

/*The data-structure "Line" stores the starting-offset of each line and the number of bytes in that line*/
typedef struct Line
{
	int startingOffset;
	int numberOfBytes;
}Line;


int lineCount;		/*Total number of lines present in a file*/
int linesPerFile;	/*Number of lines that should be present in each splitted file*/


/*	For each file present in the files under the path "source", 
 *	the "createSplittedFiles" function creates splitted-files("linesPerFile" number of lines per splitted-file), 
 *	in the "desti" path.*/
void createSplittedFiles(const char* source, char* desti, Line* lines, char* fileName){

	chdir(desti);

	char* targetFileName = (char*) malloc( 512 );
	char* sourceFileName = (char*) malloc( 512 );

	#ifdef DEBUG
	std::cout<< "\nStart---";
	std::cout<< source << "\n";
	std::cout<< desti << "\n";
	std::cout<< fileName << "\nEnd-----\n";
	#endif

	strcpy(sourceFileName, source);
	strcpy(targetFileName, desti);
	if(targetFileName[ strlen(targetFileName)-1 ] != '/')
		targetFileName = strcat(targetFileName, "/");

	targetFileName = strcat(targetFileName, fileName);
	targetFileName = strcat(targetFileName, "_");

	if(sourceFileName[ strlen(sourceFileName)-1 ] != '/')
		sourceFileName = strcat(sourceFileName, "/");
	sourceFileName = strcat(sourceFileName, fileName);

	int fdSource = open( sourceFileName , O_RDONLY);
	int EOF_REACHED_IN_SOURCE = 0;

	for(int j = 0; j<lineCount; j++){


		char fileIndexAsString[4];
		sprintf(fileIndexAsString, "%d", j);

		char* targetFileNameWithIndex = (char*)malloc(256);
		strcpy(targetFileNameWithIndex, targetFileName);
		strcat(targetFileNameWithIndex, fileIndexAsString);

		int fdTarget = creat(targetFileNameWithIndex, 0644);

		#ifdef DEBUG
		std::cout << "\n*****Creating the file: " << targetFileNameWithIndex << "...";
		#endif

		for (int i = j*4+4-1; i >=j*4; --i) {
			
			#ifdef DEBUG
			if( j==66){
				int asad =12*3;
			}
			#endif

			if( i>=lineCount){
			 	EOF_REACHED_IN_SOURCE = 1;
				i = lineCount-1;
			}
			lseek(fdSource, lines[i].startingOffset+1, SEEK_SET );
			int len;

			{
				char *buff2 = (char*) malloc( sizeof(char)* lines[i].numberOfBytes );
				buff2 = (char*) malloc( lines[i].numberOfBytes );

				if( (len=read(fdSource, buff2, lines[i].numberOfBytes ))>0 )
					write(fdTarget, buff2, len);
			}
		}

		close(fdTarget);
		if( EOF_REACHED_IN_SOURCE)
			 	break;
		#ifdef DEBUG
		std::cout<<"Done*****\n";
		#endif
	}
	
	close(fdSource);
}


/*	
 *	Creates the destination path for the splitted-files
*/
void createDestiFolderStruc( char* desti ){
	int FLAG= 0;
	if( *desti == '.'){
		FLAG= 1;
	}
	char pwd[512];
	int ABSOLUTE_PATH = 0;
	getcwd(pwd, sizeof(pwd));

	if( *desti == '/' ){
		chdir("/");
		ABSOLUTE_PATH = 1;
	}

	const char delim[2] = "/";
	char* dirName; dirName = strtok(desti, delim);
	int i=0;
	
	while(dirName){
		if(chdir(dirName)==-1){
			mkdir(dirName, 0777);
			chdir(dirName);
		}
		dirName = strtok(NULL, "/");
		i++;
	}
	if(ABSOLUTE_PATH || FLAG)
		chdir(pwd);
	else
		while(i-->0)
			chdir("..");
}


/*	
 *	Creates an array of "Lines" structure for storing the meta-data about each line present in the file "fileName"
 *	under the path "source"
*/
Line* getMetdataForLines(const char* source, const char* fileName){

	char* f = (char*)malloc(256);
	f = strcpy(f,source);
	if( source[ strlen(source)-1 ] != '/' )
		f = strcat(f,"/");
	f = strcat(f,fileName);
	int fd = open( f , O_RDONLY);
	char buff[50];

	Line* lines = (Line*)malloc(sizeof(Line)*1024);
	int bytecount=0, len;
	lineCount = 0;

	while((len=read(fd, buff, 50))>0){
		
		#ifdef DEBUG
		if(len <50){
				int aa=34*46;
		}
		#endif

		for(int i=0; i<len; i++){
			bytecount++;
			if(buff[i]=='\n' || buff[i]=='\0'){
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
	return lines;
}


/*	
 *	Returns the list of all the regular files that are present in the "source" path, using the pointer "fNames"
 */
void getFilesInSourcDir(const char* source, char** fNames, int* filesCount ){
	
	struct dirent *dir;
	char* sourceDir = (char*)malloc(256);
	strcpy(sourceDir, "./");
	sourceDir = strcat(sourceDir, source);

	char pwd[256]={0};
	getcwd(pwd, sizeof(pwd));

	DIR *d = opendir(sourceDir);
	struct stat sb;
	if (d)
 	{
 		int i = 0;
		while ((dir = readdir(d)) != NULL)
			if (dir->d_type == DT_REG && stat(dir->d_name, &sb) == 0 && !(sb.st_mode & S_IXUSR) )
				*(fNames + i++) = dir->d_name;
		closedir(d);
		*filesCount = i;
	}
	//printf("In Other Func: %d\n", *filesCount);
}


/*
 *	Splits all the files that are present in the source-path such that 
 *	each splitted-file contains "linesCount" number of files and stores the splitted files under the "desti" path.
 */
void splitTac(const char* source, char* desti){

	char* temp = (char*)malloc(256);
	strcpy(temp, desti);

	createDestiFolderStruc(temp);
	char* filesInSourcDir[1024];
	char* fileName = (char*)malloc(32);
	int filesCount = 0;
	
	getFilesInSourcDir(source, filesInSourcDir, &filesCount);

	char* sourceAbs = (char*)malloc(256); 
	sourceAbs = (char*)malloc(256);
	realpath(source, sourceAbs);

	char* destiAbs = (char*)malloc(256); 
	destiAbs = (char*)malloc(256);
	realpath(desti, destiAbs);

	#ifdef DEBUG
	std::cout << sourceAbs <<"\n";
	std::cout << destiAbs <<"\n\n";
	std::cout << "\nLoop begins: "<< filesCount <<"\n";
	#endif

  	for (int i = 0; i < filesCount; ++i){
  		strcpy( fileName, filesInSourcDir[i] );
  		Line* l = getMetdataForLines(sourceAbs, filesInSourcDir[i]);
  		createSplittedFiles(sourceAbs, destiAbs, l, fileName);
 	}

 	#ifdef DEBUG
  	std::cout << "\nLoop ends\n";
  	#endif
}

/*	
 *	returns 1, if the source and desti are directories
 *				otherwise, 0 is returned
 */
int isSourceDestiDir(const char* source, char* desti){
	struct stat sourceMeta;

	if( lstat(source, &sourceMeta)<0 ){
		printf("lstat failed on source directory\n");
		exit(1);
	}

	if (S_ISDIR(sourceMeta.st_mode) && S_ISDIR(sourceMeta.st_mode))
		return 1;

	return 0;
}

int main(int argc, char *argv[])
{
	if(argc == 4 && isSourceDestiDir(argv[1], argv[2])){
		for (int i = 0; *(argv[3]+i)!='\0'; ++i)
			if( *(argv[3]+i)<'0' || *(argv[3]+i)>'9' ){
				printf("Parameters missing\nCorrect format: %s\n", "./a.out <source dir> <path to destination dir> <#lines> ");
				exit(1);
			}

		char* sourceAbs = (char*)malloc(256); 
		sourceAbs = (char*)malloc(256);
		realpath(argv[1], sourceAbs);

		char* destiAbs = (char*)malloc(256); 
		destiAbs = (char*)malloc(256);
		realpath(argv[2], destiAbs);

		/*if( strcmp(sourceAbs, destiAbs)==0 ){
			printf("Source and destination directory can't be the same.(It Leads to recursive split problem)\n");
			exit(1);			
		}*/
		linesPerFile = atoi(argv[3]);
		splitTac(argv[1], argv[2]);
	}
	else
		printf("Parameters missing\nCorrect format: %s\n", "./a.out <source dir> <path to destination dir> <#lines> ");
	return 0;
}