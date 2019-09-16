#ifndef _CMD_H
#define _CMD_H
#include "config.h"
#include "util.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>


typedef struct Cmd {
    char **argv;
} Cmd; 

int runbuiltIn(Cmd* c) {
    if (strcmp(c->argv[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(c->argv[0], "pwd") == 0) {
        char *buf = NULL;
        buf = getcwd(buf, CARRIGE_SIZE);
        if (buf == NULL) {
            err(error_message);
        }
        printf("%s\n", buf);
        return 1;
    }

    return 0;
}

char **parseCmd(char *cmdName) {
    static char buf[CARRIGE_SIZE][CARRIGE_SIZE+1];
    int argc = 0;
    char *localcmd = strcopy(cmdName);
    char *token, *delim = " \n", *saveptr;
    for (char *s=localcmd; ; s=NULL) {
        token = strtok_r(s, delim, &saveptr);
        if (token == NULL || strlen(token) == 0) break;
        #ifdef DEBUG
        fprintf(stdout, "%d %s\n", argc, token);
        #endif
        memcpy(buf[argc++], token, sizeof(char) * (strlen(token) + 1));
    }
    char **argv = (char**)malloc(sizeof(char*) * (argc+1));
    for (int i=0; i<argc; ++i) {
        argv[i] = strcopy(buf[i]);
    }
    argv[argc] = NULL;

    free(localcmd);
    return argv;
}


Cmd* newCommand(char *cmdName) {    
    
    Cmd *res = (Cmd*)malloc(sizeof(Cmd));
    res->argv = parseCmd(cmdName);
    return res;
}


int run(Cmd *c) {
    int res = runbuiltIn(c);
    if (res > 0) return 0;

    int pid = fork();
    if (pid == 0) {
        execvp(c->argv[0], c->argv);
        return 0;
    }
    else if (pid > 0) {
        int status;
        wait(&status);
        return 0;
    } else return -1;
}

#endif