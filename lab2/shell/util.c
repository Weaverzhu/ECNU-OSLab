#include "util.h"
#include "config.h"

int ORIGIN_STDOUT_FILENO, ORIGIN_STDIN_FILENO;

char **parse(char *str, const char *delim, int allowEmpty) {
    dbg("parse started");
    char *localstr = strdup(str), *token, *saveptr;
    static char buf[SIZE >> 1][SIZE];
    int count = 0;
    for (char *s=localstr; ; s=NULL) {
        token = strtok_r(s, delim, &saveptr);
        if (token == NULL) break;
        if (strlen(token) == 0) {
            if (allowEmpty) continue;
            else return NULL;
        }
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

void trim(char *s) {
    char *back = s + strlen(s)-1;
    while (back >= s && (*back == ' ' || *back == '\n')) --back;
    *(back+1) = 0;
}

char *strconcat(char *a, char *b) {
    int len = strlen(a);
    strcpy(a+len, b);
    len += strlen(b);
    a[len] = 0;
    return a;
}

char *compose(char **argv) {
    int len = 0;
    for (int i=0; argv[i]!=NULL; ++i) {
        len += strlen(argv[i]);
    }
    len++;
    char *s = allocate(char, len);
    for (int i=0; argv[i]!=NULL; ++i)
        strconcat(s, argv[i]);
    s[len-1] = 0;
    return s;
}