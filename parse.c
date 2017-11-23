#include "common.h"
#include "parse.h"
#include "buildin_cmd.h"

int sigintflag = 0;
int running;
int sigwinch_received;
extern char prompt[100];

/* Handle SIGWINCH and window size changes when readline is not active and
   reading a character. */
void
sighandler(int sig)
{
    switch (sig) {

    case SIGWINCH:
        sigwinch_received = 1;
        break;
    case SIGINT:
        printf("%s\n", "^C");
        fflush(stdout);
        reset_readline_callback();
        break;
    }
}



void
__free_ptrstr__(char **ptrstr)
{
    int i = 0;
    for(i = 0; ptrstr[i] != NULL; i++) {
        free(ptrstr[i]);
    }
}

void
__cmd_error__(char **cmd, char* error)
{
    printf("firesh: %s: %s\n", cmd[0], error);
    __free_ptrstr__(cmd);
    exit(0);
}

void
reset_readline_callback()
{
    rl_callback_handler_remove ();
    rl_callback_handler_install (prompt, cb_linehandler);
}


void
cb_linehandler (char *line)
{
    int status;
    /* Can use ^D (stty eof) to exit. */
    if (line == NULL ) {
        if (line == 0)
            printf ("\n");
        printf ("exit\n");
        /* This function needs to be called to reset the terminal settings,
           and calling it from the line handler keeps one extra prompt from
           being displayed. */
        rl_callback_handler_remove ();
        running = 0;
    } else {
        if (*line) {
            add_history (line);
            char *strptr[100];
            char **cmd = str_to_strptr(trim(line, ' '), strptr);
            if (is_buildin_cmd(cmd)) {
                exec_buildin_cmd(cmd);
                __free_ptrstr__(cmd);
            } else {
                int pid;
                pid = fork();
                if (pid < 0) {
                    printf("error in fork!\n");
                } else if (pid == 0) {
                    if (execvp(cmd[0], cmd) == -1) {
                        __cmd_error__(cmd, "command not found");
                    } else {
                        exit(0);
                    }
                } else {
                    waitpid(pid, &status, 0);
                }
            }
        }
        goto end;
    }
end:
    free(line);
}

