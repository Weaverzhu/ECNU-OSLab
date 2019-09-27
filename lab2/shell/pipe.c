#include "pipe.h"
#include "util.h"
#include "config.h"
#include <unistd.h>

Pipe *newPipe() { // return a new pipe pointer
    Pipe *res = allocate(Pipe, 1);
    return res;
}

int configurePipe(Pipe *p, int fatherRead, pid_t cpid) { // configure pipe for fp and chdp, 0 for suc, -1 for error
    pipe(p->pipefd);
    int openid = 0;
    if (fatherRead) openid = 0;
    else openid = 1;
    if (cpid == 0) openid ^= 1;
    return close(p->pipefd[openid ^ 1]);
}

int pipeRead(Pipe *p, char *buf) { // return bytes read from pipe, 0 for EOF, -1 for error
    return read(p->pipefd[0], buf, PIPE_SIZE);
}

int pipeWrite(Pipe *p, char *buf) {
    return write(p->pipefd[1], buf, strlen(buf));
}

// int passOnPipe(Pipe *left, Pipe *right) { // pass data from a pipe to another, 0 for suc, -1 for error
//     static char buf[PIPE_SIZE];
//     int ret = pipeRead(left, buf);
//     if (ret == -1) return -1;
//     ret = pipeWrite(right, buf);
//     if (ret == -1) return -1;
//     return 0;
// }