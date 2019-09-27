#ifndef _PIPE_H
#define _PIPE_H
#include <sys/types.h>

typedef struct Pipe {
    int pipefd[2];
} Pipe;

int configurePipe(Pipe *p, int fatherRead, pid_t cpid);
Pipe *newPipe();
// int passOnPipe(Pipe *left, Pipe *right);
int pipeRead(Pipe *p, char *buf);
int pipeWrite(Pipe *p, char *buf);

#endif