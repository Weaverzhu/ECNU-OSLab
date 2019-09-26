#include "cmd.h"
#include "util.h"
#include "unistd.h"
#include "mysh.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "config.h"

CmdList *insertCmd(CmdList *head, Cmd *c){
    CmdList *res = allocate(CmdList, 1); res->data = c; res->next = NULL;
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

    char **cmdstr = parse(cmdline, "|\n", 0);
    if (cmdstr == NULL) return NULL;
    
    CmdList *head = NULL;
    for (int i=0; cmdstr[i]!=NULL; ++i){
        Cmd *newcmd = newCommand(cmdstr[i]);
        head = insertCmd(head, newcmd);
    }
    return head;
}

int tryBuiltIn(Cmd *c) {
    dbg("trying builtin");
    
    char *cmdname = c->argv[0];
    #ifdef DEBUG
    setgreen;
    fprintf(stderr, "cmdname = %s\n", cmdname);
    setwhite;
    #endif


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
    dbg("tried builtin");
    if (ret == 1) return 0;
    else if (ret == 0) {
        #ifdef DEBUG
            setgreen;
            for (int i=0; c->argv[i]!=NULL; ++i)
                fprintf(stderr, "[%d] |%s|\n", i, c->argv[i]);
            setwhite;
        #endif


        pid_t cpid = fork();
        if (cpid == -1) return -1;
        
        if (cpid == 0) {
            int ret = execvp(c->argv[0], c->argv);
            if (ret == -1) {
                dbg("error in execvp");
               
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
    } else return -1;
    return 0;
}

