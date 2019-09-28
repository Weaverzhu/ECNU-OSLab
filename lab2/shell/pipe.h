#ifndef _MYPIPE_H_
#define _MYPIPE_H_
#include <sys/types.h>
#include "config.h"
#include "util.h"

typedef struct Pipe {
    int pipefd[2];
} Pipe;

Pipe *newPipe();

#endif