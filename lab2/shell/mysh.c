#include "config.h"
#include <fcntl.h>
#include <unistd.h>
#include "cmd.h"
#include "mysh.h"
#include <stdlib.h>
#include "util.h"
CmdList *bghead;

int main(int argc, char const *argv[])
{
    dbg("started");
    while (1) {
        static char cmdline[SIZE];
        PRINT_HEADER;
        memset(cmdline, 0, sizeof cmdline);
        int ret = read(STDIN_FILENO, cmdline, SIZE);
        if (ret == -1) {
            REPORT_ERR;
            continue;
        } else if (ret == 1) {
            dbg("empty command");
            continue;
        }
        CmdList *head = parseLine(cmdline);
        if (head == NULL) {
            REPORT_ERR;
            continue;
        }

        #ifdef DEBUG
        dbg("=========\nshow cmds: ");
        setred;
        int count = 0;
        // dbg("pipe cmds:");
        for (CmdList *t=head; t!=NULL; t=t->next) {
            fprintf(stderr, "[%d] %s ...\n", count++, t->data->argv[0]);
        }
        setwhite;
        dbg("=========");
        #endif


        Cmd *c = newCommand(cmdline);
        dbg("newcommand completed");
        if (c == NULL) {
            dbg("new command is null");
            REPORT_ERR;
        } else {
            int ret = runCommand(c);
            if (ret == -1) {
                REPORT_ERR;
            }
        }
        free(c);
    }
    return 0; 
}
