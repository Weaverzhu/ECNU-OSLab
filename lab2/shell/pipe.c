#include "pipe.h"
#include "util.h"
#include "config.h"
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>

Pipe *newPipe() { // return a new pipe pointer
    Pipe *res = allocate(Pipe, 1);
    pipe(res->pipefd);
    return res;
}


// int passOnPipe(Pipe *left, Pipe *right) { // pass data from a pipe to another, 0 for suc, -1 for error
//     static char buf[PIPE_SIZE];
//     int ret = pipeRead(left, buf);
//     if (ret == -1) return -1;
//     ret = pipeWrite(right, buf);
//     if (ret == -1) return -1;
//     return 0;
// }