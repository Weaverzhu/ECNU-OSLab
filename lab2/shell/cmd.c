#include "cmd.h"
#include "util.h"
#include "unistd.h"
#include "mysh.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

CmdList *insertCmd(CmdList *head, Cmd *c){
    CmdList *res = allocate(CmdList*, 1); res->data = c; res->next = NULL;
    if (head == NULL) {
        return res;
    } else {
        CmdList *t;
        for (t=head; t->next!=NULL; t=t->next);
        t->next = res;
    }
    return head;
}

Cmd *newCommand(char *cmdstr) {
    Cmd *res = allocate(Cmd*, 1);
    res->argv = parse(cmdstr, " ");
    res->read = res->write = NULL;
    res->bgpid = 0;
    return res;
}


int isBackground;
CmdList *parseLine(char *cmdline) {
    isBackground = 0;
    char **cmdstr = parse(cmdline, "|\n");
    for (int i=0; cmdstr[i]!=NULL; ++i) {
        if (match(cmdstr[i], "&")) {
            if (cmdstr[i+1] == NULL) {
                free(cmdstr[i]);
                isBackground = 1;
                break;
            } else {
                return NULL;
            }
        }
    }
    CmdList *head = NULL;;
    for (int i=0; cmdstr[i]!=NULL; ++i){
        Cmd *newcmd = newCommand(cmdstr[i]);
        head = insertCmd(head, newcmd);
    }
    return head;
}

int tryBuiltIn(Cmd *c) {
    char *cmdname = c->argv[0];
    if (match(cmdname, "exit")) {
        exit(0);
    } else if (match(cmdname, "pwd")) {
        static char *buf;
        buf = getcwd(buf, 0);
        if (buf == NULL) return -1;
        write(STDOUT_FILENO, buf, strlen(buf));
        free(buf);
        return 1;
    } else if (match(cmdname, "cd")) {
        static char *buf;
        if (c->argv[1] == NULL) buf = getcwd(buf, 0);
        else buf = c->argv[1];
        int ret = chdir(buf);
        if (ret == -1) return -1;
        else return 1;
    } else if (match(cmdname, "wait")) {
        for (CmdList *t=bghead; t!=NULL; t=t->next) {
            pid_t cpid = c->bgpid;
            int ret = waitpid(cpid, NULL, 0);
            if (ret == -1) return -1;
        }
        return 1;
    }

    return 0;
}

int runCommand(Cmd *c) {
    int ret = tryBuiltIn(c);
    if (ret) return 0;
    else {
        pid_t cpid = fork();
        if (cpid == -1) return -1;
        
        if (cpid == 0) {
            int ret = execvp(c->argv[0], c->argv);
            if (ret == -1) {
                REPORT_ERR;
                exit(-1);
            }
        } else {
            if (isBackground) {
                c->bgpid = cpid;
                insertCmd(bghead, c);
            } else {
                waitpid(cpid, NULL, 0);
            }
        }
    }
    return 0;
}

