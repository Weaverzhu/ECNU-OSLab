# Project 1a: A Unix Shell

完成一个 `shell` 需要考虑各个模块的实现方式，以下是 `mysh` 大致实现思路

## File structure

```
|-- lab2
    |-- cmd.c // 执行命令
    |-- cmd.h
    |-- config.c // 全局设置
    |-- config.h
    |-- makefile // makefile
    |-- mysh
    |-- mysh.c // 主程序
    |-- mysh.h
    |-- pipe.c // 管道
    |-- pipe.h
    |-- run.sh // 执行脚本
    |-- util.c // 一些辅助函数
    |-- util.h

```

### Run

运行说明

```sh
$ make
$ ./mysh [batch file]
```

或者直接交互

```sh
$ ./run.sh
```

## Environment

+ gcc version 5.5.0 20171010 (Ubuntu 5.5.0-12ubuntu1~16.04)
+ aliyun 轻量应用服务器 2GB RAM Ubuntu 16.04

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

这里我们需要去掉尾部的 whitespace



## Functions

### utils

一些定义的辅助函数

```c
char **parse(char *str, const char *delim, int allowEmpty) {
    dbg("parse started");
    char *localstr = strdup(str), *token, *saveptr;
    static char buf[SIZE >> 1][SIZE];
    int count = 0;
    for (char *s=localstr; ; s=NULL) {
        token = strtok_r(s, delim, &saveptr);
        if (token == NULL) break;
        if (strlen(token) == 0) {
            if (allowEmpty) continue;
            else return NULL;
        }
        strcpy(buf[count++], token);
    }
    char **res = allocate(char*, count+1);
    for (int i=0; i<count; ++i) {
        res[i] = strdup(buf[i]);
    }
    res[count] = NULL;
    dbg("parse completed");
    return res;
}

#define allocate(type, size) (type*)malloc(sizeof(type) * size)
#define match(a, b) (strcmp(a, b) == 0)
```
FILE util.c, util.h

会根据 delim 中间的字符分割字符串，返回二维字符数组



### Running commands

使用 `execvp()` 命令，可以在程序自动在相关环境变量下搜索程序位置并且执行。我们使用新的进程去运行命令，之后一直为空。

我们需要将解析完的命令名称和参数作为参数，然后别忘记如果没有正常退出就报错。解析的方法就是使用空格分隔成一些字符串，然后作为参数。

### Built-in commands

内建函数是一类特殊的指令。因为子进程和父进程的内存空间是相互独立的，所以如果要改变父进程的状态（比如切换目录），就必须在父进程本身完成。内建命令通常就是需要在父进程下完成的命令。这里简单的匹配命令实现即可。我遇到的一个有趣的坑点是，如果在 pipe 中坚持在 shell 进程中运行内建函数，那么就可能会出现连续的内建函数，父进程的输入管道必须关闭，因为父进程和子进程的管道 file descriptor 是相反的，而要使得父进程对父进程通信，显然可能会出错。这样做看似有问题（不就读入不了东西了吗），但是由于内建函数并不会从 stdin 中读入，所以也不算是问题。

### Redirection

我们使用 `dup2()` 来进行重定向。其原理是关闭原来的 file descriptor, 而成为另一个 descriptor 的副本。这里我们需要将 `STDOUT_FILENO` 重定向为要输出的文件。

解析这个字符需要一点耐心，因为有各种 whitespace 会造成困扰。这里的实现不是很优雅，因为配合后面的pipe，我们希望重定向的优先级高于pipe。一开始写 `struct Cmd` 的结构时没有考虑到，因此等到重定向的时候命令指令已经被打碎成空格分隔了。这里的实现就是暴力判断了一下，分别判断 `>` 出现在参数末尾，出现在开头和单个出现的情况。如果重构代码，我肯定会选择一开始就解析出重定向的文件，然后存放于结构体中。

### Program Errors

由于只有一个错误信息，这里我们只需要返回 `-1`，然后在主函数的消息循环里面打印错误信息。要注意的是，如果在子进程出错，子进程要及时调用 `exit` 推出，否则会有同时两个 `shell` 进程运行。

### Batch mode

重定向 stdin 为 batch file 即可

### quote

