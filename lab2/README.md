# Project 1a: A Unix Shell

完成一个 `shell` 需要考虑各个模块的实现方式，以下是 `mysh` 大致实现思路

## Multi-process

### fork

```c
#include <unistd.h>
pid_t fork(void);
// Returns: 0 in child, process ID of child in parent, −1 on error
```

我们肯定不希望交互的 `shell` 程序在运行命令的时候卡死，但是同一程序只会串行的执行下一句命令。这个时候我们创建一个新的进程执行命令。

`fork()` 会复制一个新的进程，包括当前进程的各种数据。

> The new process created by fork is called the child process. This function is called once
but returns twice. The only difference in the returns is that the return value in the child
is 0, whereas the return value in the parent is the process ID of the new child. The
reason the child’s process ID is returned to the parent is that a process can have more
than one child, and there is no function that allows a process to obtain the process IDs of
its children. The reason fork returns 0 to the child is that a process can have only a
single parent, and the child can always call getppid to obtain the process ID of its
parent. (Process ID 0 is reserved for use by the kernel, so it’s not possible for 0 to be the
process ID of a child.) 
> -- <cite>Page 229, Advanced Programming in the UNIX Environment,
W. Richard Stevens, Stephen A. Rago</cite>

书中很好的解释了这个函数的运行原理。子进程只有一个父进程，父进程会有多个子进程。父进程会通过函数返回值得到子进程的 `pid`, 而子进程通过得到返回值 `0` 得知自己是生成的子进程。

> Both the child and the parent continue executing with the instruction that follows
the call to fork. The child is a copy of the parent. For example, the child gets a copy of
the parent’s data space, heap, and stack. Note that this is a copy for the child; the parent
and the child do not share these portions of memory
> -- <cite>Page 229 In Advanced Programming in the UNIX Environment,
W. Richard Stevens, Stephen A. Rago</cite>

子进程和父进程有相互独立的内存空间。不过这样会有相当大的内存浪费。书中也讲到很多时候 `fork()` 经常会迎来下一句的 `exec()`，复制出来的内存常常是不需要的。

> Instead, a technique called copy-on-write (COW) is used.
> -- <cite>Page 229, Advanced Programming in the UNIX Environment,
W. Richard Stevens, Stephen A. Rago</cite>

使用这种技术，只有在父子进程中做出修改，才会创造出副本。





### Background Jobs

在默认情况下，`mysh` 会等待上一条命令运行结束才会允许让用户执行下一条命令。由于进程是并发执行的，我们需要让父进程（shell进程）停下来等待子进程。有用的函数是 `waitpid()`


> When a process terminates, either normally or abnormally, the kernel notifies the parent
by sending the SIGCHLD signal to the parent. Because the termination of a child is an
asynchronous event—it can happen at any time while the parent is running — this
signal is the asynchronous notification from the kernel to the parent. The parent can
choose to ignore this signal, or it can provide a function that is called when the signal
occurs: a signal handler.
> -- <cite>Page 229, Advanced Programming in the UNIX Environment,
W. Richard Stevens, Stephen A. Rago</cite>

简而言之，执行`waitpid(cpid, &status, 0)` 的父进程会在子进程运行的时候被阻塞，然后在子进程停止运行后返回结果于 `status` 中。这正是我们需要的，如果不是后台命令，就等，否则不等。


## Some steps


### Parsing commands

我们显然需要解析输入的用户命令

如果不考虑实现 `Pipe`，我们要做的是以空格(white space)作为分割，获取各个参数。一个好用的命令是 `strtok()`，该函数能以分隔符中的字符将一个字符串分割成多个字符串。

```c
#include <string.h>
char *strtok(char *str, const char *delim);
char *strtok_r(char *str, const char *delim, char **saveptr);
```

我们设置好分隔符，然后不断调用该函数。第一次 `*str` 为我们要分割的字符串。这里最好使用线程安全的 `strtok_r`。

值得注意的时，`*str` 作为函数参数时，可能会被修改，最好使用 `strdup()` 函数复制一个新的字符串。

### get feature characters

我们需要截取特定的字符比如 '&' 来判断是否命令是后台执行的，有用的函数是 `strchar()`

```c
#include <string.h>
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
```

它会返回指向第一个出现字符的指针。在 `mysh` 里，我们要求重定向符号和后台命令符号都出现在末尾，因此我们只需要判断是否出现这样的标志字符以及他们的位置是否在末尾就行了。



## Functions

### Running commands

使用 `execvp()` 命令，可以在程序自动在相关环境变量下搜索程序位置并且执行。我们使用新的进程去运行命令，之后一直为空。

我们需要将解析完的命令名称和参数作为参数，然后别忘记如果没有正常退出就报错。

### Built-in commands

