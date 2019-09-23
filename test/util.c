#include "util.h"
#include "config.h"
#include "fcntl.h"
#include "string.h"
#include "unistd.h"


char **parseCmd(char *str, const char *delim) {
    char buf[SIZE][SIZE+1];
    int argc = 0;
    char *localstr = strdup(str);
    char *token, *saveptr;
    for (char *s = localstr; ; s=NULL) {
        token = strtok_r(s, delim, &saveptr);
        if (token == NULL) break;
        if (strlen(token) == 0) continue;
        strcpy(buf[argc++], token);
    }
    char **argv = (char**)malloc(sizeof(char*) * (argc+1));
    for (int i=0; i<argc; ++i)
        argv[i] = strdup(buf[i]);
    argv[argc] = NULL;
    free(localstr);
    return argv;
}