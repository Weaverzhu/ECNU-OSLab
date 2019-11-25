# 操作系统实践第三次实验报告

## Project 3b: xv6 VM Layout

### Part A: Null-pointer Dereference

#### Objectives

* 熟悉 xv6 虚拟内存系统
* 给 xv6 添加一些现代操作系统常有的功能


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

```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char const *argv[])
{
    char *a;
    printf(1, "%d\n", *a);
    exit();
}
```


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
	nulldereference # new program we add
```

结果如下，发现指针 `a` 指向未知的一串值

```sh
xv6...
lapicinit: 1 0xfee00000
cpu1: starting
cpu0: starting
init: starting sh
$ nulldereference
-115
$ 
```

当我们在 `Linux` 中运行类似的程序

```c
#include <stdio.h>

int main(int argc, char const *argv[])
{
    char *a;
    printf("%d\n", *a);
    return 0;
}
```

```sh
zt@iZuf60n9722bkqxpt1w1sgZ:~/ECNU-OSLab/lab4/test$ ./main
Segmentation fault
```

#### figuring out the outlook of xv6 VM

我们先来观察 `exec()` 函数，这个函数会创建一个新的进程运行指定程序。这个过程中将会给新程序分配内存空间，我们可以观察到新的一段虚拟内存空间是怎样被创建的

小技巧：使用 `visual studio code` 组合键 `Ctrl + 鼠标左键` 点击函数定义 (在 `./xv6 VM Layout/user/user.h`) 中可以快速定位函数实现在 `./xv6 VM Layout/kernel/exec.c`

##### page table

`xv6` 虚拟地址定义如下

32 位无符号地址被分为三个部分，第一个部分为 page directory index，第二部分为 page table index，第三部分表示 offset with page。这样每个页表构成 $2^{10}$ 个页表项，每个页表项有 $2^{12}$ 字节。然后我们也可以通过简单的位运算获取线性地址的这些部分。

回顾一下 x86 系统中使用两级树结构存放内存，每一级都是一个 1024 项的表，每一项是一个 32 位的数据，一般来说前 20 位表示物理地址的前 20 位，也就是我们要做的映射结果；后 12 位表示各种 flag。第一级存了1024个 page table，我们把这一级称为 page directory。因为每个 page table 正好有 1024x4=4096 大，前 20 位刚好可以表示一个 page table 的头，所以这里第一级的结构里面放的都是页表。第二级存放了 1024 项 physical page number(PPN) 这 20 位替换虚拟地址中的前二十位就是物理地址了。总的来说，虚拟地址替换为物理地址的步骤时，前 10 位在 page directory 中找到 page table，然后 10 位找到 PPN，最后替换虚拟地址的前 20 位。原理如下图所示

![img/pagemode.PNG](img/pagemodel.PNG)

`./xv6 VM Layout/kernel/mmu.h`
```c
// A linear address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(la) --/ \--- PTX(la) --/

// page directory index
#define PDX(la)		(((uint)(la) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(la)		(((uint)(la) >> PTXSHIFT) & 0x3FF)

