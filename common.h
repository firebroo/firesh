#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>

#ifndef bool
#   define bool           unsigned char
#endif

#ifndef false
#   define false          (0)
#endif

#ifndef true
#   define true           (!(false))
#endif

char *trim(char *data, char c);
