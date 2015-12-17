/*
 *		Name: 		B Prabhakar
 *		Roll no.:	201505618
 */

#define _SVID_SOURCE

#define RESET   "\033[0m"
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#include <sys/stat.h>
#include <time.h>
#include "pwd.h"
#include "grp.h"
#include <sys/types.h>
#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

/*Represents a record of ls -l*/
typedef struct filesList
{
	char fileType;			char* permission;
	int linkCount;			long inode;
	char* owner;			char* group;
	long size;				char* dateTime;
	char* month;			int year;
	int date, hh, mm;		char* fileName;
	char* linkPath;		
}filesList;

long count;
long STAT_ERROR = 0;

/*Stores the info that is obtained from the command-line*/
typedef struct CommandStruct
{
    const char* cmdName;        //  Command Name
    char* options;         		//  Command-Options that are passed to the command
    char** nonOpt; 				//  Stores the arguments that are not options
    int nonOptCount;            //  Total number of arguments that are not options
}CommandStruct;



/*Returns an instance of lstat*/
struct stat getStat(char* fileName){
    struct stat sb;
    if ( strlen(fileName)>0 && lstat(fileName, &sb) == -1) {
    	STAT_ERROR = 1;
    }
    return sb;
}


/* Returns the first character for the "ls -l" */
char getFileType(struct stat sb){
    char fileType;
    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:  fileType = 'b';  break;
        case S_IFCHR:  fileType = 'c';  break;
        case S_IFDIR:  fileType = 'd';  break;
        case S_IFIFO:  fileType = 'p';  break;
        case S_IFLNK:  fileType = 'l';  break;
        case S_IFREG:  fileType = '-';  break;
        case S_IFSOCK: fileType = 's';  break;
        default:       fileType = 'X';  
    }
    return fileType;
}



/* Returns the path to the file referenced by symlink */
char* getTargetForSymLink(char* linkName, int st_size){
    
    char* target = (char*)malloc(st_size+1);
    if (target == NULL) {
        fprintf(stderr, "insufficient memory\n");
        exit(EXIT_FAILURE);
    }
    int r = readlink(linkName, target, st_size + 1);
    if (r == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }
    if (r > st_size) {
        fprintf(stderr, "symlink increased in size "
                            "between lstat() and readlink()\n");
        exit(EXIT_FAILURE);
    }
    target[r] = '\0';

    return target;
}


char* getCreationTime(time_t timeIs){

    char* c_time_string = (char*)malloc(30);
   	strcpy( c_time_string, (char*) ctime(&timeIs) );

    char* delim = (char*)" ";
    strtok(c_time_string, delim);
    char* month = strtok(NULL, delim);
    char* date = strtok(NULL, delim);
    char* hhmmss = strtok(NULL, delim);
    char* year = strtok(NULL, delim);
    year[strlen(year)-1] = 0;
    char* hh = strtok(hhmmss, ":");

    char* mm = strtok(NULL, ":");
    char* creationTime = (char*)malloc(14);
    sprintf(creationTime, "%3s %4s %2s %2s:%2s", month, year, date, hh, mm);

    free( c_time_string );
    return creationTime;
}



char* getOwner(uid_t ownerId){
    struct passwd pwd, *result;
    size_t bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    char* buf = (char*) malloc(bufsize);
    int s = getpwuid_r(ownerId, &pwd, buf, bufsize, &result);

    if (result == NULL) {
        if (s == 0)
            printf("Not found\n");
        else {
            perror("getpwnam_r");
        }
        exit(EXIT_FAILURE);
    }
    return pwd.pw_name;
}


char* getGroup(uid_t ownerId){
    struct group grp, *result;
    size_t bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
    char* buf = (char*) malloc(bufsize);
    int s = getgrgid_r(ownerId, &grp, buf, bufsize, &result);

    if (result == NULL) {
        if (s == 0)
            printf("Not found\n");
        else {
            perror("getgrgrid_r");
        }
        exit(EXIT_FAILURE);
    }
    return grp.gr_name;
}


