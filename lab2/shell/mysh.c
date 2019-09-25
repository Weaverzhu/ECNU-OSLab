
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
    stdoutOriginFd = dup(STDOUT_FILENO);

    #ifdef TEST
    test();
    #endif
    
    char curDir[100];
    chdir(getcwd(curDir, 100));
    write(stdoutOriginFd, SHELL_HEADER, strlen(SHELL_HEADER));
    while (1) {
        dup2(stdoutOriginFd, STDOUT_FILENO);
        static char s[CARRIGE_SIZE];
        fgets(s, CARRIGE_SIZE, stdin);
        Cmd *cmd = newCommand(s);
        run(cmd); 
        write(stdoutOriginFd, SHELL_HEADER, strlen(SHELL_HEADER));
    }
    
    return 0;
}


