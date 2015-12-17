#define _SVID_SOURCE

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


typedef struct filesList
{
	char fileType;
	char* permission;
	int linkCount;
	char* owner;
	char* group;
	long size;
	char* dateTime;
	char* fileName;
	char* linkPath;
}filesList;

long count;


typedef struct CommandStruct
{
    const char* cmdName;        //  Command Name
    char* options;         //  Command-Options that are passed to the command
    char** nonOpt; //  Stores the file-names that are passed to the command
    int nonOptCount;              //  Total number of file-names that are passed as an argument to the command
}CommandStruct;




struct stat getStat(char* fileName){

    struct stat sb;

    if ( strlen(fileName)>0 && lstat(fileName, &sb) == -1) {
    	printf("lstat-Err: %s\n", fileName);
        perror("stat");
        //exit(EXIT_FAILURE);
    }

    return sb;
}



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


char* getCreationTime(time_t timeIs ){

    char* c_time_string = ctime(&timeIs);

    char* delim = (char*)" ";
    strtok(c_time_string, delim);
    char* month = strtok(NULL, delim);
    char* date = strtok(NULL, delim);
    char* hhmmss = strtok(NULL, delim);
    char* hh = strtok(hhmmss, ":");
    char* mm = strtok(NULL, ":");
    char* creationTime = (char*)malloc(14);
    sprintf(creationTime, "%3s %2s %2s:%2s", month, date, hh, mm);

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
    while(i < count && a[i] == b[i]) i++;
    return i == count;
}


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



filesList* ls(const char* path, CommandStruct cmd){

	int filesCount = 0;
	filesList* _filesList;
    struct dirent **namelist;
    char* temp = (char*)malloc(PATH_MAX);
	
	// Iterate and process the contents of each folder, one at a time
	for (int i = 0; i < cmd.nonOptCount; ++i)	{		

		strcat(temp, cmd.nonOpt[i]);
		if( temp[strlen(temp)-1] !='/')
			strcat(cmd.nonOpt[i], "/");

		//getFilesInSourcDir( p, filesInSourcDir, &filesCount);
		filesCount = scandir(temp, &namelist, NULL, alphasort);

		if(filesCount<0){
			printf("3. Ern");
			return NULL;
		}

		_filesList = (filesList*) malloc(sizeof(filesList) * filesCount);

		for (int fileIndex = 0; fileIndex < filesCount; ++fileIndex) {

	        strcpy(temp, cmd.nonOpt[i]);
	        //strcat(temp, "/");
	        strcat(temp, namelist[fileIndex]->d_name );

			struct stat sb = getStat( temp );

			_filesList[ fileIndex ].fileType = getFileType( sb );
	        _filesList[ fileIndex ].dateTime = getCreationTime(sb.st_mtime);
	        _filesList[ fileIndex ].owner = getOwner(sb.st_uid);
	        _filesList[ fileIndex ].group = getGroup(sb.st_uid);
	        _filesList[ fileIndex ].size = sb.st_size;
	        _filesList[ fileIndex ].permission = getPermissions(sb.st_mode);
	        _filesList[ fileIndex ].linkCount = sb.st_nlink;
        	_filesList[ fileIndex ].fileName = (char*)malloc(PATH_MAX);
        	strcpy(_filesList[ fileIndex ].fileName , namelist[fileIndex]->d_name);
	        
		    char* fileNameWithLink= (char*)malloc(PATH_MAX);

	        if( _filesList[ fileIndex ].fileType == 'l' ){

		        _filesList[ fileIndex ].linkPath = 
		        	getTargetForSymLink( temp, sb.st_size );
		        	strcpy(fileNameWithLink, _filesList[ fileIndex ].fileName);
		        	strcat(fileNameWithLink, (char*)" -> ");
		        	strcat(fileNameWithLink, _filesList[ fileIndex ].linkPath);
		    }
		    else{
		    	_filesList[ fileIndex ].linkPath = (char*)"";
		    	strcpy(fileNameWithLink, namelist[fileIndex]->d_name );
		    }
		}
		count = filesCount;
	}
	return _filesList;
}

void longListing(CommandStruct cmd, filesList* lst){

	char* temp = (char*)malloc(PATH_MAX);
	strcat(temp, cmd.nonOpt[0]);
	if( temp[strlen(temp)-1] !='/')
		strcat(cmd.nonOpt[0], "/");

	for (int fileIndex = 0; fileIndex < count; ++fileIndex) {

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

		    printf( "%c%s %5d %12s %12s %8ld %-12s %s \n", 
		                   lst[ fileIndex ].fileType
		                 , lst[ fileIndex ].permission
		                 , lst[ fileIndex ].linkCount
		                 , lst[ fileIndex ].owner
		                 , lst[ fileIndex ].group
		                 , lst[ fileIndex ].size
		                 , lst[ fileIndex ].dateTime
		                 , fileNameWithLink);		
	}
}

int main(int argc, char const *argv[])
{
	CommandStruct cmd = ParseCommand(argc, argv);
    
	filesList* lst = ls(argv[1], cmd);

	if(strstr(cmd.options, "l"))
		longListing(cmd, lst);

	return 0;
}