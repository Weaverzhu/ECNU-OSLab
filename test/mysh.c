#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define N 10000

char buf[N];


int
main(int argc, char *argv[])
{
    pid_t cpid;

    int fd1[2];
    pipe(fd1);
    if ((cpid = fork()) == 0) {
        close(fd1[0]);
        dup2(fd1[1], STDOUT_FILENO);
        char *argv[10] = {
            "ls"
        };
        execvp(argv[0], argv);
        puts("FUCK");
    } else {
        close(fd1[1]);
    }

    if ((cpid = fork()) == 0) {
        dup2(fd1[0], STDIN_FILENO);
        char *argv[10] = {
            "wc"
        };
        execvp(argv[0], argv);
        puts("FUCK");
    } else {
        waitpid(cpid, NULL, 0);
        return 0;
    }


    return 0;
}
