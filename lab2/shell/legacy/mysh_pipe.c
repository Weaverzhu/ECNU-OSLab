
#include "cmd.h"

#define CARRIGE_SIZE 512

// ===============main===================


void test() {
    char *buf = NULL;
    buf = getcwd(buf, 0);
    printf("%s\n", buf);
    exit(0);
}



Cmd *firstCmd, *lastCmd;

void go(CmdList *head) {
    Cmd *c = head->data;
    int pipefd[2];
    int ret = pipe(pipefd);
    if (ret == -1) {
        fprintf(stderr, "mysh: error in pipe\n");
        return;
    } else if (head->next != NULL) {
        pid_t cpid = fork();
        if (cpid == 0) {
            dup2(pipefd[1], STDOUT_FILENO);
            run(c);
            return;
        } else {
            head = head->next;
            go(head);
        }
    }
}

int main(int argc, char const *argv[])
{
    stdoutOriginFd = dup(STDOUT_FILENO);
    stdinOriginFd = dup(STDIN_FILENO);

    #ifdef TEST
    test();
    #endif
    
    char curDir[100];
    chdir(getcwd(curDir, 100));
    write(stdoutOriginFd, SHELL_HEADER, strlen(SHELL_HEADER));
    // while (1) {
    //     dup2(stdOriginFd, STDOUT_FILENO);
    //     static char s[CARRIGE_SIZE];
    //     fgets(s, CARRIGE_SIZE, stdin);
    //     Cmd *cmd = newCommand(s);
    //     run(cmd);
    //     write(stdOriginFd, SHELL_HEADER, strlen(SHELL_HEADER));
    //     free(cmd);
    // }

    while (1) {
        dup2(stdoutOriginFd, STDOUT_FILENO);
        dup2(stdinOriginFd, STDIN_FILENO);
        // reset input and output flow

        firstCmd = lastCmd = NULL;
        // reset first and last cmd

        static char s[CARRIGE_SIZE];
        fgets(s, CARRIGE_SIZE, stdin);

        char **cmds = parseCmd(s, "|");
        #ifdef DEBUG
        fprintf(stderr, "pipe:\n");
        for (int i=0;cmds[i]!=NULL; ++i)
            fprintf(stderr, "[%d] cmd: %s\n", i, cmds[i]);
        #endif

        CmdList *head = NULL;
        
        for (int i=0; cmds[i]!=NULL; ++i) {
            head = insertCmd(head, lastCmd = newCommand(cmds[i]));
            if (firstCmd == NULL) firstCmd = lastCmd;   
        }

        
        
    }
    
    return 0;
}


