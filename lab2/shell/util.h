#ifndef _UTIL_H
#define _UTIL_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define allocate(type, size) (type*)malloc(sizeof(type) * size)
#define match(a, b) (strcmp(a, b) == 0)
char **parse(char *str, const char *delim);
void dbg(char *s);

#define setgreen fprintf(stderr, "\033[32;1m");
#define setwhite fprintf(stderr, "\033[39;0m");

#endif