#include "prompt.h"

static const int max_name_len = 256;
static const int max_path_len = 1024;

void 
type_prompt(char *prompt)
{
    int length;
    extern struct passwd *pwd;
    char hostname[max_name_len];
    char pathname[max_path_len];

    pwd = getpwuid(getuid());
    getcwd(pathname, max_path_len);
    if(gethostname(hostname, max_name_len) == 0)
        sprintf(prompt, "[%s@%s ", pwd->pw_name, hostname);
    else
        sprintf(prompt, "[%s@unknown ", pwd->pw_name);
    length = strlen(prompt);
    if(strlen(pathname) < strlen(pwd->pw_dir) || 
            strncmp(pathname, pwd->pw_dir, strlen(pwd->pw_dir)) != 0)
        sprintf(prompt+length, "%s", pathname);
    else
        sprintf(prompt+length, "~%s", pathname+strlen(pwd->pw_dir));
    length = strlen(prompt);
    sprintf(prompt+length, "]☻ ");
    return;
}