我没有在作业要求中看到对处理引号的要求，但是在 pipe 的样例中发现要处理 pipe。想着这门课也不是编译原理（主要还是因为懒），我用了一个不太优雅的方法处理引号。这种方法只能处理一些单个引号（没有嵌套引号的情况）。我们成对的寻找命令字符串中的被引号括起来的部分，然后将空格替换成用户无法输入的字符，这里选择的是 `chr(7,8)`，阻止其中的空格被解析，然后再换回来。这样显然无法处理嵌套括号，也对转义符号毫无办法。如果要实现完整的括号功能，我可能会去构建有限状态自动机去解析命令字符串。

### debug

写代码的时候会产生各种各样的 bug，有的时候我们希望输出调试信息，但是又希望能够不输出信息看看调试的结果。这里使用的是条件编译。

```c
#define REPORT_ERR write(STDERR_FILENO, error_message, strlen(error_message));
// #define PRINT_HEADER { if (BATCH_MODE == 0) write(STDOUT_FILENO, HEADER, strlen(HEADER)); else write(ORIGIN_STDOUT_FILENO, cmdline, strlen(cmdline)); }
#define WRITE_FILE_MODE O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU
// #define DEBUG
#define REDIRECT_FLG 2
#define _GNU_SOURCE
```

定义 `DEBUG` 宏之后，会输出调试信息。这里一直没有删除关于调试的语句，为的是以防马上发现 bugs 时又要重新打印。

## Pipe

主要的工作量还是实现管道

管道是进程通信的方法。如果我们没有系统实现的 pipe，如何实现 IPC 进程间通信呢？就是将一个进程写入一个文件，然后另一个进程去读，当然如果需要 read 的时候引发中断，就不知道具体是怎么处理了。我们事先准备好一个读的 file descriptor，一个写的。然后 fork() 出一个新的进程。父进程写给子进程的话，就把子进程的 write 关掉，父进程的 read 关掉。开心的是系统为我们实现了 `pipe`。

### methods

这里 mysh 的管道实现方式和 bash 及 xv6 shell 的略有不同

#### mysh

首先使用 '|' 分割符将命令行分成若干条命令，使用链表维护。如果当前命令不是下一个命令，重定向**当前指令的输出为下一条指令的输入**。

#### xv6 shell

对于每一个分隔符 '|'，将两边的命令作为**子进程**执行然后重定向前者的输出为后者的输入。这样实现的好处就是非常简洁，甚至可以递归嵌套的方式处理各种情况，下面是 xv6 的实现方法，代码量要少得多。


```c
case PIPE:
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){ // 处理左边的命令
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if(fork1() == 0){ // 处理右边的命令
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait();
    wait();
    break;
```
FILE: xv6/user/sh.c

#### different behaviors

于是在这两种不同的实现下，内建函数就有了不一样的行为。比如对于命令 `cd | wc`，按照道理来说，`cd` 没有输入的情况下应该将自身工作目录转为根目录，然后没有输出。再输入 `pwd` 的时候，应该输出根目录。可是在 xv6 shell 的方式下 `cd` 命令将会在子进程，并不会改变目录。

