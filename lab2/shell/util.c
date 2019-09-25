#include "util.h"
#include "config.h"

char **parse(char *str, const char *delim) {
    char *localstr = strdup(str), *token, *saveptr;
    static char buf[SIZE][SIZE];
    int count = 0;
    for (char *s=localstr; ; s=NULL) {
        token = strtok_r(s, delim, &saveptr);
        strcpy(buf[count++], token);
    }
    char **res = allocate(char**, count+1);
    for (int i=0; i<count; ++i) {
        res[i] = strdup(buf[i]);
    }
    res[count] = NULL;
    return res;
}