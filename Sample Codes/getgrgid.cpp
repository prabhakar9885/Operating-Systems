       #include <grp.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <errno.h>

       int
       main(int argc, char *argv[])
       {
           struct group grp;
           struct group *grpResult;
           char *buf;
           size_t bufsize;
           int s;

           if (argc != 2) {
               fprintf(stderr, "Usage: %s username\n", argv[0]);
               exit(EXIT_FAILURE);
           }

           bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
           if (bufsize == -1)          /* Value was indeterminate */
               bufsize = 16384;        /* Should be more than enough */

           buf = (char*)malloc(bufsize);
           if (buf == NULL) {
               perror("malloc");
               exit(EXIT_FAILURE);
           }
           s = getgrgid_r(atoi(argv[1]), &grp, buf, bufsize, &grpResult);
           if (grpResult == NULL) {
               if (s == 0)
                   printf("Not found\n");
               else {
                   errno = s;
                   perror("getgrgid_r");
               }
               exit(EXIT_FAILURE);
           }

           printf("GID: %d; Name: %s; Passwd: %s; Members: ", grp.gr_gid, grp.gr_name, grp.gr_passwd);//, grp.gr_mem);
           printf(" -- %ld--",sizeof(grp.gr_mem)/sizeof(char*));
           for (int i = 0; i < sizeof(grp.gr_mem)/sizeof(char*); ++i)
           {
              printf("%s; ", *(grp.gr_mem + i));
           }
           printf("\n");
           exit(EXIT_SUCCESS);
       }

