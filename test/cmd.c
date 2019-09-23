#include "cmd.h"
#include "config.h"
#include "util.h"


Cmd *newCommand(char *cmdstr) {
    Cmd *res = (Cmd*)malloc(sizeof(Cmd));
    res->argv = parse(cmdstr, " ");
    res->argc = 0;
    for (int i=1; res->argv[i] != NULL; ++i)
        ++res->argc;
    if (res->argc && strcmp(res->argv[res->argc] == "&") == 0) {
        res->isBackground = 1;
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

CmdList *newCommandList(char *line) {
    char **cmdstrs = parse(line, "|");
    CmdList *head = NULL;
    for (int i=0; cmdstrs[i]!=NULL; ++i) {
        Cmd *c = newCommand(cmdstrs[i]);
        head = insertCmd(head, c);
    }
    return head;
}

int runCmdListWithPipe(CmdList *head) {
    
}