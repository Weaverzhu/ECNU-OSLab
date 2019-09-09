# Project 0: Linux Warm Up

## 项目简介

这个项目包含两个部分，第一部分主要有 `shell` 脚本完成一些基本的操作，而第二部分由 `linux` 下 `C` 语言编程完成一些基本的操作。在 `bonus` 部分，使用一些高级的文本处理工具完成人物。

## 0a

### Part 1

+ `./s1.sh`

1.  使用 `mkdir` 指令创建文件夹 `foo` 在当前文件夹
2.  使用 `touch` 指令创建新的文件
3.  使用 `echo` 指令和重定向将特定内容写入文件

#### mkdir
+ 用法
```
mkdir [OPTION]... DIRECTORY...
```

创建目录，这里使用的选项为

```sh
-p, --parents
              no error if existing, make parent directories as needed
```

仅当未存在目录时创建，且自动创建相应的父级目录


#### touch

+ 用法
```
touch [OPTION]... FILE...
```

`touch` 本来是改变文件的时间戳，意思就是更新文件的时间信息。但是如果不存在，则会新创建这个文件。

```
A FILE argument that does not exist is created empty, unless -c or
-h is supplied.
```

#### echo

+ 用法
```
echo [SHORT-OPTION]... [STRING]...
echo LONG-OPTION
```

`echo` 指令用于输出一串文字。我们需要这些文字被写入文件，于是使用输出重定向，使用方法为 `> [file name]`

#### cp

+ 用法
```
cp [OPTION]... [-T] SOURCE DEST
cp [OPTION]... SOURCE... DIRECTORY
cp [OPTION]... -t DIRECTORY SOURCE...
```

这里我们使用第二种用法，选项为 `-f`，即强制替换已有文件。

```
   -f, --force
              if an existing destination file cannot be opened, remove it
              and try again (this option is ignored when the -n option is
              also used)
```

+ `./s2.sh`

#### 大致思路

1. 使用 `find` 查找 `b` 开头文件
2. 使用 `ls -l` 列出完整信息
3. 使用 `awk` 输出特定的列
4. 使用 `pipeline`  组合这些程序的输入输出
5. 使用 `sed` 对调整格式

#### rm

+ 用法
```
rm [OPTION]... [FILE]...
```

删除文件，这里使用 `-f` 选项。这里目的是删除之前已有的文件，所以使用该选项，如果没有要删除的文件也忽略警告。

```
-f, --force
              ignore nonexistent files and arguments, never prompt
```

#### find

+ 用法

```
 find  [-H]  [-L] [-P] [-D debugopts] [-Olevel] [starting-point...]
       [expression]
```

使用的第一个选项为 `-iname`，用来匹配 `b` 字母文件名开头的文件

```
-iname pattern
              Like  -name,  but the match is case insensitive.  For exam‐
              ple, the patterns `fo*' and  `F??'  match  the  file  names
              `Foo', `FOO', `foo', `fOo', etc.   The pattern `*foo*` will
              also match a file called '.foobar'.
```
#### awk

`awk` 可以以分隔符为界，逐步处理部分文本，默认为行，即逐行处理。

`awk` 可以使用 `%[0..9]` 指定选取哪些列，列由列分隔符隔开，默认为空格。`%0` 为整行，而其他为指定列

这里我们需要打印第 1, 3, 9 列，并且没有其他条件，于是使用最基本的用法，最后输出到 `./output` 文件

```sh
awk '{print $9" "$3" "$1}' >> output
```


#### | (管道) 和 xargs

管道用来把一个指令的输出加入到另一个指令的标准输入，而 `xargs` 还可以将输入改为下一个命令的参数。我们需要将 `find` 指令找到的文件路径当作参数输入到 `ls` 指令中，而根据 `awk` 的用法， `ls` 输出到标准输出的内容，则需要成为 `awk` 的标准输入内容，故 `find` 和 `ls` 中间使用 `xargs` 而 `ls` 和 `awk` 之间使用 `|`。


#### chmod

+ 用法

```
chmod [OPTION]... MODE[,MODE]... FILE...
chmod [OPTION]... OCTAL-MODE FILE...
chmod [OPTION]... --reference=RFILE FILE...
```

linux 使用字母缩写来表示三个对文件操作的权限

+ r 代表 read 读取
+ w 代表 write 写入
+ x 代表 execute 执行

又使用 `bitmask` 表示这三个字母，分别用 4,2,1 表示 r,w,x 那么所有的权限拥有情况都能用 0..7 表示。

linux 的文件分别有对三个对象的文件权限说明，分别是 `user`, `group` 和 `other`，分别表示用户，用户所在组的其他用户，和其他组的用户。

根据题目要求，只有本用户有读取权限，所以应该为 `rwx-wx-wx`，即指令为 `chmod 644 [file]`

### Part 2

对 `set_operation.c` 进行调试

#### 编译选项

我们可以通过如下方法获取 `gcc` 的使用帮助

```
$ gcc --help
```

一般来说，直接运行程序的话，我们直接运行 `$ gcc [sourcefile] -o [outputfile]`，然后运行 `$ ./outputfile [arg]` 即可。若没有 `-o` 选项，则输出文件为 `a.out`

#### 大致过程

个人来说，我比较喜欢在使用 `gdb` 调试之前，先手动阅览一边代码，即肉眼查错。`gdb` 虽然简单，在有错误数据的情况下一般一定能找出错误，但是万一某些错误在这个数据中和正确的写法会得到同样输出，这时就需要更多的数据，或者干脆来说，直接检查代码找出错误。（根据个人的经验上在 online judge 上练习题目，测试数据是看不到的，往往手动检查代码，比自己再去大规模生成随机数据并写对拍程序更可靠

#### bugs

1. `set_operation.c:26`

      ```c
      if((p->->next)->number==num)
      ```
      从函数的作用可知，这段代码需要检查一个链表中是否包含特定的元素。`while(p != NULL)` 保证了当前 `p` 不为 `NULL`，结合后面 `p = p->next` 也是遍历整个链表的手段。所以循环体内检查的当前结点是 `p` 而不是 `p->next`。
2. `set_operation.c:89`

      ```c
       for(i=0;i<=A_size;i++){
      ```
      这个是常见的错误，循环一共执行了 `A_size+1` 次，而一共只有 `A_size` 个

3. `set_operation.c:107, 133`
      
      ```c
      if(!check(p1->number,B_head)){ //if this element is in B
      if(!check(p1->number,A2_head)){
      ```
      `check()` 在包含元素返回真，那么这里代码搞反了真假的条件

