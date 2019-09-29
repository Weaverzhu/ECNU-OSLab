#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


int main(int argc, char const *argv[])
{
    int fd[2]; pipe(fd);
    int cpid = fork();
    if (cpid == 0) {
        char buf[] = "hello";
        close(fd[0]);
        write(fd[1], buf, strlen(buf));
        exit(0);
    } else {
        close(fd[1]);
        char buf[100];
        memset(buf, 0, sizeof buf);
        read(fd[0], buf, 60);
        write(STDOUT_FILENO, buf, 100);
    }
    return 0;
}
