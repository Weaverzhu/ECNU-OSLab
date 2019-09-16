#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
const int CARRIGE_SIZE = 512;

// ===============utils==================

const char *noMsgError = "should've put msg";

void err(char *msg) {
    // return with err msg, **for debug only**
    if (msg == NULL) write(STDERR_FILENO, noMsgError, strlen(noMsgError));
    else write(STDERR_FILENO, msg, strlen(msg));
    exit(1);
}

void msg(char *msg) {
    // show msg, **for debug only**
    if (msg == NULL) err("should've put err msg");
    else write(STDERR_FILENO, msg, strlen(msg));
}

char *strcopy(char *b) {
    int len = strlen(b);
    char *res = (char*)malloc(sizeof(char) * (len+1));
    memcpy(res, b, sizeof(char) * len);
    return res;
}



// ===============main===================

typedef struct Cmd {
    char *cmdName;
    int argc;
    char **argv;
} Cmd; 


Cmd* newCommand(char *cmdName) {
    Cmd* res = (Cmd*)malloc(sizeof(Cmd));
    res->cmdName = strcopy(cmdName);
    printf("%s\n", res->cmdName);
    res->argc = 0;
    res->argv = malloc(sizeof(char*) * (res->argc + 2));
    res->argv[0] = strcopy(cmdName);
    //The first argument, by convention, should point to the filename associated with the file being  executed.
    res->argv[res->argc + 1] = NULL;
    // argument list should be NULL terminated according to the manual page of exec()
    return res;
}

int run(Cmd *c) {
    return execvp(c->cmdName, c->argv);
}

int runtest(Cmd *c) {
    return system(c->cmdName);
}

void testargv(char *cmd) {
    msg(cmd);
    char *token;
    const char* delim = ":;";
    msg("FUCK\n");
    char *saveptr;
    token = strtok_r(cmd, delim, &saveptr);
    printf("%s\n", token);
    // for (char *s = cmd; ; s = NULL) {
    //     if (s != NULL) printf("%s %s\n", s, delim);
    //     token = strtok_r(s, delim, &saveptr);
    //     if (token == NULL) break;
    //     printf("%s\n", token);
    // }
}

int main(int argc, char const *argv[])
{
    char cmd[] = "a/bbb///cc;xxx:yyy:";
    char *token = strtok(cmd, ":;");
    // testargv(cmd);
    return 0;
}


