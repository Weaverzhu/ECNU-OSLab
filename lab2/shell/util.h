#ifndef _UTIL_H
#define _UTIL_H

#include <string.h>
#include <stdlib.h>

#define allocate(type, size) (type)malloc(sizeof(type) * size)

char **parse(char *str, const char *delim);


#endif