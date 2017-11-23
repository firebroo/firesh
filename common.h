#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>

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
char **str_to_strptr(char *str, char **strptr);
