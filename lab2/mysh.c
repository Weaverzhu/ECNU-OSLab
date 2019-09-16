#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#define CARRIGE_SIZE 512

// ===============utils==================

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



// ===============main===================

typedef struct Cmd {
    int argc;
    char **argv;
} Cmd; 




Cmd* newCommand(char *cmdName) {
    // quote isn't allowed
    static char buf[CARRIGE_SIZE][CARRIGE_SIZE+1];


    char *localcmd = strcopy(cmdName);
    Cmd *res = (Cmd*)malloc(sizeof(Cmd));
    char *token, *delim = " \n", *saveptr;
    res->argc = 0;
    printf("%s %s\n", localcmd, delim);
    for (char *s=localcmd; ; s=NULL) {
        token = strtok_r(s, delim, &saveptr);
        if (token == NULL) break;
        memcpy(buf[res->argc++], token, sizeof(char) * (strlen(token) + 1));
    }
    res->argv = (char**)malloc(sizeof(char*) * (res->argc+1));
    
    for (int i=0; i<res->argc; ++i) {
        res->argv[i] = strcopy(buf[i]);
    }
    res->argv[res->argc] = NULL;
    
    return res;
}

int run(Cmd *c) {
    return execvp(c->argv[0], c->argv);
}

// int runtest(Cmd *c) {
//     return system(c);
// }

void testargv(char *cmd) {
    char *token, *localcmd = strcopy(cmd), *saveptr;
    const char* delim = " \n";

    for (char *s=localcmd;; s=NULL) {
        token = strtok_r(s, delim, &saveptr);
        if (token==NULL) break;
        printf("%s\n", token);
    }
}

int main(int argc, char const *argv[])
{
    Cmd *cmd = newCommand("ls -l");
    run(cmd);
    return 0;
}


