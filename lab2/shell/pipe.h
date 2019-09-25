#ifndef _PIPE_H
#define _PIPE_H

typedef struct Pipe {
    int pipefd[2];
} Pipe;

void configurePipe(Pipe *p, int fatherRead);

#endif