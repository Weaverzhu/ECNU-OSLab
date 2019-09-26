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
        read(STDIN_FILENO, cmdline, SIZE);
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