内建函数是一类特殊的指令。因为子进程和父进程的内存空间是相互独立的，所以如果要改变父进程的状态（比如切换目录），就必须在父进程本身完成。内建命令通常就是需要在父进程下完成的命令。这里简单的匹配命令实现即可。

### Redirection

我们使用 `dup2()` 来进行重定向。其原理是关闭原来的 file descriptor, 而成为另一个 descriptor 的副本。这里我们需要将 `STDOUT_FILENO` 重定向为要输出的文件。

### Program Errors

由于只有一个错误信息，这里我们只需要返回 `-1`，然后在主函数的消息循环里面打印错误信息。

### Batch mode

重定向 stdin 为 batch file 即可

## Pipe

主要的工作量还是实现管道

管道是进程通信的方法。如果我们没有系统实现的 pipe，如何实现 IPC 进程间通信呢？就是将一个进程写入一个文件，然后另一个进程去读。我们事先准备好一个读的 file descriptor，一个写的。然后 fork() 出一个新的进程。父进程写给子进程的话，就把子进程的 write 关掉，父进程的 read 关掉。然后我们就发现这基本就和 `pipe(int fd[])` 一样了

### methods

这里 mysh 的管道实现方式和 bash 及 xv6 shell 的略有不同

#### mysh

首先使用 '|' 分割符将命令行分成若干条命令，使用链表维护。如果当前命令不是下一个命令，重定向**当前指令的输出为下一条指令的输入**。

#### xv6 shell

对于每一个分隔符 '|'，将两边的命令作为**子进程**执行然后重定向前者的输出为后者的输入。

#### different behaviors

于是在这两种不同的实现下，内建函数就有了不一样的行为。比如对于命令 `cd | wc`，按照道理来说，`cd` 没有输入的情况下应该将自身工作目录转为根目录，然后没有输出。再输入 `pwd` 的时候，应该输出根目录。可是在 xv6 shell 的方式下 `cd` 命令将会在子进程，并不会改变目录。

### some details

这里将讲解实现 `pipe` 的过程

```c
typedef struct Pipe {
    int pipefd[2];
} Pipe;

Pipe *newPipe();
int closeWrite(Pipe *p);
int closeRead(Pipe *p);
void closeBoth(Pipe *p);
```
FILE: pipe.c

`Pipe` 结构体内有一个数组表示两个读写端的 `file descriptor`

```c
char **cmdstr = parse(cmdline, "|\n", 0);
if (cmdstr == NULL) return NULL;

CmdList *head = NULL;
for (int i=0; cmdstr[i]!=NULL; ++i){
    Cmd *newcmd = newCommand(cmdstr[i]);
    head = insertCmd(head, newcmd);
}
```
FILE: cmd.c in method `Cmd *newCommand(char *cmdstr)`

先以管道分隔符分成多个命令，存放于链表中

```c
int runCmdWithPipe(CmdList *head) {
    Pipe *p;
    pid_t lastpid;
    int id = 0;

    int pids[500], pcnt = 0;
    char *buf = NULL;

    for (CmdList *t=head; t!=NULL; t=t->next) {
        
        p = NULL;
        if (t->next != NULL) {
            p = newPipe();
            t->pright = t->next->pleft = p;
        } else {
            t->pright = NULL;
        }

        #ifdef DEBUG
        setred;
        fprintf(stderr, "id=%d pleft=%p pright=%p\n", id, t->pleft, t->pright);
        #endif

        Cmd *c = t->data;
      
        if (isBuiltIn(c)) {
            dbg("yes");
            if (t->pleft != NULL) {
                dup2(t->pleft->pipefd[0], STDIN_FILENO);
            }
            int ret = tryRedirect(c);
            if (ret == -1) return -1;
            buf = allocate(char, SIZE);
            ret = tryBuiltIn(c, buf);
            if (ret == -1) return -1;
        } else {
            dbg("no");
            pid_t cpid = fork();
            if (cpid == -1) return -1;
            if (cpid == 0) {
                if (t->pleft != NULL) {
                    if (buf != NULL) {
                        dbg("close here");
                        closeWrite(t->pleft);
                    }
                    dup2(t->pleft->pipefd[0], STDIN_FILENO);
                }
                if (t->pright != NULL) {
                    closeRead(t->pright);
                    dup2(t->pright->pipefd[1], STDOUT_FILENO);
                }


                int ret = tryRedirect(c);
                if (ret == -1) return -1;
                
                ret = execvp(c->argv[0], c->argv);
                if (t->pright != NULL) closeWrite(t->pright);
                if (ret == -1) return -1;
                exit(-1);
            } else {
                if (buf != NULL) {
                    dbg("output from last builtin");
                    #ifdef DEBUG
                    fprintf(stderr, "buf=%s\n", buf);
                    #endif
                    closeRead(t->pleft);
                    write(t->pleft->pipefd[1], buf, strlen(buf));
                    free(buf); buf = NULL;
                    closeWrite(t->pleft);
                }

                if (t->pright != NULL)
                    closeWrite(t->pright);
                pids[pcnt++] = cpid;
            }
        }
        dup2(ORIGIN_STDIN_FILENO, STDIN_FILENO);
        dup2(ORIGIN_STDOUT_FILENO, STDOUT_FILENO);
        ++id;
    }
    if (!isBackground) {
        dbg("wait");
        for (int i=0; i<pcnt; ++i) {
            waitpid(pids[i], NULL, 0);
        }
    }
    if (buf != NULL) {
        
        write(STDOUT_FILENO, buf, strlen(buf));
        buf[strlen(buf)] = '\n';
        free(buf); buf = NULL;
        
    }
    return 0;
}
```

