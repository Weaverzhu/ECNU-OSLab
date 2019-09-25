#include "cmd.h"
#include "config.h"
#include "util.h"
#include <stdio.h>

int isBuiltIn(char *cmd) {
    for (int i=0; BUILTINS[i] != NULL; ++i)
        if (strcmp(BUILTINS[i], cmd) == 0) return 1;
    return 0;
}


int runbuiltIn(Cmd* c) {
    if (strcmp(c->argv[0], "exit") == 0) {
        if (c->argc != 0) {
            // fprintf(stderr, "mysh: exit: %s\n", ERR_TOO_MANY_ARGS);
            PRINT_ERR_MSG;
            return -1;
        }
        else exit(0);
    } else if (strcmp(c->argv[0], "pwd") == 0) {
        if (c->argc != 0) {
            PRINT_ERR_MSG;
            return -1;
        }
        char *buf = NULL;
        buf = getcwd(buf, CARRIGE_SIZE);
        if (buf == NULL) {
            PRINT_ERR_MSG;
            return -1;
        }
        printf("%s\n", buf);
        return 1;
    } else if (strcmp(c->argv[0], "cd") == 0) {
        if (c->argc != 0) {
            PRINT_ERR_MSG;
            return -1;
        }
        else if (c->argc == 1) {
            int ret = chdir(c->argv[1]);
            #ifdef DEBUG
            printf("ret = %d\n", ret);
            #endif
            if (ret) {
                PRINT_ERR_MSG;
                return -1;
            }
        } else {
            chdir(getenv("HOME"));
        }
        return 1;
    } else if (strcmp(c->argv[0], "wait") == 0) {
        if (c->argc != 0) {
            PRINT_ERR_MSG;
            return -1;
        } else {
            for (int i=0; i<childNum; ++i) {
                int status, cpid = wait(&status);
                if (cpid == -1) {
                    PRINT_ERR_MSG;
                    return -1;
                }
            }
            childNum = 0;
        }
    }
    return 0;
}

Cmd *newCommand(char *cmdstr) {
    Cmd *res = (Cmd*)malloc(sizeof(Cmd));
    res->argv = parse(cmdstr, " ");
    res->argc = 0;
    for (int i=1; res->argv[i] != NULL; ++i)
        ++res->argc;
    if (res->argc && strcmp(res->argv[res->argc] == "&") == 0) {
        res->bg = 1;
        res->argv[res->argc--] = NULL;
    }
    if (res->argc >= 2 && strcmp(res->argv[res->argc-1] == ">")) {
        res->redirect = strdup(res->argv[res->argc-1]);
        res->argc -= 2;
        res->argv[res->argc+1] = NULL;
    } else res->redirect = NULL;
    res->pread = res->pwrite = NULL;
    return res;
}

CmdList *insertCmd(CmdList *head, Cmd *c) {
    CmdList *res = (CmdList*)malloc(sizeof(CmdList));
    res->data = c;
    res->next = NULL;
    if (head == NULL) {
        return res;
    } else {
        while (head->next != NULL) head = head->next;
        head->next = res;
        return head;
    }
}

CmdList *delCmd(CmdList *head, Cmd *c) {
    if (c == head) return head->next;
    else {
        CmdList *t;
        for (t=head; t->next!=c && t->next!=NULL; t=t->next);
        if (t->next == c) {
            t->next = t->next->next;
        }
        return head;
    }
}

CmdList *newCommandList(char *line) {
    char **cmdstrs = parse(line, "|");
    CmdList *head = NULL;
    for (int i=0; cmdstrs[i]!=NULL; ++i) {
        Cmd *c = newCommand(cmdstrs[i]);
        head = insertCmd(head, c);
    }
    return head;
}

int runCmdWithPipe(Cmd *c, Pipe *pr, Pipe *pw, pid_t cpid) {
    if (cpid == 0) { // child process
        if (pr != NULL) dup2(pr->pipefd[0], STDIN_FILENO);
        if (pw != NULL) dup2(pw->pipefd[1], STDOUT_FILENO);
        execvp(c->argv[0], c->argv);
    } else {
        if (pr != NULL) {
            pipeRead(pr);
            fprintf(stderr, "dbg:\n %s\n===========\n", pr->data);
        }
        if (pw != NULL && pr != NULL) pipeWrite(pw, pr->data);
        wait(NULL);
    }
}

int runCmdListWithPipe(CmdList *head) {
    static char *buf[SIZE];
    Pipe *pf = NULL, *prec = NULL; 
    for (CmdList *t=head; t!=NULL; t=t->next) {
        Cmd *c = t->data;
        if (t == head) {
            pf == NULL;
        } else {
            pf = prec;
        }
        if (t->next == NULL) {
            prec = NULL;
        } else {
            prec = newPipe();
        }
        buf[0] = 0;
        
        if (isBuiltIn(c->argv[0])) {
            
            if (strcmp(c->argv[0], "exit") == 0) {
                exit(0);
            } else if (strcmp(c->argv[0], "pwd")) {
                getcwd(buf, SIZE);
            } else if (strcmp(c->argv[0], "cd")) {
                if (c->argc == 0) {
                    chdir(getenv("HOME"));
                } else if (c->argc > 1 || chdir(c->argv)) {
                    PRINT_ERR_MSG;
                    return -1;
                }
                getcwd(buf, SIZE);
            } else if (strcmp(c->argv[0], "wait") == 0) {
                
            }
        } else {
            pid_t cpid = fork();
            configurePipe(prec, cpid, 1);
            if (pf != NULL)
                pipeRead(pf, buf);
            runCmdWithPipe(c, pf, prec, cpid);
        }
    }
}