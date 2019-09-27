#ifndef _MYPIPE_H_
#define _MYPIPE_H_
#include <sys/types.h>
#include "config.h"
#include "util.h"

typedef struct Pipe {
    int pipefd[2];
} Pipe;

Pipe *newPipe();
int configurePipe(Pipe *p, int fatherRead, pid_t cpid);

// int passOnPipe(Pipe *left, Pipe *right);
int pipeRead(Pipe *p, char *buf);
int pipeWrite(Pipe *p, char *buf);

#endif