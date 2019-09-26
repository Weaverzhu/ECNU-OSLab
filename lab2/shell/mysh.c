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

    dup2(STDOUT_FILENO, ORIGIN_STDOUT_FILENO); // make backup for stdout

    while (1) {
        static char cmdline[SIZE];
        PRINT_HEADER; // write header
        memset(cmdline, 0, sizeof cmdline); // clear the str
        int ret = read(STDIN_FILENO, cmdline, SIZE); // read 512 bytes
        if (ret == -1) { // go wrong
            REPORT_ERR;
            continue;
        } else if (ret == 1) { // empty file, only a \n left
            dbg("empty command");
            continue;
        }
        CmdList *head = parseLine(cmdline); // a bunch of cmds
        if (head == NULL) { // go wrong
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
        if (c == NULL) { // go wrong
            dbg("new command is null");
            REPORT_ERR;
        } else { // it's show time
            int ret = runCommand(c);
            if (ret == -1) {
                REPORT_ERR;
            }
        }
        free(c); // clean the things up
    }
    return 0; 
}
