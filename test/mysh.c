#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cmdold.h"

#include "mypipe.h"

#define N 10000

char buf[N];

int runCmdWithPipe(Cmd *c, Pipe *pr, Pipe *pw, pid_t cpid) {
    if (cpid == 0) { // child process
        if (pr != NULL) dup2(pr->pipefd[0], STDIN_FILENO);
        if (pw != NULL) dup2(pw->pipefd[1], STDOUT_FILENO);
        execvp(c->argv[0], c->argv);
    } else {
        if (pr != NULL) {
            pipeRead(pr);
            fprintf(stderr, "dbg:\n %s\n===========\n", pr->data);
        }
        if (pw != NULL && pr != NULL) pipeWrite(pw, pr->data);
        wait(NULL);
    }
}

int
main(int argc, char *argv[])
{
    Cmd *c1 = newCommand("find /bin"),
    *c2 = newCommand("grep /bin/b");
    Pipe *p1 = newPipe(), *p2, *p3;

    pid_t cmd1 = fork();
    configurePipe(p1, cmd1, 1);
    runCmdWithPipe(c1, NULL, p1, cmd1);
    p2 = newPipe();
    pid_t cmd2 = fork();
    configurePipe(p2, cmd2, 0);
    runCmdWithPipe(c2, p1, p2, cmd2);
    


    return 0;
}
