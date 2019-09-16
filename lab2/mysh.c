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
    
    test();
   
    while (1) {
        printf("mysh>");
        static char s[CARRIGE_SIZE];
        fgets(s, CARRIGE_SIZE, stdin);
        Cmd *cmd = newCommand(s);
        run(cmd);
    }
    
    return 0;
}