char* getPermissions( long st_mode ){

    char* perms = (char*) malloc(10);
    *(perms + 9) = '\0';

    if ( S_IRUSR & st_mode)     strcpy(perms, "r");
    else                        strcpy(perms, "-");
    if ( S_IWUSR & st_mode)     strcat(perms, "w");
    else                        strcat(perms, "-");
    if ( S_IXUSR & st_mode)     strcat(perms, "x");
    else                        strcat(perms, "-");

    if ( S_IRGRP & st_mode)     strcat(perms, "r");
    else                        strcat(perms, "-");
    if ( S_IWGRP & st_mode)     strcat(perms, "w");
    else                        strcat(perms, "-");
    if ( S_IXGRP & st_mode)     strcat(perms, "x");
    else                        strcat(perms, "-");

    if ( S_IROTH & st_mode)     strcat(perms, "r");
    else                        strcat(perms, "-");
    if ( S_IWOTH & st_mode)     strcat(perms, "w");
    else                        strcat(perms, "-");
    if ( S_IXOTH & st_mode)     strcat(perms, "x");
    else                        strcat(perms, "-");

    return perms;
}





int matchBegining(const char* a, const char *b, int count){
    int i = 0;
    while(i < count && a[i] && b[i] && a[i] == b[i]) i++;
    return i == count;
}



/*	
 *		Parses the command int the data-structure CommandStruct		
 */
CommandStruct ParseCommand(int argc, const char* argv[]){
    
    CommandStruct cmd;
    char** nonOpt =(char**) malloc(1024);

    cmd.cmdName = argv[0];
    int nonOptCount = 0;
    char options[52]={ '\0' };
    
    // Obtain the options and command parameters.... options may contain duplicate letters.
    for (int i = 1; i < argc; ++i){
        if( matchBegining( argv[i], "-", 1 ) )
            strcat(options, argv[i]+1);
        else{
            nonOpt[nonOptCount] = (char*)malloc(25);
            strcpy( nonOpt[nonOptCount++], (char*)argv[i] );
        }
    }
    cmd.nonOptCount = nonOptCount;

    cmd.nonOpt = (char**)malloc(nonOptCount);
    for (int i = 0; i < nonOptCount; ++i)
        cmd.nonOpt[i] = nonOpt[i];

    // Remove the duplicate options from the Options[] by using Counting-sort
    int aux[26*2] = {0};
    for (unsigned int i = 0; i < strlen(options); ++i){
        if(options[i] != '-'){
            if(options[i] >='A' && options[i] <='Z')
                aux[options[i] +26 - 'A'] = 1;
            else
                aux[options[i]- 'a'] = 1;
        }
    }

    int k = 0, j=0;
    for ( ; k < 26; ++k){   
        if(aux[k])
            options[j++] = (char)(k +'a');
    }
    for ( ; k < 26*2; ++k){ 
        if(aux[k])
            options[j++] = (char)(k-26 +'A');
    }
    options[j] = '\0';
    
    cmd.options = (char*)malloc(j);
    strcpy(cmd.options, options);
    return cmd;
}



int alphasort(const struct dirent **e1, const struct dirent **e2) {
  const char *a = (*e1)->d_name;
  const char *b = (*e2)->d_name;
  return strcmp( a, b);
}


/*
 *	Retruns a single record for the "ls -l"
 */
filesList* getLsRecord(struct stat sb, char* dirUnderFolder, char* dirName){
	filesList* fsl = (filesList*)malloc( sizeof(filesList) );

	fsl->fileType = getFileType( sb );
	if(fsl->fileType != 'd' && dirName[ strlen(dirName)-1]=='/' )
    	dirName[ strlen(dirName)-1] = 0;

    fsl->dateTime = getCreationTime(sb.st_mtime);
    fsl->month =(char*)malloc(3);    
    char* tempDateTime = (char*)malloc(PATH_MAX);
	strcpy( tempDateTime, fsl->dateTime);

	//printf("%s\n", tempDateTime);

	strcpy(fsl->month, strtok(tempDateTime, (char*)": ") );
	fsl->year = atoi(strtok(NULL, (char*)": "));
	fsl->date = atoi(strtok(NULL, (char*)": "));
	fsl->hh = atoi(strtok(NULL, (char*)": "));
	fsl->mm  = atoi(strtok(NULL, (char*)": "));
	sprintf( fsl->dateTime, "%3s %2d %2d:%2d", 
					        	fsl->month
						       , fsl->date
						       , fsl->hh
						       , fsl->mm);

	fsl->owner = getOwner(sb.st_uid);
	fsl->group = getGroup(sb.st_uid);
	fsl->size = sb.st_size;
	fsl->inode = sb.st_ino;
	fsl->permission = getPermissions(sb.st_mode);
	fsl->linkCount = sb.st_nlink;
    fsl->fileName = (char*)malloc(PATH_MAX);
    strcpy(fsl->fileName , dirName);
	        
	char* fileNameWithLink= (char*)malloc(PATH_MAX);

	if( fsl->fileType == 'l' ){
		fsl->linkPath = getTargetForSymLink( dirUnderFolder, sb.st_size );
		strcpy(fileNameWithLink, fsl->fileName);
		strcat(fileNameWithLink, (char*)" -> ");
		strcat(fileNameWithLink, fsl->linkPath);
	}
	else{
		fsl->linkPath = (char*)"";
		strcpy(fileNameWithLink, dirName );
	}

	return fsl;
}


