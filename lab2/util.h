#ifndef _UTIL_H
#define _UTIL_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

const char *noMsgError = "should've put msg";

void err(char *msg) {
    // return with err msg, **for debug only**
    if (msg == NULL) write(STDERR_FILENO, noMsgError, strlen(noMsgError));
    else write(STDERR_FILENO, msg, strlen(msg));
    exit(1);
}

void msg(char *msg) {
    // show msg, **for debug only**
    if (msg == NULL) err("should've put err msg");
    else write(STDERR_FILENO, msg, strlen(msg));
}

char *strcopy(char *b) {
    int len = strlen(b);
    char *res = (char*)malloc(sizeof(char) * (len+1));
    memcpy(res, b, sizeof(char) * len);
    return res;
}

#endif