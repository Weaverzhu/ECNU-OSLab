#include "util.h"
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

    while (1) {
        dup2(stdOriginFd, STDOUT_FILENO);
        printf("mysh>");
        static char s[CARRIGE_SIZE];
        fgets(s, CARRIGE_SIZE, stdin);
        Cmd *cmd = newCommand(s);
        run(cmd);
    }
    
    return 0;
}


