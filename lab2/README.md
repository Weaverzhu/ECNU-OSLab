# Project 1a: A Unix Shell

完成一个 `shell` 需要考虑各个模块的实现方式，以下是 `mysh` 大致实现思路

## Multi-process


### 普通命令

使用 `execvp()` 命令，可以在程序自动在相关环境变量下搜索程序位置并且执行。注意执行后程序将自动关闭，因此应该使用子程序。
