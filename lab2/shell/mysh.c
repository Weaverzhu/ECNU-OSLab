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

    dup2(STDIN_FILENO, ORIGIN_STDIN_FILENO);
    dup2(STDOUT_FILENO, ORIGIN_STDOUT_FILENO); // make backup for stdin, stdout

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

        #ifdef DEBUG
        outputCmdList(head);
        #endif

        if (head == NULL) { // go wrong
            REPORT_ERR;
            continue;
        }
        
        ret = runCmdWithPipe(head); // show time
        if (ret == -1)  {
            REPORT_ERR;
        }

        // free(c); // clean the things up
    }
    return 0; 
}
