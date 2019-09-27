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
        if (fd < 0) return -1; // file open error
        dup2(fd, STDOUT_FILENO);
        return 1;
    }
    return 0;
}

int runCmdWithPipe(CmdList *head) {
    
    Pipe *last = NULL, *next = NULL;
    pid_t cpid;
    for (CmdList *t=head; t!=NULL; t=t->next) { // handle every cmd
        static char buf[SIZE];
        int datafrom;
        if (t == head) {
            // read(STDIN_FILENO, buf, SIZE);
            datafrom = STDIN_FILENO;
        } else {
            // read(next->pipefd[0], buf, SIZE);
            datafrom = next->pipefd[0];
        }
        int ret = read(datafrom, buf, SIZE);
        if (ret == -1) return -1;

        // if (t == head) { // the first cmd
        //     last = NULL;
        // } else {fa
        //     last = next;
        // }
        next = newPipe();
        last = newPipe();

        Cmd *c = t->data;

        if (isBuiltIn(c)) { // handle the cmd in this process when dealing builtin cmds
            dup2(next->pipefd[0], STDOUT_FILENO);
            dup2(last->pipefd[0], STDIN_FILENO);
            int ret = close(next->pipefd[1]);
            if (ret == -1) return -1;
            ret = tryBuiltIn(c);
            if (ret != 1) return -1;

        } else { // configure pipes, this process will read from child process
            cpid = fork();
            int ret = configurePipe(next, 1, cpid);
            if (ret == -1) return -1;
            ret = configurePipe(last, 0, cpid);
            if (ret == -1) return -1;
            if (cpid == 0) { // in child process
                dup2(next->pipefd[1], STDOUT_FILENO); // redirect stdin and stdout to pipe
                dup2(last->pipefd[0], STDIN_FILENO);

                int ret = tryRedirect(c); // try to redirect
                if (ret == -1) return -1;
                ret = execvp(c->argv[0], c->argv); // exec the cmd
                if (ret == -1) exit(-1);
            } else { // in shell process
                write(last->pipefd[1], buf, strlen(buf)); // pass data from last process
            }   
        }

        if (t->next != NULL) { // still cmd left to exec
            waitpid(cpid, NULL, 0); // wait the current cmd stop to get the output
        } else {
            if (isBackground) { // no need to wait for the last cmd to finish
                c->bgpid = cpid;
                insertCmd(bghead, c); // add it to the list
            } else {
                waitpid(cpid, NULL, 0); // wait as usual
            }
        }
    }

    static char buf[SIZE];
    // read(next->pipefd[0], buf, SIZE);
    pipeRead(next, buf);
    dup2(ORIGIN_STDOUT_FILENO, STDOUT_FILENO); // redirect to normal stdout
    
    return 1;
}