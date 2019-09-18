#ifndef _CONFIG_H
#define _CONFIG_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#define CARRIGE_SIZE 512

#define C_SIZE 100

#define DEBUG

#define WRITE_FILE_MODE (O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)

char error_message[30] = "An error has occurred\n";
char SHELL_HEADER[] = "mysh> ";

// char ERR_WRONG_ARGS[] = "mysh: Wrong input with built-in commands\n";
char ERR_NO_FD[] = "No such file or directory";
char ERR_TOO_MANY_ARGS[] = "too many arguments";
char ERR_CANT_REDIRECT[] = "cannot redirect stdout";
char ERR_CANT_OPEN_FILE[] = "error in opening file";
int stdOriginFd, outfd;
int childNum = 0;
// #define TEST

#endif