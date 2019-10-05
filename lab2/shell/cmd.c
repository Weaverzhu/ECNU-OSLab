#include "cmd.h"
#include "util.h"
#include "unistd.h"
#include "mysh.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "config.h"
#include <assert.h>

CmdList *insertCmd(CmdList *head, Cmd *c){
    CmdList *res = allocate(CmdList, 1); res->data = c; res->next = NULL;
    res->pleft = res->pright = NULL;
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
    #ifdef DEBUG
    setgreen;
    fprintf(stderr, "start to add new command from str %s\n", cmdstr);
    setwhite;
    #endif

    Cmd *res = allocate(Cmd, 1);
    res->argv = parse(cmdstr, " \n", 1);
    res->read = res->write = NULL;
    res->bgpid = 0;
    return res;
}


int isBackground;


void replacechr(char *s, int l, int r, char a, char b) {
    for (int i=l; i<=r; ++i) {
        if (s[i] == a) s[i] = b;
    }
}


CmdList *parseLine(char *cmdline) {
    isBackground = 0;
    trim(cmdline);
    #ifdef DEBUG
    dbg("after trim:");
    setred;
    fprintf(stderr, "after trim, cmd is %s\n", cmdline);
    setwhite;
    #endif

    char *note = strchr(cmdline, '&');
    if (note == NULL || note == (cmdline + strlen(cmdline))) { // & not found
        dbg("& not found");
        isBackground = 0;
    } else if (note != NULL && (cmdline + strlen(cmdline) - note) > 1) { // & not at the end of cmd
        return NULL;
    } else if (note != NULL) { // set background
        dbg("this cmd is background cmd");
        *note = 0;
        isBackground = 1;
    }

    for (int i=0, j; cmdline[i]!=0; ++i) {
        if (cmdline[i] == '\"') {
            j = i+1;
            while (cmdline[j]!=0 && cmdline[j]!='\"') ++j;
            if (cmdline[j]!='\"') return NULL;

            replacechr(cmdline, i, j, ' ', 6);
            replacechr(cmdline, i, j, '|', 7);
            replacechr(cmdline, i, j, '\"', ' ');
            i = j;
        }
    }


    char **cmdstr = parse(cmdline, "|\n", 0);
    if (cmdstr == NULL) return NULL;
    
    CmdList *head = NULL;
    for (int i=0; cmdstr[i]!=NULL; ++i){
        int len = strlen(cmdstr[i]);
        replacechr(cmdstr[i], 0, len-1, 6, ' ');
        replacechr(cmdstr[i], 0, len-1, 7, '|');


        Cmd *newcmd = newCommand(cmdstr[i]);
        head = insertCmd(head, newcmd);
    }
    return head;
}

int tryBuiltIn(Cmd *c, char *output) {
    dbg("trying builtin");
    
    char *cmdname = c->argv[0];
    #ifdef DEBUG
    setgreen;
    fprintf(stderr, "cmdname = %s\n", cmdname);
    setwhite;
    #endif


    if (match(cmdname, "exit")) {
        dbg("exit here");
        if (c->argv[1] != NULL) return -1;
        exit(0);
        dbg("exit failed");
    } else if (match(cmdname, "pwd")) {
        if (c->argv[1] != NULL) return -1;
        char buf[SIZE];
        memset(buf, 0, sizeof buf);
        getcwd(buf, SIZE);
        buf[strlen(buf)] = '\n';
        if (buf == NULL) return -1;
        if (output == NULL) {
        } else {
            strcpy(output, buf);
        }
        return 1;
    } else if (match(cmdname, "cd")) {
        char *buf;
        if (c->argv[1] == NULL) buf = getenv("HOME");
        else buf = c->argv[1];
        int ret = chdir(buf);
        if (ret == -1) return -1;
        else return 1;
    } else if (match(cmdname, "wait")) {
        if (c->argv[1] != NULL) return -1;
        for (CmdList *t=bghead; t!=NULL; t=t->next) {
            pid_t cpid = c->bgpid;
            int ret = waitpid(cpid, NULL, 0);
            if (ret == -1) return -1;
        }
        return 1;
    }

    return 0;
}


