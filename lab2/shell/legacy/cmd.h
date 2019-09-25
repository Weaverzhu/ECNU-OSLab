#ifndef _CMD_H
#define _CMD_H
#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>



typedef struct Cmd {
    int argc, backpid;
    char **argv;
} Cmd;

typedef struct CmdList{
    Cmd *data;
    struct CmdList *next;
} CmdList;



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
    } else if (strcmp(c->argv[0], "wait") == 0) {
        if (c->argc != 0) {
            fprintf(stderr, "mysh: %s\n", error_message);
            return -1;
        } else {
            for (int i=0; i<childNum; ++i) {
                int status, cpid = wait(&status);
                if (cpid == -1) {
                    fprintf(stderr, "mysh: %s\n", error_message);
                    return -1;
                }
                fprintf(stderr,"[%d] %d Stopped\n", i+1, cpid);
            }
            childNum = 0;
        }
    }

    return 0;
}

char **parseCmd(char *cmdName, const char *delim) {
    // without quote, space * pipeline

    static char buf[CARRIGE_SIZE][CARRIGE_SIZE+1];
    int argc = 0;
    char *localcmd = strdup(cmdName);
    char *token, *saveptr;
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
    // without quote, space * pipeline
    
    Cmd *res = (Cmd*)malloc(sizeof(Cmd));
    res->argv = parseCmd(cmdName, " \n");
    res->argc = 0;
    
    for (int i=0;; ++i) {
        if (res->argv[i] == NULL) {
            res->argc = i-1;
            break;
        }
    }
/////////////////////////// background //////////////////////////////

    if (strcmp(res->argv[res->argc], "&") == 0) {
        res->backpid = 1;
        res->argv[res->argc--] = NULL;
    }

 ////////////////////////// redirect /////////////////////////////

    for (int i=1; i<=res->argc; ++i) {
        if (strcmp(res->argv[i], ">") == 0) {
            if (i == res->argc - 1) {
                
                #ifdef DEBUG
                    fprintf(stderr, "redirect stdout here, supposed file: %s\n", res->argv[res->argc]);
                #endif
                outfd = open(res->argv[res->argc], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                if (outfd == -1) {
                    fprintf(stderr, "mysh: %s\n", ERR_NO_FD);
                    return NULL;
                }
                dup2(outfd, STDOUT_FILENO);
                res->argc -= 2;
                res->argv[res->argc+1] = NULL;
            } else {
                fprintf(stderr, "mysh: %s\n", error_message);
                return NULL;
            }
        }
    }
/////////////////////////////////////////////////////////////////
    #ifdef DEBUG
        // fprintf(stderr, "argc: %d\n", res->argc);
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

    int pid = fork(); ++childNum;
    //////////////////////////////child process///////////////////////////////////
    if (pid == 0) {
        #ifdef DEBUG
            fprintf(stderr, "in child process, pid=%d\n", getpid());
        #endif
        execvp(c->argv[0], c->argv);
        return 0;
    } else if (pid > 0) {
        int status;
        if (c->backpid == 0) {
            for (int i=0; i<childNum; ++i) {
                int status, cpid = wait(&status);
                if (cpid == -1) {
                    fprintf(stderr, "mysh: %s\n", error_message);
                    return -1;
                }
                fprintf(stderr,"[%d] %d Stopped\n", i+1, cpid);
            }
            #ifdef DEBUG
                fprintf(stderr, "waited here for %d processes\n", childNum);
            #endif
            childNum = 0;
            
        }
        // sleep(1);
        return 0;
    } else {
        fprintf(stderr, "mysh: failed to run child process\n");
    }
    ///////////////////////////////////////////////////////////////////////////
}


CmdList* insertCmd(CmdList* head, Cmd *c) {
    if (head == NULL) {
        head = (CmdList*)malloc(sizeof(CmdList));
        head->data = c;
        head->next = NULL;
        return head;
    }
    else {
        for (CmdList* next=head->next; next!=NULL; next=next->next) {
            next = (CmdList*)malloc(sizeof(CmdList));
            next->data = c;
            next->next = NULL;
        }
        return head;
    }
}

void rmCmdList(CmdList *head) {
    if (head == NULL) return;
    rmCmdList(head->next);
    free(head);
}

#endif