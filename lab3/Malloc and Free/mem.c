#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem.h"

#define handle_error(msg)   \
    do                      \
    {                       \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

#define MAGIC 0x19260817

//------------------ some tools
#define setgreen fprintf(stderr, "\033[32;1m")
#define setwhite fprintf(stderr, "\033[39;0m")
#define setred fprintf(stderr, "\033[31;1m")
#define setblue fprintf(stderr, "\033[34;1m")
#define setyellow fprintf(stderr, "\033[33;1m")

// #define DEBUG

#ifdef DEBUG
#define dprintf(x...) fprintf(stderr, ##x)
#else
#define dprintf(...)
#endif
//------------------------------

typedef long long Align;
typedef unsigned uint;
typedef unsigned long long ULL;

union node {
    struct
    {
        union node *next;
        uint size;
    } s;
    Align x;
};

union header {
    struct
    {
        uint size, magic;
    } s;
    Align x;
};

typedef union header Header;
typedef union node Node;

void setNode(Node *n, uint size);
void setHeader(Header *h, uint size);
void connect(Node *prev, Node *next);

int memdump_id;
void dblock(void *l, void *r, int is_free)
{
    printf("[%d] ", memdump_id++);
    if (is_free)
    {
        setgreen;
        printf("Memory available: ");
        printf("%p -- %p, size %lu\n", l, r - 1, r - l);
    }
    else
    {
        setred;
        printf("Memory allocated: ");
        printf("%p -- %p, size %lu\n", l, r - 1, r - l);
    }

    setwhite;
}

static Node *base;
int m_error;
static int called = 0;
void *hptr;
int totsize;

////////////////////////////function////////////////////////////

void connect(Node *prev, Node *next)
{
    if (prev != NULL)
        prev->s.next = next;
}

void setNode(Node *n, uint size)
{
    n->s.size = size;
}

void setHeader(Header *h, uint size)
{
    h->s.size = size;
    h->s.magic = MAGIC;
}

int mem_init(int size_of_region)
{

    if (size_of_region <= 0 || called)
    {
        m_error = E_BAD_ARGS;
        return -1;
    }

    const int PAGE_SIZE = getpagesize();
    size_of_region = (size_of_region + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
    totsize = size_of_region;
    int fd = open("/dev/zero", O_RDWR);

    // size_of_region (in bytes) needs to be evenly divisible by the page size
    hptr = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (hptr == MAP_FAILED)
    {
        m_error = E_BAD_ARGS;
        return -1;
    }

    ++called;

    // initialize the link list, the first node is a node_t
    base = (Node *)hptr;
    setNode(base, size_of_region - sizeof(Node));

    // close the device (don't worry, mapping should be unaffected)
    close(fd);
    return 0;
}

void *mem_alloc(int size, int style)
{
    if (!called)
    {
        m_error = E_BAD_ARGS;
        return NULL;
    }

    size = (size + sizeof(Align) - 1) / sizeof(Align) * sizeof(Align);

    uint need = size + sizeof(Header);

    Node *n = NULL, *prevp = NULL, *cn = NULL, *cp = NULL;
    uint bestsize;

    switch (style)
    {
    case M_FIRSTFIT:
        for (n = base; n != NULL; prevp = n, n = n->s.next)
        {
            if (n->s.size + sizeof(Node) >= need)
            {
                break;
            }
        }
        break;

    case M_BESTFIT:
        cn = NULL;
        cp = NULL;
        bestsize = 0xffffffff;
        for (cn = base; cn != NULL; cp = cn, cn = cn->s.next)
        {
            if (cn->s.size < bestsize && cn->s.size + sizeof(Node) >= need)
            {
                n = cn;
                prevp = cp;
                bestsize = n->s.size;
            }
        }
        break;

    case M_WORSTFIT:
        cn = NULL;
        cp = NULL;
        bestsize = 0;
        for (cn = base; cn != NULL; cp = cn, cn = cn->s.next)
        {
            if (cn->s.size >= bestsize && cn->s.size + sizeof(Node) >= need)
            {
                n = cn;
                prevp = cp;
                bestsize = n->s.size;
            }
        }
        break;
    default:
        break;
    }

    if (n == NULL)
    {
        m_error = E_NO_SPACE;
        return NULL;
    }
    uint remain = n->s.size + sizeof(Node) - need;
    Node *newnode;
    if (remain >= sizeof(Node))
    {
        newnode = (void *)n + need;
        setNode(newnode, n->s.size - need);
        connect(newnode, n->s.next);
    }
    else
    {
        newnode = n->s.next;
        size += remain;
    }

    if (prevp == NULL)
    {
        base = newnode;
    }
    else
    {
        connect(prevp, newnode);
    }

    Header *h = (Header *)n;
    setHeader(h, size);

    return (void *)h + sizeof(Header);
}

void outputAllocated(void *base, void *ed) {
    int num = 0;
    while (base < ed)
    {
        ++num;
        Header *h = base;
        dblock((void*)h, (void*)h + sizeof(Header) + h->s.size, 0);
        base = base + sizeof(Header) + h->s.size;
    }
    // printf("dbg: get %d\n", num);
}

void mem_dump()
{

    printf("------------mem_dump--------------\n");
    if (!called)
    {
        setred;
        printf("Oops, you haven't successfully called mem_init yet.\n");
        setwhite;
    }
    else if (base == NULL)
    {
        setred;
        printf("Oops, it seems that all space has been allocated.\n");
        setwhite;
    }
    else
    {
        memdump_id = 0;
        if (hptr < (void *)base)
        {
            outputAllocated(hptr, base);
        }
        Node *p, *prevp = NULL;
        for (p = base; p != NULL; prevp = p, p = p->s.next)
        {
            if (prevp != NULL) {
                outputAllocated((void *)prevp + sizeof(Node) + prevp->s.size, p);
            }
            dblock((void *)p, (void *)p + sizeof(Node) + p->s.size, 1);
        }
        if ((void*)prevp+sizeof(Node)+prevp->s.size < hptr + totsize) {
            outputAllocated((void *)prevp + sizeof(Node) + prevp->s.size, hptr + totsize);
        }
    }

    printf("----------------------------------\n");
}

int mem_free(void *ptr)
{
    if (!called)
    {
        m_error = E_BAD_ARGS;
        return -1;
    }
    Header *p = (void *)ptr - sizeof(Header);
    if (p->s.magic != MAGIC)
    {
        m_error = E_BAD_POINTER;
        return -1;
    }

    Node *t = ptr - sizeof(Header);
    setNode(t, p->s.size + sizeof(Header) - sizeof(Node));

    // merge with left
    Node *lp = NULL, *rp = NULL;
    for (Node *n = base; n != NULL; n = n->s.next)
    {
        if ((void *)n + n->s.size + sizeof(Node) <= (void *)t)
        {
            lp = n;
        }
        else
        {
            rp = n;
            break;
        }
    }

    if (lp == NULL)
    {
        connect(t, base);
        base = t;
    }
    else if ((void *)lp + lp->s.size + sizeof(Node) == (void*)t)
    {
        lp->s.size += sizeof(Node) + t->s.size;
        t = lp;
    }
    else
    {
        connect(t, rp);
        connect(lp, t);
    }

    if (rp != NULL && (void *)t + sizeof(Node) + t->s.size == (void*)rp)
    {
        t->s.size += sizeof(Node) + rp->s.size;
        connect(t, rp->s.next);
        }
    return 0;
}

///////////////////////////////////////////////////////////////
