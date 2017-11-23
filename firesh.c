#include "prompt.h"
#include "firesh.h"
#include "common.h"

extern char prompt[100];
static int sigintflag = 0;
static int running;
static int sigwinch_received;
static char *build_cmds[] = {"cd"};

static void __cb_linehandler__(char *);
static void __sighandler__(int);
static bool __is_buildin_cmd__(char **cmd);
static void __exec_buildin_cmd__(char **cmd);
static void __cmd_error__(char **cmd, char* error);
static void __reset_readline_callback__();
static void __free_ptrstr__(char **ptrstr);
static char** __str_to_strptr__(char *str);


static bool 
__is_buildin_cmd__(char **cmd) 
{
    int i;
    int len = sizeof(build_cmds) / sizeof(char*);
    for(i = 0; i < len; i++) {
        if (!strcmp(build_cmds[i], cmd[0])) {
            return true;
        }
    }
    return false;
}

static void 
__exec_buildin_cmd__(char **cmd)
{
    char path[1024];

    if (cmd[1] == NULL) { 
        /*cd*/
    } else if(cmd[2] != NULL) {
        printf("firesh: %s: %s\n", cmd[0], "too many arguments");
    } else if (!strcmp(cmd[0], "cd")) {
        if (!strncmp(cmd[1], "~", strlen("~"))) {
            sprintf(path, "%s%s", pwd->pw_dir, cmd[1]+strlen("~"));
        } else {
            sprintf(path, "%s", cmd[1]);
        }
        if (chdir(path) == -1) {
            printf("firesh: %s: %s: %s\n", cmd[0], path, strerror(errno));
        } else {
            type_prompt(prompt); /*change dir, reset prompt*/
            __reset_readline_callback__();
        }
    }

    __free_ptrstr__(cmd);
    return;
}

/* Handle SIGWINCH and window size changes when readline is not active and
   reading a character. */
static void
__sighandler__ (int sig)
{
    switch (sig) {

    case SIGWINCH:
        sigwinch_received = 1;
        break;
    case SIGINT:
        printf("%s\n", "^C");
        fflush(stdout);
        __reset_readline_callback__();
        break;
    }
}

static void
__reset_readline_callback__()
{
    rl_callback_handler_remove ();
    rl_callback_handler_install (prompt, __cb_linehandler__);
}

static char **
__str_to_strptr__(char *str)
{
    char a[100];
    int i = 0, j = 0;
    char** ptr = (char **)malloc(100 * sizeof(char*)); 
    if (str && *str) {
        while (*str) {
            if (*str != ' ') {
                a[j++] = *str;
            } else {
                a[j] = '\0';
                ptr[i++] = strdup(a);
                j = 0;
            }
            str++;
        }
        a[j] = '\0';
        ptr[i] = strdup(a);
        ptr[++i] = NULL;
    } else {
        return NULL;
    }
    return ptr;
}

void
__free_ptrstr__(char **ptrstr)
{
    int i = 0;
    for(i = 0; ptrstr[i] != NULL; i++) {
        free(ptrstr[i]);
    }
    free(ptrstr);
}

void
__cmd_error__(char **cmd, char* error)
{
    printf("firesh: %s: %s\n", cmd[0], error);
    __free_ptrstr__(cmd);
    exit(0);
}



/* Callback function called for each line when accept-line executed, EOF
   seen, or EOF character read.  This sets a flag and returns; it could
   also call exit(3). */
static void
__cb_linehandler__ (char *line)
{
    int status;
    /* Can use ^D (stty eof) or `exit' to exit. */
    if (line == NULL || strcmp (line, "exit") == 0) {
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
            char **cmd = __str_to_strptr__(trim(line, ' '));
            if (__is_buildin_cmd__(cmd)) {
                __exec_buildin_cmd__(cmd);
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


bool
check_argv (int argc, char *argv[])
{
    int    opt;
    int    cmd_ret;
    char **cmd = NULL;

    if (argc < 2) {
        goto end;
    }

    while((opt = getopt (argc, argv, ":c:")) != -1) {
        switch(opt) {

        case 'c':
            cmd = __str_to_strptr__(trim(optarg, ' '));
            int cmd_ret = execvp(cmd[0], cmd);
            if (cmd_ret == -1) {
                __cmd_error__(cmd, "command not found");
            }
        default:
            goto end;
        }
    }

end:
    return false;
}

int
main(int argc, char **argv)
{
    int r;
    fd_set fds;

    check_argv(argc, argv);
    setlocale(LC_ALL, "");

    signal(SIGWINCH, __sighandler__);
    signal(SIGINT, __sighandler__);
    /* Install the line handler. */
    type_prompt(prompt);
    rl_callback_handler_install(prompt, __cb_linehandler__);

    /* Enter a simple event loop.  This waits until something is available
       to read on readline's input stream (defaults to standard input) and
       calls the builtin character read callback to read it.  It does not
       have to modify the user's terminal settings. */
    running = 1;
    while (running) {
        FD_ZERO(&fds);
        FD_SET(fileno (rl_instream), &fds);    

        r = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
        if(r < 0 && errno != EINTR) {
            perror("firesh: select");
            rl_callback_handler_remove ();
            break;
        }
        if(sigwinch_received) {
            rl_resize_terminal ();
            sigwinch_received = 0;
        }
        if(r < 0)
            continue;     

        if(FD_ISSET(fileno (rl_instream), &fds))
            rl_callback_read_char ();
    }

    return 0;
}