int getMonthNumber(char* month){
	if( ! strcmp(month, "Jan"))		return 1;
	if( ! strcmp(month, "Feb"))		return 2;
	if( ! strcmp(month, "Mar"))		return 3;
	if( ! strcmp(month, "Apr"))		return 4;
	if( ! strcmp(month, "May"))		return 5;
	if( ! strcmp(month, "Jun"))		return 6;
	if( ! strcmp(month, "Jul"))		return 7;
	if( ! strcmp(month, "Aug"))		return 8;
	if( ! strcmp(month, "Sep"))		return 9;
	if( ! strcmp(month, "Oct"))		return 10;
	if( ! strcmp(month, "Nov"))		return 11;
	if( ! strcmp(month, "Dec"))		return 12;
	return -1;
}


/*
 *		Functions used for persorming the sor, based on time and file-size
 *
 */
filesList* _qsort_Month(filesList* lst, int count){
	for (int i = 0; i < count-1; ++i)
	{
		int Max = i;
		for (int j = i+1; j < count; ++j)
		{
			if(lst[j].year == lst[Max].year &&  getMonthNumber(lst[j].month) > getMonthNumber(lst[Max].month) )
				Max = j;
		}
		filesList temp = lst[Max];
		lst[Max] = lst[i];
		lst[i] = temp;
	}
	return lst;
}
filesList* _qsort_Year(filesList* lst, int count){
	for (int i = 0; i < count-1; ++i)
	{
		int Max = i;
		for (int j = i+1; j < count; ++j)
		{
			if(lst[j].year > lst[Max].year)
				Max = j;
		}
		filesList temp = lst[Max];
		lst[Max] = lst[i];
		lst[i] = temp;
	}
	return lst;
}
filesList* _qsort_date(filesList* lst, int count){
	for (int i = 0; i < count-1; ++i)
	{
		int Max = i;
		for (int j = i+1; j < count; ++j)
		{
			if( lst[j].year == lst[Max].year && getMonthNumber(lst[j].month) == getMonthNumber(lst[Max].month)
						&&  lst[j].date > lst[Max].date)
				Max = j;
		}
		filesList temp = lst[Max];
		lst[Max] = lst[i];
		lst[i] = temp;
	}
	return lst;
}
filesList* _qsort_HH(filesList* lst, int count){
	for (int i = 0; i < count-1; ++i)
	{
		int Max = i;
		for (int j = i+1; j < count; ++j)
		{
			if( lst[j].year == lst[Max].year && getMonthNumber(lst[j].month) == getMonthNumber(lst[Max].month)
						&&  lst[j].date == lst[Max].date
						&&  lst[j].hh == lst[Max].hh)
				Max = j;
						
		}
		filesList temp = lst[Max];
		lst[Max] = lst[i];
		lst[i] = temp;
	}
	return lst;
}
filesList* _qsort_MM(filesList* lst, int count){
	for (int i = 0; i < count-1; ++i)
	{
		int Max = i;
		for (int j = i+1; j < count; ++j)
		{
			if( lst[j].year == lst[Max].year && getMonthNumber(lst[j].month) == getMonthNumber(lst[Max].month)
						&&  lst[j].date == lst[Max].date
						&&	lst[j].hh == lst[Max].hh
						&& 	lst[j].mm > lst[Max].mm)
				Max = j;
		}
		filesList temp = lst[Max];
		lst[Max] = lst[i];
		lst[i] = temp;
	}
	return lst;
}
filesList* _qsort_Size(filesList* lst, int count){
	for (int i = 0; i < count-1; ++i)
	{
		int Max = i;
		for (int j = i+1; j < count; ++j)
		{
			if(lst[j].size > lst[Max].size)
				Max = j;
		}
		filesList temp = lst[Max];
		lst[Max] = lst[i];
		lst[i] = temp;
	}
	return lst;
}


