#include "common.h"

extern char split[];

char *
trim(char *data, char c)
{
    size_t  len;

    while (*data == c) {
        data++;
    }
    len = strlen(data);
    if (len <= 0)
        return NULL;
    while (*(data+len-1) == c) {
        data[len-1] = '\0';
        len--;
    }

    return data;
}

char **
str_to_strptr(char *str, char **strptr)
{
    char *p;
    char *buff;
    buff = str;

    p = strsep(&buff, split);
    int i = 0;
    while(p && *p) {
        strptr[i++] = strdup(p);
        p = strsep(&buff, " "); 
    }
    strptr[i] = NULL;

    return strptr;
}

