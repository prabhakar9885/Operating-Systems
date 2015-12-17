       #define _SVID_SOURCE
       /* print files in current directory in reverse order */
        #include <stdio.h>
        #include <stdlib.h>
       #include <dirent.h>

       int
       main(void)
       {
           struct dirent **namelist;
           int n;

           n = scandir(".", &namelist, NULL, alphasort);
           if (n < 0)
               perror("scandir");
           else {
               while (n--) {
                   printf("%s\n", namelist[n]->d_name);
                   free(namelist[n]);
               }
               free(namelist);
           }

           n = scandir(".", &namelist, NULL, versionsort);
           if (n < 0)
               perror("scandir");
           else {
               while (n--) {
                   printf("%s\n", namelist[n]->d_name);
                   free(namelist[n]);
               }
               free(namelist);
           }
       }