/*
 *	Function takes care of rendering the output on standard output device
 */
void display(CommandStruct cmd, filesList* lst, int i, int isDir){
	
	if(cmd.nonOptCount > 1 && STAT_ERROR==0 && isDir){
		printf("%s:\n", cmd.nonOpt[i]);
		STAT_ERROR = 0;
	}

	if(strstr(cmd.options, "S")){
		lst = _qsort_Size(lst, count);
		/*qsort((void *) lst, count, sizeof(struct filesList), 
						(compfn)compare );*/
	}
	if(strstr(cmd.options, "t")){
		//lst = _qsort_Year(lst, count);
		lst = _qsort_Month(lst, count);
		lst = _qsort_date(lst, count);
		lst = _qsort_HH(lst, count);
		lst = _qsort_MM(lst, count);
	}

	char* temp = (char*)malloc(PATH_MAX);
	strcat(temp, cmd.nonOpt[0]);
	if( temp[strlen(temp)-1] !='/')
		strcat(cmd.nonOpt[0], "/");

	int fileIndex, colNum = 0;

	for ( fileIndex = 0; fileIndex < count; ++fileIndex) {

			if( strstr(cmd.options, "a")==NULL && matchBegining(lst[ fileIndex ].fileName, ".", 1) )
				continue;
			
			char* fileNameWithLink= (char*)malloc(PATH_MAX);

	        if( lst[ fileIndex ].fileType == 'l' ){

		        	strcpy(fileNameWithLink, lst[ fileIndex ].fileName);
		        	strcat(fileNameWithLink, (char*)" -> ");
		        	strcat(fileNameWithLink, lst[ fileIndex ].linkPath);
		    }
		    else{
		    	lst[ fileIndex ].linkPath = (char*)"";
		    	strcpy(fileNameWithLink, lst[fileIndex].fileName );
		    }

		    
			if(strstr(cmd.options, "i")){
			    printf( "%-40ld ", lst[ fileIndex ].inode);
			}
		    if(strstr(cmd.options, "l")){

		    	if( lst[ fileIndex ].fileType =='l'){
				    printf( "%c%s %5d %12s %12s %12ld %-12s" BOLDRED " "RESET, 
				                   lst[ fileIndex ].fileType
				                 , lst[ fileIndex ].permission
				                 , lst[ fileIndex ].linkCount
				                 , lst[ fileIndex ].owner
				                 , lst[ fileIndex ].group
				                 , lst[ fileIndex ].size
				                 , lst[ fileIndex ].dateTime);	
				}
				else if( lst[ fileIndex ].fileType =='d'){
				    printf( "%c%s %5d %12s %12s %12ld %-12s" BOLDBLUE " "RESET, 
				                   lst[ fileIndex ].fileType
				                 , lst[ fileIndex ].permission
				                 , lst[ fileIndex ].linkCount
				                 , lst[ fileIndex ].owner
				                 , lst[ fileIndex ].group
				                 , lst[ fileIndex ].size
				                 , lst[ fileIndex ].dateTime);	
				}
				else{
				    printf( "%c%s %5d %12s %12s %12ld %-12s %s "RESET, 
				                   lst[ fileIndex ].fileType
				                 , lst[ fileIndex ].permission
				                 , lst[ fileIndex ].linkCount
				                 , lst[ fileIndex ].owner
				                 , lst[ fileIndex ].group
				                 , lst[ fileIndex ].size
				                 , lst[ fileIndex ].dateTime
				                 , (lst[ fileIndex ].fileType=='d' || strstr(lst[ fileIndex ].permission, "x")!=NULL)?
				                 									BOLDGREEN:RESET);	
				}	
			}
			if( lst[ fileIndex ].fileType =='l')
			    printf( BOLDRED "%-25s"RESET,   fileNameWithLink);	
			else if( lst[ fileIndex ].fileType =='d'){
			    printf( BOLDBLUE "%-25s"RESET, fileNameWithLink);					}
			else
				printf( "%s%-25s"RESET,       ( strstr(lst[ fileIndex ].permission, "x")!=NULL) ? 
													BOLDGREEN:RESET, fileNameWithLink);	
			
			if( strstr(cmd.options, "l")==NULL && ++colNum ==3 ){
				printf("\n");
				colNum = 0;
			}
			else if( strstr(cmd.options, "l") )
				printf("\n");
	}

	//printf("*****************%s---\n", !strstr(cmd.options, "l"));

	if(colNum != 0 && ! strstr(cmd.options, "l") )
				printf("\n");
}



