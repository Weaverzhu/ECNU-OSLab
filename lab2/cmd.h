#ifndef _CMD_H
#define _CMD_H
#include "config.h"
#include "util.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>


typedef struct Cmd {
    int argc;
    char **argv;
} Cmd; 

int runbuiltIn(Cmd* c) {
    if (strcmp(c->argv[0], "exit") == 0) {
        if (c->argc != 0) {
            fprintf(stderr, "mysh: exit: %s\n", ERR_TOO_MANY_ARGS);
            return 1;
        }
        else exit(0);
    } else if (strcmp(c->argv[0], "pwd") == 0) {
        if (c->argc != 0) {
            fprintf(stderr, "mysh: %s\n", error_message);
            return -1;
        }
        char *buf = NULL;
        buf = getcwd(buf, CARRIGE_SIZE);
        if (buf == NULL) {
            fprintf(stderr, "mysh: %s\n", error_message);
            return -1;
        }
        printf("%s\n", buf);
        return 1;
    } else if (strcmp(c->argv[0], "cd") == 0) {
        if (c->argc != 0) {
            fprintf(stderr, "mysh: %s\n", error_message);
            return -1;
        }
        else if (c->argc == 1) {
            int ret = chdir(c->argv[1]);
            #ifdef DEBUG
            printf("ret = %d\n", ret);
            #endif
            if (ret) {
                fprintf(stderr, "mysh: cd: %s: %s\n", c->argv[1], ERR_NO_FD);
                return -1;
            }
        } else {
            chdir(getenv("HOME"));
        }
        return 1;
    }

    return 0;
}

char **parseCmd(char *cmdName) {
    
    

    static char buf[CARRIGE_SIZE][CARRIGE_SIZE+1];
    int argc = 0;
    char *localcmd = strdup(cmdName);
    char *token, *delim = " \n", *saveptr;
    for (char *s=localcmd; ; s=NULL) {
        token = strtok_r(s, delim, &saveptr);
        if (token == NULL) break;
        if (strlen(token) == 0) continue;
        
        memcpy(buf[argc++], token, sizeof(char) * (strlen(token) + 1));
    }
    char **argv = (char**)malloc(sizeof(char*) * (argc+1));
    for (int i=0; i<argc; ++i) {
        argv[i] = strdup(buf[i]);
    }
    argv[argc] = NULL;

    free(localcmd);
    return argv;
}


Cmd* newCommand(char *cmdName) {    
    
    Cmd *res = (Cmd*)malloc(sizeof(Cmd));
    res->argv = parseCmd(cmdName);
    res->argc = 0;
    #ifdef DEBUG
   
    #endif
    for (int i=0;; ++i) {
        if (res->argv[i] == NULL) {
            res->argc = i-1;
            break;
        }
    }

    for (int i=1; i<=res->argc; ++i) {
        if (strcmp(res->argv[i], ">") == 0) {
            if (i == res->argc - 1) {
                outfd = open(res->argv[res->argc], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                if (outfd == -1) fprintf(stderr, "mysh: %s\n", ERR_NO_FD);
                dup2(outfd, STDOUT_FILENO);
                res->argc -= 2;
                res->argv[res->argc+1] = NULL;
            } else {
                fprintf(stderr, "mysh: %s\n", error_message);
                return NULL;
            }
        }
    }

    #ifdef DEBUG
        fprintf(stderr, "argc: %d\n", res->argc);

        for (int i=1; i<=res->argc; ++i) {
            fprintf(stderr, "%d %s\n", i, res->argv[i]);
        }
    #endif

    return res;
}


int run(Cmd *c) {
    if (c == NULL) return -1;
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