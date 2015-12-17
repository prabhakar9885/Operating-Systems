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



struct stat getStat(char* fileName){

    struct stat sb;

    if (lstat(fileName, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
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



int alphasort(const struct dirent **e1, const struct dirent **e2) {
  const char *a = (*e1)->d_name;
  const char *b = (*e2)->d_name;
  return strcmp(b, a);
}


char* getOwner(uid_t ownerId){
    struct passwd pwd, *result;
    size_t bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    char* buf = (char*) malloc(bufsize);
    int s = getpwuid_r(ownerId, &pwd, buf, bufsize, &result), errno;

    if (result == NULL) {
        if (s == 0)
            printf("Not found\n");
        else {
            errno = s;
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
    int s = getgrgid_r(ownerId, &grp, buf, bufsize, &result), errno;

    if (result == NULL) {
        if (s == 0)
            printf("Not found\n");
        else {
            errno = s;
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




void longlisting(dirent **namelist, int n){
    
    char* row = (char*)malloc(256);

    printf("total %ld\n", getStat((char*)".").st_blocks);

    while (n--) {
        struct stat sb = getStat( namelist[n]->d_name );
        char fileType = getFileType( sb );
        char* creationTime = getCreationTime(sb.st_mtime);
        char* owner = getOwner(sb.st_uid);
        char* group = getGroup(sb.st_uid);
        char* perm = getPermissions(sb.st_mode);

        time_t tt = sb.st_mtime;

        char* symlinkTarget = (char*)"";
        char* fileNameAndLink = (char*)malloc(128);
        strcpy(fileNameAndLink, namelist[n]->d_name);

        if(fileType == 'l'){
            symlinkTarget = getTargetForSymLink( namelist[n]->d_name, sb.st_size );
            strcat(fileNameAndLink, " -> ");
            strcat(fileNameAndLink, symlinkTarget);
        }

        sprintf(row, "%c%s %12s  %12s  %8ld  %-12s  %s", 
                        fileType, perm, owner, group, sb.st_size, creationTime, fileNameAndLink);
        printf("%s\n", row);

        free(namelist[n]);
    }
    free(namelist);
}


int main(int argc, char const *argv[]){

    struct dirent **namelist;
    int n;

    char* sourceAbs = (char*)malloc(256); 
    realpath(argv[1], sourceAbs);
    
    //printf("Path: %s\n", sourceAbs);
    n = scandir(".", &namelist, NULL, alphasort);

    if (n < 0)
        perror("scandir");
    else {
        longlisting(namelist, n);
    }
}
