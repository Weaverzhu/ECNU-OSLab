# Project 2a: Malloc and Free

## 存储内存空间分配情况

我们使用链表来存放每个连续的空余空间

```c
typedef struct __node_t {
    int size;
    struct __node_t *next;
} node_t;
```

## 表示占用空间

我们需要记录每部分空间的占用情况，否则，当内存空间被清除时我们将不知道该释放多少空间

```c
#define MAGIC 19260817

typedef struct __header_t {
    int size, magic;
} header_t;
```

其中 `MAGIC` 是校验位，如果 `magic` 发生改变，那么我们有理由相信内存发生错误。

需要注意的是，在 64 位计算机上一个指针的大小有 8 个字节，所以链表节点占用 12 字节而空间信息结构体将占用 4 个 字节，因此最小的分配空间应该不低于 4 个 字节，否则清理内存后可能会出现反而占用更多内存的现象。$x \le 3 \Rightarrow 8+x < 12$

