#include "common.h"
#include "buildin_cmd.h"

extern CMD build_cmds[];

void
help(void)
{
    int i = 0;

    printf("buildin cmds\n");
    printf("-----------------------------------------\n");
    while(build_cmds[i].func != NULL) {
        printf("%5s:%10s%s\n", build_cmds[i].name, " ", build_cmds[i].desc);
        i++;
    }
    exit(0);
}
