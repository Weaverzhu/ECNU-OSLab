#ifndef _UTIL_H
#define _UTIL_H

#include "cmd.h"

Cmd *cmddup(Cmd *c) {
    Cmd *res;
    res = (Cmd*)malloc(sizeof(Cmd));
    memcpy(res, c, sizeof(Cmd));
    return res;
}


#endif