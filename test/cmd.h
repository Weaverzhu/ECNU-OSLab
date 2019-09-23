#ifndef _CMD_H
#define _CMD_H

#include "mypipe.h"
#include "fcntl.h"

typedef struct Cmd {
    int argc, isBackground;
    char **argv;
    Pipe *pread, *pwrite;
    char *redirect;
} Cmd;

typedef struct CmdList {
    Cmd *data;
    struct CmdList *next;
} CmdList;

int runWithPipe(Cmd *c, pid_t cpid);
Cmd* newCommand(char *cmdstr);
CmdList *newCommandList(char *line);
int tryBuiltIn(Cmd *c);
CmdList *insertCmd(CmdList *head, Cmd *c);
int runCmdListWithPipe(CmdList *head);
#endif