#ifndef _CONF_H
#define _CONF_H
#include <string.h>

#define SIZE 512
#define PIPE_SIZE 512

extern const char error_message[];
extern const char HEADER[];
#define REPORT_ERR write(STDERR_FILENO, error_message, strlen(error_message));
#define PRINT_HEADER write(STDOUT_FILENO, HEADER, strlen(HEADER));
#define DEBUG

#endif