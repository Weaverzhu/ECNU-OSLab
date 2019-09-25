#ifndef _CMD_H
#define _CMD_H

#include "mypipe.h"
#include "fcntl.h"

////////////////definition////////////////////

typedef struct Cmd {
    int argc, bg;
    char **argv;
    Pipe *pread, *pwrite;
    char *redirect;
} Cmd;

typedef struct CmdList {
    Cmd *data;
    struct CmdList *next;
} CmdList;

CmdList *bgList = NULL;


////////////////////function////////////////////////
int runWithPipe(Cmd *c, pid_t cpid);
Cmd* newCommand(char *cmdstr);
CmdList *newCommandList(char *line);
int tryBuiltIn(Cmd *c);
CmdList *insertCmd(CmdList *head, Cmd *c);
CmdList *delCmd(CmdList *head, Cmd *c);
int runCmdListWithPipe(CmdList *head);
int runCmdWithPipe(Cmd *c, Pipe *pr, Pipe *pw, pid_t cpid);
int isBuiltIn(char *cmd);
#endif