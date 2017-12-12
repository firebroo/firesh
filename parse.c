#include "common.h"
#include "parse.h"
#include "buildin_cmd.h"
#include "hashtable.h"

int sigintflag = 0;
int running;
int sigwinch_received;
int BGJOB = 0;

extern HashTable *hashtable;
extern char prompt[100];

void 
sig_chld(int sig)
{
    HashNode * node;
    int  i, pid, status;
    char pid_str[10];

    i = 1;
    while ((pid = (waitpid(-1, &status, 0))) != -1) {
        sprintf(pid_str, "%d", pid);
        node = (hash_table_lookup(hashtable, pid_str));
        if (node) {
            printf("[%d]+    Done%10s%s\n", i, " ", zStrValue(node));
            fflush(stdout);
            hash_table_remove(hashtable, pid_str);
            reset_readline_callback();
        }
        i++;
    }
}

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
    case SIGCHLD:
        sig_chld(sig);
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
    rl_callback_handler_remove();
    rl_callback_handler_install(prompt, cb_linehandler);
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
            if (BGJOB == 1) {                /*daemon*/
                BGJOB = 0;
                int pid;
                pid = fork();
                if (pid < 0) {
                    printf("error in fork!\n");
                }
                if(pid == 0) { /*children*/
                    if (is_buildin_cmd(cmd)) {
                        exec_buildin_cmd(cmd);
                        __free_ptrstr__(cmd);
                        exit(0);
                    } else {
                        if (execvp(cmd[0], cmd) == -1) {
                            __cmd_error__(cmd, "command not found");
                        } else {
                            exit(0);
                        }
                    }
                } else {
                    char pid_str[10];
                    sprintf(pid_str, "%d", pid);
                    hash_table_insert(hashtable, pid_str, cmd[0]);
                    printf("[1]: %d\n", pid);
                    __free_ptrstr__(cmd);
                }
            } else {
                if (is_buildin_cmd(cmd)) {
                    exec_buildin_cmd(cmd);
                    __free_ptrstr__(cmd);
                } else {
                    int pid;
                    pid = fork();
                    if (pid < 0) {
                        printf("error in fork!\n");
                    }
                    if (pid == 0) {  /*children*/
                        if (execvp(cmd[0], cmd) == -1) {
                            __cmd_error__(cmd, "command not found");
                        } else {
                            exit(0);
                        }
                    } else {        /*father*/
                        waitpid(pid, &status, 0);
                        __free_ptrstr__(cmd);
                    }
                }
            }
        }
        goto end;
    }
end:
    free(line);
}
