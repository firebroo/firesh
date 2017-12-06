#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static char *path = "/usr/bin:/usr/local/bin";

char **
get_executablefile(char **ptr) {
    char   *buff = (char*)malloc(100);
    char   *p;
    DIR    *dp;
    struct dirent  *entry;
    int    i = 0;
    strncpy(buff, path, 100);

    p = strsep(&buff, ":");
    while(p && *p) {
        if((dp = opendir (p)) != NULL) {
            while ((entry = readdir(dp)) != NULL) {
                if(entry->d_type == 8 || entry->d_type == 10) {    
                    ptr[i++] = strdup(entry->d_name);
                } else if(entry->d_type == 4){
                }
            }
            closedir (dp);
        } else {
            fprintf (stderr, "cannot open directory: %s\n",path);
        }
        p = strsep(&buff, " "); 
    }

    free(buff);
    ptr[i] = NULL;
    return ptr;
}

//int
//main(void)
//{
//    int i = 0;
//    char *ptr[100000] = {NULL};
//    get_executablefile(ptr);
//    while(ptr[i]) {
//        printf("%s\n", ptr[i]);
//        i++;
//    }
//    return 0;
//}
