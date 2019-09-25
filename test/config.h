#ifndef _CONFIG_H
#define _CONFIG_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#define CARRIGE_SIZE 512
#define SIZE 512
#define C_SIZE 100

#define DEBUG

#define WRITE_FILE_MODE (O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)
#define PRINT_ERR_MSG write(STDERR_FILENO, error_message, strlen(error_message))

const char error_message[30] = "An error has occurred\n";
const char SHELL_HEADER[] = "mysh> ";

// char ERR_WRONG_ARGS[] = "mysh: Wrong input with built-in commands\n";
const char ERR_NO_FD[] = "No such file or directory";
const char ERR_TOO_MANY_ARGS[] = "too many arguments";
const char ERR_CANT_REDIRECT[] = "cannot redirect stdout";
const char ERR_CANT_OPEN_FILE[] = "error in opening file";
int stdoutOriginFd, outfd, stdinOriginFd;
int childNum = 0; // deprecated

const char *BUILTINS[] = {
    "exit",
    "pwd",
    "cd",
    "wait",
    NULL
};
// #define TEST

#endif