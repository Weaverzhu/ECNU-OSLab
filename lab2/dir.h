#ifndef _DIR_H
#define _DIR_H
#include "config.h"
#include "util.h"
typedef struct {
    char **dirs;
} Dir;

char **parseDir(char *path) {
    char *localcmd = strdup(path);
    static char buf[C_SIZE][C_SIZE];
    int c = 0;
    char *token, *saveptr;
    for (char *s=localcmd; ; s=NULL) {
        token = strtok_r(s, "/", &saveptr);
        if (token == NULL) break;
        if (strlen(token) == 0) continue;
        strcpy(buf[c++], token);
    }
    char **res = (char**)malloc(sizeof(char*) * (c));
    for (int i=0; i<c; ++i)
        strcpy(res[i], buf[i]);
    free(localcmd);
    return res;
}

Dir *newDir(char *path) {
    Dir *res = (Dir*)malloc(sizeof(Dir));
    res->dirs = parseDir(path);
    return res;
}

char *getFullPath(char *buf, Dir *newDir) {
    
}

#endif