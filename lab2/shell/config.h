#ifndef _CONF_H
#define _CONF_H
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SIZE 520
#define PIPE_SIZE 5120

extern const char error_message[];
extern const char HEADER[];
extern const int DBG_MODE;
extern int BATCH_MODE;


#define REPORT_ERR write(STDERR_FILENO, error_message, strlen(error_message));
#define PRINT_HEADER { if (BATCH_MODE == 0) write(STDOUT_FILENO, HEADER, strlen(HEADER)); }
#define WRITE_FILE_MODE O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU
// #define DEBUG
#define REDIRECT_FLG 2
#define _GNU_SOURCE

#define TEST



#endif
