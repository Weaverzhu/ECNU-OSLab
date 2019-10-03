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
    // #ifdef TEST
    // test();
    // #endif
    // test();


    dbg("started");

    // int SOURCE_FD = STDOUT_FILENO;
    FILE *f =stdin;

    if (argc > 1) {
        dbg("batch mode on");
        BATCH_MODE = 1;
        f= fopen(argv[1], "r");
        if (f == NULL) {
            REPORT_ERR;
            exit(-1);
        }
    }

    dup2(STDIN_FILENO, ORIGIN_STDIN_FILENO);
    dup2(STDOUT_FILENO, ORIGIN_STDOUT_FILENO); // make backup for stdin, stdout
    
    #ifdef DEBUG
    printf("%d %d\n", BATCH_MODE, SOURCE_FD);

    #endif


    while (1) {
        char cmdline[SIZE];
        PRINT_HEADER; // write header
        memset(cmdline, 0, sizeof cmdline); // clear the str
        // int ret = read(SOURCE_FD, cmdline, SIZE); // read 512 bytes
        char *rets = fgets(cmdline, SIZE, f);
        if (strlen(cmdline) > 512) {
            REPORT_ERR;
            continue;
        }
        if (rets == NULL) break;
        else if (strlen(cmdline) == 1) { // empty file, only a \n left
            dbg("empty command");
            continue;
        }
        int ret = 0;
        // printf("%s\n", cmdline);
        CmdList *head = parseLine(cmdline); // a bunch of cmds

        #ifdef DEBUG
        outputCmdList(head);
        #endif

        // return 0;

        if (head == NULL) { // go wrong
            REPORT_ERR;
            continue;
        }
        
        ret = runCmdWithPipe(head); // show time
        // ret = runtest(head);
        if (ret == -1)  {
            REPORT_ERR;
        }
    }
    return 0; 
}