// construct linear address from indexes and offset
#define PGADDR(d, t, o)	((uint)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))
```

每个页表项需要一些 flag 设置他们的属性，比如说 Writeable 表示可以写入，Present 表示这个页表已经被用到了。还可以指定被当成缓存时的各种策略

`./xv6 VM Layout/kernel/mmu.h`
```c
// Page table/directory entry flags.
#define PTE_P		0x001	// Present
#define PTE_W		0x002	// Writeable
#define PTE_U		0x004	// User
#define PTE_PWT		0x008	// Write-Through
#define PTE_PCD		0x010	// Cache-Disable
#define PTE_A		0x020	// Accessed
#define PTE_D		0x040	// Dirty
#define PTE_PS		0x080	// Page Size
#define PTE_MBZ		0x180	// Bits must be zero
```

##### memory layout

xv6 使用如下的内存外貌

![./img/memlayout.PNG](./img/memlayout.PNG)

用户内存从 0 开始一直到 KERNBASE。在 `./xv6 VM Layout/include/types.h` 中我们看到 NULL 被定义为 0。因此一个空指针会访问到 User text 部分，这部分的页表 flag 上有 `Present` 因此会被认为是一个合法的内存访问。现在我们希望把前两页空出来，这样当前两页将没有 `Present` flag，访问的时候 kernel 就会帮我们处理错误。

##### Code: exec

exec 函数首先会检查 page table directory 是否设置，以及检查 elf header。

让我们先理解一些函数（无关紧要的细节已经被忽略）

`./xv6 VM Layout/kernel/kalloc.c` 该函数分配一个 4096 长度的物理内存空间
```
// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void) {
	// ...
}
```

`./xv6 VM Layout/kernel/vm.c walkpgdir` 返回页表地址

```c
// Return the address of the PTE in page table pgdir
// that corresponds to linear address va.  If create!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int create) {
	// ...
}
```

`./xv6 VM Layout/kernel/vm.c mappages` 该函数把会根据新分配的物理空间（4096长度）建立页表。也就是说会建立参数 la (虚拟地址) 到 pa (物理地址) 之间的映射。我们可以看到获取 `la` 的地址后会检查是否该地址已经被其他物理地址占用了（`panic("remap")`），否则就把 flag 加上 Present 一起添加。由之前的定义，相邻 page table directory 本来就有相邻的地址，如果这个 page table 满了自动就会应用到下一个 page table 中。（其实也可以看作是 $2^{20}$ 个连续页表）

```c
// Create PTEs for linear addresses starting at la that refer to
// physical addresses starting at pa. la and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, void *la, uint size, uint pa, int perm)
{
  char *a, *last;
  pte_t *pte;
  
  a = PGROUNDDOWN(la);
  last = PGROUNDDOWN(la + size - 1);
  for(;;){
    pte = walkpgdir(pgdir, a, 1);
    if(pte == 0)
      return -1;
    if(*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}
```

`./xv6 VM Layout/kernel/vm.c` 我们要改的代码在这个函数中
```c
int
allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  char *mem;
  uint a;

  if(newsz > USERTOP)
    return 0;
  if(newsz < oldsz)
    return oldsz;

  a = PGROUNDUP(oldsz);
  for(; a < newsz; a += PGSIZE){
    mem = kalloc();
    if(mem == 0){
      cprintf("allocuvm out of memory\n");
      deallocuvm(pgdir, newsz, oldsz);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    mappages(pgdir, (char*)a, PGSIZE, PADDR(mem), PTE_W|PTE_U);
  }
  return newsz;
}
```

虚拟地址就是 `a`，程序刚开始加载时有 `oldsz=0`，`a` 自然等于 0，这是目前从 0 开始存放用户内存空间的情况。那我们就加上 `2x4096=8192` 到 `a` 上，使得整体的虚拟内存后移了一段，这样 0 这个位置本来表示 `NULL` 的地方在页表中没有 Preset 标签，程序就会出错了。


```c
  // a = PGROUNDUP(oldsz);
  a = PGROUNDDOWN(oldsz) + 8192;
```

但是如果我们现在编译运行 `xv6` 发现无法启动 

```sh
xv6...
lapicinit: 1 0xfee00000
cpu1: starting
cpu0: starting
pid 1 initcode: trap 14 err 7 on cpu 1 eip 0x7e addr 0xfffffff4--kill proc
cpu1: panic: init exiting
 104057 106095 pid 2 initcode: trap 14 err 7 on cpu 0 eip 0x7e addr 0xfffffff4--kill proc
105cb3 0 0 0 0 0 0 0
```

##### Code: fork

`fork` 函数会创建新的进程，这个过程中会复制出新的页表，这里有一个地方也需要更改


`./xv6 VM Layout/kernel/vm.c copyuvm`
```c
for(i = 0; i < sz; i += PGSIZE){
    // if((pte = walkpgdir(pgdir, (void*)i, 0)) == 0)
    if((pte = walkpgdir(pgdir, (void*)i+8192, 0)) == 0)
```

显然这里的 shift 也需要被加上