坚持在 shell 进程执行 内建函数还有一个缺点，就是无法完成父进程到父进程的通信（比如说有连续的内建函数）。不过作业要求所有的内建函数都不需要从 stdin 中输入。

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
    int id = 0;

    int pids[500], pcnt = 0;
    char *buf = NULL;

    for (CmdList *t=head; t!=NULL; t=t->next) {
        dbg("in main loop");
        p = NULL;
        if (t->next != NULL) { // if we need to build a pipe for the next cmd
            p = newPipe();
            t->pright = t->next->pleft = p;
        } else {
            t->pright = NULL;
        }

        Cmd *c = t->data;
      
        if (isBuiltIn(c)) {
            if (t->pleft != NULL) {
                closeRead(t->pleft); // should be close if the last cmd is also built-in
            }
            int ret = tryRedirectold(c);
            if (ret == -1) return -1;
            buf = allocate(char, SIZE);
            ret = tryBuiltIn(c, buf);
            if (ret == -1) return -1;
        } else {
            pid_t cpid = fork();
            if (cpid == -1) return -1;
            if (cpid == 0) {
                if (t->pleft != NULL) {
                    if (buf != NULL) {
                        closeWrite(t->pleft);
                    }
                    dup2(t->pleft->pipefd[0], STDIN_FILENO);
                }
                if (t->pright != NULL) {
                    closeRead(t->pright);
                    dup2(t->pright->pipefd[1], STDOUT_FILENO);
                }


                int ret = tryRedirect(c);
                if (ret == -1) {
                    REPORT_ERR;
                    exit(-1);
                }
                
                ret = execvp(c->argv[0], c->argv);
                REPORT_ERR; // error msg should be display here
                if (t->pright != NULL) closeWrite(t->pright);
                if (t->pleft != NULL) closeRead(t->pleft);
                exit(-1); // exit properly
            } else {
                if (buf != NULL) {
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
        dbg("id increased");
    }
    if (!isBackground) {
        for (int i=0; i<pcnt; ++i) {
            waitpid(pids[i], NULL, 0);
        }
    }
    if (buf != NULL) {
        write(STDOUT_FILENO, buf, strlen(buf));
        buf[strlen(buf)] = '\n';
        free(buf); buf = NULL;

    }
    dbg("return 0 correctly");
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
    if (t->pleft != NULL) {
        closeRead(t->pleft); // should be close if the last cmd is also built-in
    }
    int ret = tryRedirectold(c);
    if (ret == -1) return -1;
    buf = allocate(char, SIZE);
    ret = tryBuiltIn(c, buf);
    if (ret == -1) return -1;
}
```

普通命令，则创建新的进程

首先子进程看是否有左边来的管道，有就重定向 `stdin`。之后看右边是否还需要管道，有就重定向 `stdout`。

之后看是否有重定向输出到文件。因为输出文件肯定优先级大于管道，故放在管道后进行。

之后执行 `execvp()`，并且有相应的报错。

作为父进程，首先得确定左边的命令是否为内建命令，如果是，管道由 `shell` 进程中写入 `buf` 内容执行。否则，由上一个进程的 `pright` 进行桥接。


```c
pid_t cpid = fork();
if (cpid == -1) return -1;
if (cpid == 0) {
    if (t->pleft != NULL) {
        if (buf != NULL) {
            closeWrite(t->pleft);
        }
        dup2(t->pleft->pipefd[0], STDIN_FILENO);
    }
    if (t->pright != NULL) {
        closeRead(t->pright);
        dup2(t->pright->pipefd[1], STDOUT_FILENO);
    }


    int ret = tryRedirect(c);
    if (ret == -1) {
        REPORT_ERR;
        exit(-1);
    }
    
    ret = execvp(c->argv[0], c->argv);
    REPORT_ERR; // error msg should be display here
    if (t->pright != NULL) closeWrite(t->pright);
    if (t->pleft != NULL) closeRead(t->pleft);
    exit(-1); // exit properly
} else {
    if (buf != NULL) {
        closeRead(t->pleft);
        write(t->pleft->pipefd[1], buf, strlen(buf));
        free(buf); buf = NULL;
        closeWrite(t->pleft);
    }

    if (t->pright != NULL)
        closeWrite(t->pright);
    pids[pcnt++] = cpid;
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

之前很少写过类似的比较“大”的程序。个人认为一个比较好的方案时先思考接口，然后根据接口实现每个函数。将每个实现的函数分割开来，这样可以最大限度地保证思路清晰。最糟糕的情况就是需要改变接口，因为这样就要改变程序的很多地方，哪里又改错了，就更加万劫不复了。我最开始设计的实现管道的方案是这样的：子进程只和父进程有数据交换。父进程一次读取完所有当前命令的输出，存入 `buf[]` 然后一次性输入到下一个子进程。然而这样就会与 `bash` 发出不一样的行为，比如命令 `sleep 5s | echo hello`，由于 `sleep` 并不会输出，所以 `echo hello` 自然不会等待 `sleep`，而最初的方案就会因为读入流未关闭而阻塞，必须等 `sleep` 结束才能继续执行 `echo`。于是我整个程序就要面临大改，管道甚至基本重构了一遍代码。动手写代码之前先想好系统的设计真的很重要，而最后敲键盘的时间其实真的不多，而其中写功能而不是忙着输出变量调试的部分就更少了。


# Project 1b: xv6 System Call

我们的目的是实现一个新的系统调用，那么一个合理的方法是看看其他的系统调用是在哪里实现的

在 ./kernal/ 可以寻找到 syscall.c，按照道理来说，这肯定是关于系统调用的代码。


```c
// array of function pointers to handlers for all the syscalls
static int (*syscalls[])(void) = {
[SYS_chdir]   sys_chdir,
[SYS_close]   sys_close,
[SYS_dup]     sys_dup,
[SYS_exec]    sys_exec,
[SYS_exit]    sys_exit,
[SYS_fork]    sys_fork,
[SYS_fstat]   sys_fstat,
[SYS_getpid]  sys_getpid,
[SYS_kill]    sys_kill,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_mknod]   sys_mknod,
[SYS_open]    sys_open,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_unlink]  sys_unlink,
[SYS_wait]    sys_wait,
[SYS_write]   sys_write,
[SYS_uptime]  sys_uptime,
[SYS_getreadcount] sys_getreadcount // new added
};
// Called on a syscall trap. Checks that the syscall number (passed via eax)
// is valid and then calls the appropriate handler for the syscall.
void
syscall(void)
{
  int num;
  num = proc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num] != NULL) {
    proc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            proc->pid, proc->name, num);
    proc->tf->eax = -1;
  }
}

```

看起来这里执行的逻辑是，`trap` 执行后，从 `eax` 寄存器中找到函数 `id`，然后再从上面的数组中找到函数指针。

其中 `NELEM(syscalls)` 是宏定义，不需要特别改动，所以我们只需要为数组加上我们想要的函数指针就行了。

那么这个函数指针从哪里来的？我们需要找到地方实现，并且在相应的头文件中给出定义（这样其他文件才能获取函数定义）。我们可以在文件中找到相应注释：

```c
// syscall function declarations moved to sysfunc.h so compiler
// can catch definitions that don't match
```

然后我们再去 `sysfunc.h` 添加我们的函数定义


```c
#ifndef _SYSFUNC_H_
#define _SYSFUNC_H_

// System call handlers
int sys_chdir(void);
int sys_close(void);
int sys_dup(void);
int sys_exec(void);
int sys_exit(void);
int sys_fork(void);
int sys_fstat(void);
int sys_getpid(void);
int sys_kill(void);
int sys_link(void);
int sys_mkdir(void);
int sys_mknod(void);
int sys_open(void);
int sys_pipe(void);
int sys_read(void);
int sys_sbrk(void);
int sys_sleep(void);
int sys_unlink(void);
int sys_wait(void);
int sys_write(void);
int sys_uptime(void);
int sys_getreadcount(void); // new added

#endif // _SYSFUNC_H_

```

这里的函数名对应着刚刚我们在函数指针数组中添加的函数指针名

之后我们去寻找函数实现的地方，肯定和 `sys_read` 有关。从 `sysfile.c` 中我们找到了 `sys_read` 的定义。之后做的事情就相当简单了，我们定义一个全局变量去记录被调用的次数就可以了。


```c
int readcount = 0;

int
sys_read(void)
{
  ++readcount;
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int
sys_getreadcount(void) {
  return readcount;
}

```

那么还有一个问题，处于用户态时用户怎么调用它呢？我们转到 `user` 文件夹，发现了 `user.h` 中的函数定义，添加上即可。

```c
// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int getreadcount(void); // added
```

## test

最后，我们为 `xv6` 添加两个新的程序 `readcounttest, testinfork`

```c
#include "types.h"
#include "stat.h"
#include "user.h"


int
main(int argc, char *argv[])
{
  if (argc > 1) {
      printf(1, "usage: mytest");
  }
  printf(1, "total %d reads\n", getreadcount());
  exit();
}

```

FILE: ./user/readcounttest

测试结果：

```sh
cpu1: starting
cpu0: starting
init: starting sh
$ readcounttest
total 14 reads
```

我们查看 `gets()` 实现：

```c
char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}
```

读入多少字符就会由多少 `read` 产生，`readcounttest`加上回车共 `14` 字符，所以结果显示 `total 14 reads` 符合预测

至于 testinfork，测试的是两个进程共享数据的问题，fork之后看是否会有两倍的读入，结果符合。