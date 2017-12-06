#include "help.h"
#include "parse.h"
#include "prompt.h"
#include "firesh.h"
#include "common.h"
#include "complet.h"
#include "hashtable.h"
#include "buildin_cmd.h"

char split[] = {0x20, 0x09, '\0'};
HashTable *hashtable;

extern char prompt[100];
extern int sigwinch_received;
extern int running;

/* Callback function called for each line when accept-line executed, EOF
   seen, or EOF character read.  This sets a flag and returns; it could
   also call exit(3). */

bool
check_argv (int argc, char *argv[])
{
    int    opt;
    int    cmd_ret;
    char **cmd = NULL;
    char *strptr[100];

    if (argc < 2) {
        goto end;
    }

    while((opt = getopt (argc, argv, ":c:h")) != -1) {
        switch(opt) {

        case 'c':
            cmd = str_to_strptr(trim(optarg, ' '), strptr);
            int cmd_ret = execvp(cmd[0], cmd);
            if (cmd_ret == -1) {
                printf("fish: %s: command not found", cmd[0]);
                exit(0);
            }
        case 'h':
            help();
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

    hashtable = malloc (sizeof (HashTable));
    hash_table_init(hashtable);
    initialize_readline();
    signal(SIGWINCH, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGCHLD, sighandler);
    /* Install the line handler. */
    type_prompt(prompt);
    rl_callback_handler_install(prompt, cb_linehandler);

    /* Enter a simple event loop.  This waits until something is available
       to read on readline's input stream (defaults to standard input) and
       calls the builtin character read callback to read it.  It does not
       have to modify the user's terminal settings. */
    running = 1;
    while (running) {
        FD_ZERO(&fds);
        FD_SET(fileno(rl_instream), &fds);    

        r = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
        if(r < 0 && errno != EINTR) {
            perror("firesh: select");
            rl_callback_handler_remove();
            break;
        }
        if(sigwinch_received) {
            rl_resize_terminal();
            sigwinch_received = 0;
        }
        if(r < 0)
            continue;     

        if(FD_ISSET(fileno(rl_instream), &fds))
            rl_callback_read_char();
    }

    return 0;
}
