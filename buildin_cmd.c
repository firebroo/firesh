#include "common.h"
#include "prompt.h"
#include "buildin_cmd.h"
#include "parse.h"

extern int running;
extern struct passwd *pwd;
extern char prompt[100];

static void __exec_exit_cmd__(char **cmd);
static void __exec_cd_cmd__(char **cmd);
static void __exec_pwd_cmd__(char **cmd);

CMD build_cmds[] = {
    {"cd",   __exec_cd_cmd__,   "change dir"}, 
    {"exit", __exec_exit_cmd__, "eixt shell"}, 
    {"pwd",  __exec_pwd_cmd__,  "return current dir"},
    {"",     NULL,              ""}
};

bool 
is_buildin_cmd(char **cmd) 
{
    int i = 0;

    while(build_cmds[i].func) {
        if (!strcmp(build_cmds[i].name, cmd[0])) {
            return true;
        }
        i++;
    }
    return false;
}

static void
__exec_exit_cmd__(char **cmd)
{
    if(cmd[1]){
        /*set exit code, todo*/
    }
    printf("exit\n");
    rl_callback_handler_remove ();
    running = 0;
}

static void
__exec_pwd_cmd__(char **cmd)
{
    char buf[1024], *s;
    s = getcwd(buf, sizeof(buf) - 1);
    if (s == 0) {
        printf("pwd: error\n");
    }  
    printf("%s\n", buf);
}

static void 
__exec_cd_cmd__(char **cmd)
{
    char path[1024];

    if (cmd[1] == NULL) { 
        /*cd*/
    } else if(cmd[2] != NULL) {
        printf("firesh: %s: %s\n", cmd[0], "too many arguments");
    } else {
        if (!strncmp(cmd[1], "~", strlen("~"))) {
            sprintf(path, "%s%s", pwd->pw_dir, cmd[1]+strlen("~"));
        } else {
            sprintf(path, "%s", cmd[1]);
        }
        if (chdir(path) == -1) {
            printf("firesh: %s: %s: %s\n", cmd[0], path, strerror(errno));
        } else {
            type_prompt(prompt); /*change dir, reset prompt*/
            reset_readline_callback();
        }
    }
}

void
exec_buildin_cmd(char **cmd)
{
    int i = 0;

    while(build_cmds[i].func) {
        if (!strcmp(build_cmds[i].name, cmd[0])) {
            (build_cmds[i].func)(cmd);
            return;
        }
        i++;
    }
}