int isBuiltIn(Cmd *c) {
    char *cmdname = c->argv[0];
    return match(cmdname, "exit") || match(cmdname, "cd") || match(cmdname, "wait") || match(cmdname, "pwd");
}


int tryRedirect(Cmd *c) {
    int argc = 0;
    int pos = -1;
    for (; c->argv[argc]!=NULL; ++argc) {
        if (match(c->argv[argc], ">")) {
            if (~pos) return -1; // two or more >
            pos = argc;
        }
    }
    if (~pos) {

        if (pos != argc-2) 
            return -1; // > {file} is not at the end of file
        int fd = open(c->argv[argc-1], WRITE_FILE_MODE);

        if (DBG_MODE & REDIRECT_FLG)
            fprintf(stderr, "fd=%d\n", fd);

        if (fd < 0) return -1; // file open error
        dup2(fd, STDOUT_FILENO);
        free(c->argv[pos]); c->argv[pos] = NULL;
        return 1;
    }
    return 0;
}

int runCmdWithPipe(CmdList *head) {
    Pipe *p;
    int id = 0;

    int pids[500], pcnt = 0;
    char *buf = NULL;

    for (CmdList *t=head; t!=NULL; t=t->next) {
        dbg("in main loop");
        p = NULL;
        if (t->next != NULL) {
            p = newPipe();
            t->pright = t->next->pleft = p;
        } else {
            t->pright = NULL;
        }

        Cmd *c = t->data;
      
        if (isBuiltIn(c)) {
            if (t->pleft != NULL) {
                dup2(t->pleft->pipefd[0], STDIN_FILENO);
            }
            int ret = tryRedirect(c);
            if (ret == -1) return -1;
            buf = allocate(char, SIZE);
            ret = tryBuiltIn(c, buf);
            if (ret == -1) return -1;
        } else {
            pid_t cpid = fork();
            if (cpid == -1) return -1;
            if (cpid == 0) {
                if (t->pleft != NULL) {
                    if (buf != NULL) {
                        closeWrite(t->pleft);
                    }
                    dup2(t->pleft->pipefd[0], STDIN_FILENO);
                }
                if (t->pright != NULL) {
                    closeRead(t->pright);
                    dup2(t->pright->pipefd[1], STDOUT_FILENO);
                }


                int ret = tryRedirect(c);
                if (ret == -1) return -1;
                
                ret = execvp(c->argv[0], c->argv);
                REPORT_ERR;
                if (t->pright != NULL) closeWrite(t->pright);
                if (t->pleft != NULL) closeRead(t->pleft);
                exit(-1);
            } else {
                if (buf != NULL) {
                    closeRead(t->pleft);
                    write(t->pleft->pipefd[1], buf, strlen(buf));
                    free(buf); buf = NULL;
                    closeWrite(t->pleft);
                }

                if (t->pright != NULL)
                    closeWrite(t->pright);
                pids[pcnt++] = cpid;
            }
        }
        dup2(ORIGIN_STDIN_FILENO, STDIN_FILENO);
        dup2(ORIGIN_STDOUT_FILENO, STDOUT_FILENO);
        ++id;
        dbg("id increased");
    }
    if (!isBackground) {
        for (int i=0; i<pcnt; ++i) {
            waitpid(pids[i], NULL, 0);
        }
    }
    if (buf != NULL) {
        
        write(STDOUT_FILENO, buf, strlen(buf));
        buf[strlen(buf)] = '\n';
        free(buf); buf = NULL;
        
    }
    dbg("return 0 correctly");
    return 0;
}

void outputCmdList(CmdList *head) {
    const int ind = 1;
    for (CmdList *t=head; t!=NULL; t=t->next) {
        // indent(ind);
        setindent(ind);
        outputcmd(t->data);
    }
}

void outputcmd(Cmd *c) {
    #ifdef DEBUG
    setgreen;
    fprintf(stderr, "output Cmd %p:\n", c);
    setred;
    for (int i=0; c->argv[i]!=NULL; ++i)
        fprintf(stderr, "%s ", c->argv[i]);
    fprintf(stderr, "\n");
    setwhite;
    #endif
}