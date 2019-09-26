#include "util.h"
#include "config.h"

char **parse(char *str, const char *delim) {
    dbg("parse started");
    char *localstr = strdup(str), *token, *saveptr;
    static char buf[SIZE][SIZE];
    int count = 0;
    for (char *s=localstr; ; s=NULL) {
        token = strtok_r(s, delim, &saveptr);
        if (token == NULL) break;
        if (strlen(token) == 0) continue;
        strcpy(buf[count++], token);
    }
    char **res = allocate(char*, count+1);
    for (int i=0; i<count; ++i) {
        res[i] = strdup(buf[i]);
    }
    res[count] = NULL;
    dbg("parse completed");
    return res;
}

void dbg(char *s) {
    #ifdef DEBUG
    fprintf(stderr, "\033[32;1m%s\033[39;0m\n", s);
    #endif
}
