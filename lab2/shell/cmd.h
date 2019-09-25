#ifndef _CMD_H
#define _CMD_H

#include "pipe.h"

typedef struct Cmd {
    char **argv;
    Pipe *read, *write;
} Cmd;

#endif