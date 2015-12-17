       #include <pwd.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <errno.h>

       int
       main(int argc, char *argv[])
       {
           struct passwd pwd;
           struct passwd *result;
           char *buf;
           size_t bufsize;
           int s;

           if (argc != 2) {
               fprintf(stderr, "Usage: %s username\n", argv[0]);
               exit(EXIT_FAILURE);
           }

           bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
           if (bufsize == -1)          /* Value was indeterminate */
               bufsize = 16384;        /* Should be more than enough */

           buf = (char*)malloc(bufsize);
           if (buf == NULL) {
               perror("malloc");
               exit(EXIT_FAILURE);
           }
           s = getpwuid_r( atoll(argv[1]), &pwd, buf, bufsize, &result);
           if (result == NULL) {
               if (s == 0)
                   printf("Not found\n");
               else {
                   errno = s;
                   perror("getpwnam_r");
               }
               exit(EXIT_FAILURE);
           }

           printf("Name: %s; UID: %ld; GID: %ld; Passwd: %s; gecos: %s; Home: %s; Shell: %s\n", 
                    pwd.pw_name, (long) pwd.pw_uid, (long)pwd.pw_gid, 
                    pwd.pw_passwd, pwd.pw_gecos, pwd.pw_dir, pwd.pw_shell);
           exit(EXIT_SUCCESS);
       }

