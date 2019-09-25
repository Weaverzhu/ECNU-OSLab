#include "mypipe.h"
Pipe* newPipe() {
    Pipe *res = (Pipe*)malloc(sizeof(Pipe));
    res->lock = res->nread = res->nwrite;
    pipe(res->pipefd);
    return res;
}

int configurePipe(Pipe *p, pid_t cpid, int isFatherRead) {
    if (cpid == 0) {
        if (isFatherRead) {
            close(p->pipefd[0]);
            p->isReadOpen = 0;
            p->isWriteOpen = 1;
        } else {
            close(p->pipefd[1]);
            p->isReadOpen = 1;
            p->isWriteOpen = 0;
        }
    } else {
        if (isFatherRead) {
            close(p->pipefd[1]);
            p->isReadOpen = 1;
            p->isWriteOpen = 0;
        } else {
            close(p->pipefd[0]);
            p->isReadOpen = 0;
            p->isWriteOpen = 1;
        }
    }
}

int pipeRead(Pipe *p, char *buf) {
    return read(p->pipefd[0], buf, PIPESIZE);
}

int pipeWrite(Pipe *p, char *buf) {
    return write(p->pipefd[1], buf, PIPESIZE);
}


