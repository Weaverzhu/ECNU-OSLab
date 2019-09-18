
#include "cmd.h"

#define CARRIGE_SIZE 512

// ===============main===================


void test() {
    char *buf = NULL;
    buf = getcwd(buf, 0);
    printf("%s\n", buf);
    exit(0);
}


int main(int argc, char const *argv[])
{
    stdOriginFd = dup(STDOUT_FILENO);

    #ifdef TEST
    test();
    #endif
    
    char curDir[100];
    chdir(getcwd(curDir, 100));
    write(stdOriginFd, SHELL_HEADER, strlen(SHELL_HEADER));
    while (1) {
        dup2(stdOriginFd, STDOUT_FILENO);
        static char s[CARRIGE_SIZE];
        fgets(s, CARRIGE_SIZE, stdin);
        Cmd *cmd = newCommand(s);
        run(cmd);
        write(stdOriginFd, SHELL_HEADER, strlen(SHELL_HEADER));
    }
    
    return 0;
}


