#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#include <sys/types.h>
#include <sys/select.h>

#include <sys/wait.h>

#include <signal.h>

#include <stdio.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include "prompt.h"

int sigintflag = 0;
#ifndef bool
#   define bool           unsigned char
#endif

#ifndef false
#   define false          (0)
#endif

#ifndef true
#   define true           (!(false))
#endif

static void cb_linehandler (char *);
static void sighandler (int);

int running;
int sigwinch_received;


/* Handle SIGWINCH and window size changes when readline is not active and
   reading a character. */
static void
sighandler (int sig)
{
        sigwinch_received = 1;
}

void 
siginthandle(int signo)
{
    printf("%s\n", "^C");
    fflush(stdout);
    sigintflag = 1;
}

char **
str_to_strptr(char *str)
{
        char a[100];
        int i = 0, j = 0;
        char** ptr = (char **)malloc(10 * sizeof(char*)); 
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
        free(ptr[i]);
        return ptr;
}

void
free_ptrstr(char **ptrstr)
{
        int i = 0;
        for(i = 0; ptrstr[i] != NULL; i++) {
                free(ptrstr[i]);
        }
        free(ptrstr);
}

void
cmd_error(char **cmd)
{
        printf("firesh: %s: command not found\n", cmd[0]);
        free_ptrstr(cmd);
        exit(0);
}


char *
trim(char *data, char c)
{
        size_t  len;

        while (*data == c) {
                data++;
        }
        len = strlen (data);
        if (len <= 0)
                return NULL;
        while (*(data+len-1) == c) {
                data[len-1] = '\0';
                len--;
        }

        return data;
}



/* Callback function called for each line when accept-line executed, EOF
   seen, or EOF character read.  This sets a flag and returns; it could
   also call exit(3). */
static void
cb_linehandler (char *line)
{
        if (sigintflag == 1) {
            sigintflag = 0;
            goto end;
        }
        int status;
        /* Can use ^D (stty eof) or `exit' to exit. */
        if (line == NULL || strcmp (line, "exit") == 0)
        {
                if (line == 0)
                        printf ("\n");
                printf ("exit\n");
                /* This function needs to be called to reset the terminal settings,
                   and calling it from the line handler keeps one extra prompt from
                   being displayed. */
                rl_callback_handler_remove ();

                running = 0;
        } else
        {
                if (*line) {
                    add_history (line);
                    int pid;
                    pid = fork();
                    if (pid < 0) {
                            printf("error in fork!");
                    } else if (pid == 0) {
                            char **cmd = str_to_strptr(trim(line, ' '));
                            int cmd_ret = execvp(cmd[0], cmd);
                            if (cmd_ret == -1) {
                                    cmd_error(cmd);
                            } else {
                                    exit(0);
                            }
                    } else {
                            waitpid(pid, &status, 0);
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

        if (argc < 2) {
                goto end;
        }

        char *arg[]={"ls","-a", NULL};
        char **cmd = NULL;
        while ( (opt = getopt (argc, argv, ":c:")) != -1) {
                switch (opt) {
                        case 'c':
                                cmd = str_to_strptr(trim(optarg, ' '));
                                int cmd_ret = execvp(cmd[0], cmd);
                                if (cmd_ret == -1) {
                                        cmd_error(cmd);
                                }
                        default:
                                goto end;
                }
        }

end:
        return false;
}

int
main (int c, char **v)
{
        fd_set fds;
        int r;

        /* Set the default locale values according to environment variables. */
        setlocale (LC_ALL, "");

        /* Handle window size changes when readline is not active and reading
           characters. */
        signal (SIGWINCH, sighandler);
        signal(SIGINT, siginthandle);
        /* Install the line handler. */
        check_argv(c, v);
        char prompt[100];
        type_prompt(prompt);
        rl_callback_handler_install (prompt, cb_linehandler);

        /* Enter a simple event loop.  This waits until something is available
           to read on readline's input stream (defaults to standard input) and
           calls the builtin character read callback to read it.  It does not
           have to modify the user's terminal settings. */
        running = 1;
        while (running)
        {
                FD_ZERO (&fds);
                FD_SET (fileno (rl_instream), &fds);    

                r = select (FD_SETSIZE, &fds, NULL, NULL, NULL);
                if (r < 0 && errno != EINTR)
                {
                        perror ("firesh: select");
                        rl_callback_handler_remove ();
                        break;
                }
                if (sigwinch_received)
                {
                        rl_resize_terminal ();
                        sigwinch_received = 0;
                }
                if (r < 0)
                        continue;     

                if (FD_ISSET (fileno (rl_instream), &fds))
                        rl_callback_read_char ();
        }

        return 0;
}
