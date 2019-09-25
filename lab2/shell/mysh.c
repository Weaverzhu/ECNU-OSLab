#include "config.h"
#include <fcntl.h>
#include <unistd.h>
#include "cmd.h"
#include "mysh.h"
CmdList *bghead;

int main(int argc, char const *argv[])
{
    while (1) {
        static char cmdline[SIZE];
        PRINT_HEADER;
        read(STDIN_FILENO, cmdline, SIZE);
        Cmd *c = newCommand(cmdline);
        int ret = runCommand(c);
        if (ret == -1) {
            REPORT_ERR;
        }
        free(c);
    }
    return 0; 
}
