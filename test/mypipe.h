#ifndef _PIPE_H
#define _PIPE_H
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define PIPESIZE 512

typedef struct Pipe {
    int lock, pipefd[2];
    char data[PIPESIZE];
    int nread, nwrite, isReadOpen, isWriteOpen;
} Pipe;



Pipe *newPipe();
int configurePipe(Pipe *p, pid_t cpid, int isFatherRead);
int pipeRead(Pipe *p);
int pipeWrite(Pipe *p, char *buf);
#endif