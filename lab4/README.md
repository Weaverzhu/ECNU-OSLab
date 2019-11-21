## Project 3b: xv6 VM Layout

### Objectives

* 熟悉 xv6 虚拟内存系统
* 给 xv6 添加一些现代操作系统常有的功能

### Part A: Null-pointer Dereference

#### Steps

1.  找到原始 xv6 和 Linux 系统在访问空指针的区别
2.  理解 xv6 如何建立页表 (page table)，并且改动使其将前两页忽略 (unmapped)
3.  改进 xv6 使其能在访问空指针的时候使用 trap 并且 kill 掉进程

#### Difference between Linux and original xv6

首先设置 qemu 的路径（我的安装在 root 用户下，但是实际使用另一个账户运行，所以运行指令为 `sudo make qemu-nox`）

```sh
# If the makefile can't find QEMU, specify its path here
QEMU := /root/install/qemu-6.828-2.9.0/i386-softmmu/qemu-system-i386
```

编写一个程序，访问空指针，原代码见 `./xv6 VM Layout/user/nulldereference.c`


修改 `./xv6 VM Layout/user/makefile.mk`，添加我们编写的新程序

```sh
# user programs
USER_PROGS := \
	cat\
	echo\
	forktest\
	grep\
	init\
	kill\
	ln\
	ls\
	mkdir\
	rm\
	sh\
	stressfs\
	tester\
	usertests\
	wc\
	zombie\
	nulldereference
```