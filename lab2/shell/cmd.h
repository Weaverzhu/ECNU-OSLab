#ifndef _CMD_H_
#define _CMD_H_

#include "pipe.h"
#include "config.h"

typedef struct Cmd {
    char **argv;
    Pipe *read, *write;
    int bgpid;
} Cmd;

typedef struct CmdList {
    struct CmdList *next;
    Cmd *data;
} CmdList;


extern int isBackground;

CmdList *insertCmd(CmdList *head, Cmd *c);
CmdList *parseLine(char *cmdline);
Cmd *newCommand(char *cmdstr);
int runBackgourndCmd(Cmd *c);
int runCmdWithPipe(CmdList *head);
int tryBuiltIn(Cmd *c);
int runCommand(Cmd *c);
int isBuiltIn(Cmd *c);
int tryRedirect(Cmd *c);
#endif