FILE: cmd.c

这一段是运行的主体部分

首先定义向后的管道

```c
Pipe *p;
```

如果之后还有命令，则我们需要建立管道。这里每个链表节点有两个管道结构体，`pleft` 表示输入的管道，`pright` 表示输出的管道

```c
p = NULL;
if (t->next != NULL) {
    p = newPipe();
    t->pright = t->next->pleft = p;
} else {
    t->pright = NULL;
}
```

如果该命令是内建命令，则在本进程执行。如果有左边的管道，则重定向 `stdin`。注意这里会将内建命令的输出提取出来，并且会等内建命令结束之后才会进行接下来的命令。（在 shell 进程中只能是串行执行下去的）输出将会暂存于 `buf` 当中

```c
if (isBuiltIn(c)) {
    dbg("yes");
    if (t->pleft != NULL) {
        dup2(t->pleft->pipefd[0], STDIN_FILENO);
    }
    int ret = tryRedirect(c);
    if (ret == -1) return -1;
    buf = allocate(char, SIZE);
    ret = tryBuiltIn(c, buf);
    if (ret == -1) return -1;
} else {
```

普通命令，则创建新的进程

首先子进程看是否有左边来的管道，有就重定向 `stdin`。之后看右边是否还需要管道，有就重定向 `stdout`。

之后看是否有重定向输出到文件。因为输出文件肯定优先级大于管道，故放在管道后进行。

之后执行 `execvp()`，并且有相应的报错。

作为父进程，首先得确定左边的命令是否为内建命令，如果是，管道由 `shell` 进程中写入 `buf` 内容执行。否则，由上一个进程的 `pright` 进行桥接。


```c
} else {
    dbg("no");
    pid_t cpid = fork();
    if (cpid == -1) return -1;
    if (cpid == 0) {
        if (t->pleft != NULL) {
            if (buf != NULL) {
                dbg("close here");
                closeWrite(t->pleft);
            }
            dup2(t->pleft->pipefd[0], STDIN_FILENO);
        }
        if (t->pright != NULL) {
            closeRead(t->pright);
            dup2(t->pright->pipefd[1], STDOUT_FILENO);
        }


        int ret = tryRedirect(c);
        if (ret == -1) return -1;
        
        ret = execvp(c->argv[0], c->argv);
        if (t->pright != NULL) closeWrite(t->pright);
        if (ret == -1) return -1;
        exit(-1);
    } else {
        if (buf != NULL) {
            dbg("output from last builtin");
            #ifdef DEBUG
            fprintf(stderr, "buf=%s\n", buf);
            #endif
            closeRead(t->pleft);
            write(t->pleft->pipefd[1], buf, strlen(buf));
            free(buf); buf = NULL;
            closeWrite(t->pleft);
        }

        if (t->pright != NULL)
            closeWrite(t->pright);
        pids[pcnt++] = cpid;
    }
}
```

如果内建命令重定向了 `stdin` 和 `stdout` ，这里需要设置回来。由于这里的实现会使在执行内建命令的时候重定向 `stdin`，在某一瞬间可能无法输入命令。当然内建命令执行的速度相当快（这里项目要求实现的内建命令都是可以瞬间执行完成的），因此对使用几乎没有影响

```c
dup2(ORIGIN_STDIN_FILENO, STDIN_FILENO);
dup2(ORIGIN_STDOUT_FILENO, STDOUT_FILENO);
++id;
```

最后是收尾工作，如果最后一个命令是内建命令，需要从 `buf` 中输出。如果不是后台运行，需要等待所有中途的子进程执行完毕
```c
if (!isBackground) {
    dbg("wait");
    for (int i=0; i<pcnt; ++i) {
        waitpid(pids[i], NULL, 0);
    }
}
if (buf != NULL) {
    
    write(STDOUT_FILENO, buf, strlen(buf));
    buf[strlen(buf)] = '\n';
    free(buf); buf = NULL;
    
}
return 0;
```

### conclusion