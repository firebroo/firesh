#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char *path = "/usr/bin:/usr/local/bin";

char **
get_executablefile(char *path, char **ptr) {
    char   *p, *buff;
    DIR    *dp;
    struct stat     statbuf;
    struct dirent  *entry;
    buff = path;

    p = strsep(&buff, ":");
    while(p && *p) {
        stat(path, &statbuf);
        if((dp = opendir (p)) != NULL) {
            chdir(path);
            while((entry = readdir(dp)) != NULL) {
                lstat (entry->d_name, &statbuf);
                if (S_ISREG (statbuf.st_mode)) {
                    printf("%s\n", entry->d_name);
                }
            }
            chdir("..");
            closedir (dp);
        } else {
            fprintf (stderr, "cannot open directory: %s\n",path);
        }
        p = strsep(&buff, " "); 
    }
}

int
main(void)
{
    char *ptr[100], *p;
    p = strdup(path);
    get_executablefile(p, ptr);
    free(p);
    return 0;
}