/*
 *	Entry point for the "ls" logic
 */
filesList* ls(CommandStruct cmd, int i){

	int filesCount = 0;
	filesList* _filesList;
    struct dirent **namelist;
    char* temp = (char*)malloc(PATH_MAX);

	if( cmd.nonOptCount == 0){
		cmd.nonOpt[0] = (char*)malloc(20);
		strcpy( cmd.nonOpt[0], "." );
		cmd.nonOptCount = 1;
	}
	
	// Iterate and process the contents of each folder, one at a time

	if( ! matchBegining(cmd.nonOpt[i], (char*)"./", 2)  &&  ! matchBegining(cmd.nonOpt[i], (char*)"/", 1) ){
		strcpy(temp, "./");
		strcat(temp, cmd.nonOpt[i]);
	}
	else
		strcpy(temp, cmd.nonOpt[i]);

	if(temp[strlen(temp)-1] !='/')
		strcat(temp, "/");

	//getFilesInSourcDir( p, filesInSourcDir, &filesCount);
	if(strstr(cmd.options, ""))
		filesCount = scandir(temp, &namelist, NULL, alphasort);

	int isDir = 1;

	if(filesCount < 0){ // The specified name could be a file or wrong-name
		struct stat sb = getStat( temp );
		if( STAT_ERROR ){
			cmd.nonOpt[i][strlen(cmd.nonOpt[i])-1] = 0;
			printf("ls: cannot access %s: No such file or directory\n", cmd.nonOpt[i]);
			count = 0;
		}
		else{
			isDir = 0;
			_filesList = (filesList*) malloc(sizeof(filesList));
			filesList* fl = getLsRecord(sb, temp, cmd.nonOpt[i]);
			_filesList[ 0 ] = *fl;
			count = 1;
		}
	}
	else{
		_filesList = (filesList*) malloc(sizeof(filesList) * filesCount);
		for (int fileIndex = 0; fileIndex < filesCount; ++fileIndex) {

		    strcpy(temp, cmd.nonOpt[i]);
		    if(temp[strlen(temp)-1] !='/')
				strcat(temp, "/");
		    strcat(temp, namelist[fileIndex]->d_name );
			if(temp[strlen(temp)-1] !='/')
				strcat(temp, "/");

			struct stat sb = getStat( temp );

			_filesList[ fileIndex ] = *(getLsRecord(sb, temp, namelist[fileIndex]->d_name));
		}
		count = filesCount;
	}


	if( cmd.nonOptCount > 1){
		printf("%s:\n", cmd.nonOpt[i]);
	}

	display(cmd, _filesList, i, isDir);

	return _filesList;	
}


int main(int argc, char const *argv[])
{
	CommandStruct cmd = ParseCommand(argc, argv);

	if(cmd.nonOptCount == 0){
		cmd.nonOpt = (char**)malloc(sizeof(CommandStruct*));
		cmd.nonOpt[0] = (char*)malloc(sizeof(CommandStruct));
		strcpy( cmd.nonOpt[0] , "./" );
		cmd.nonOptCount = 1;
	}

	for (int i = 0; i < cmd.nonOptCount; ++i)	{
		ls( cmd, i);
		if(cmd.nonOptCount > 1 && i!=cmd.nonOptCount-1 )
			printf("\n");
	}

	return 0;
}