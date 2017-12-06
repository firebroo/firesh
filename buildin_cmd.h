#include <stdbool.h>

typedef void (*Func)(char **);

typedef struct cmd {
	char *name;
    Func func;
    char *desc;
} CMD;
bool is_buildin_cmd(char **cmd);
void exec_buildin_cmd(char **cmd);
extern char prompt[100];
