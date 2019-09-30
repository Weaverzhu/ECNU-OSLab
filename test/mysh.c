#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define N 10000

#define SIZE 512

char buf[N];


int
main(int argc, char *argv[])
{
    pid_t cpid;

    int fd1[2];
    pipe(fd1);
    char buf[SIZE] = "fuck";

    if ((cpid = fork()) == 0) {
        close(fd1[1]);
        dup2(fd1[0], STDIN_FILENO);
        char *argv[10] = {
            "wc"
        };
        execvp(argv[0], argv);
    } else {
        close(fd1[0]);
        write(fd1[1], buf, strlen(buf));
        close(fd1[1]);
        waitpid(cpid, NULL, 0);
        return 0;
    }


    return 0;
}
