#ifndef _UTIL_H
#define _UTIL_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

extern int ORIGIN_STDOUT_FILENO;

#define allocate(type, size) (type*)malloc(sizeof(type) * size)
#define match(a, b) (strcmp(a, b) == 0)
char **parse(char *str, const char *delim, int allowEmpty);
void dbg(char *s);
void trim(char *s);

#define setgreen fprintf(stderr, "\033[32;1m");
#define setwhite fprintf(stderr, "\033[39;0m");
#define setred fprintf(stderr, "\033[31;1m")

#define setindent(x) {for (int i=0; i<x; ++i) fprintf(stderr, " ");}

#endif