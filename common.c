#include "common.h"

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
