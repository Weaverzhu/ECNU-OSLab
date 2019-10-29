#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem.h"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define MAGIC 0x19260817


//------------------ some tricks
#define setgreen fprintf(stderr, "\033[32;1m")
#define setwhite fprintf(stderr, "\033[39;0m")
#define setred fprintf(stderr, "\033[31;1m") 
#define setblue fprintf(stderr, "\033[34;1m")
#define setyellow fprintf(stderr, "\033[33;1m")
#define dprintf(x...) fprintf(stderr, ##x)
//------------------------------

typedef long Align;
typedef unsigned uint;
typedef unsigned long long ULL;

union node {
    struct {
        union node *next;
        uint size;
    } s;
    Align x;
};

union header {
    struct {
        union header *next;
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
void dblock(void *l, void *r, int is_free) {
    dprintf("[%d] ", memdump_id++);
    if (is_free) {
        setgreen;
        dprintf("Memory available: ");
        dprintf("%p -- %p, size %lu-%lu\n", l, r-1, r-l, sizeof(Node));
    } else {
        setred;
        dprintf("Memory allocated： ");
        dprintf("%p -- %p, size %lu+%lu\n", l, r-1, r-l-sizeof(Header), sizeof(Header));
    }
    
    setwhite;
}

static Node *base;
int m_error;

void *ptr;

////////////////////////////function////////////////////////////

void connect(Node *prev, Node *next) {
    if (prev != NULL) prev->s.next = next;
}


void setNode(Node *n, uint size) {
    n->s.size = size;
}

void setHeader(Header *h, uint size) {
    h->s.size = size;
    h->s.magic = MAGIC;
}

int mem_init(int size_of_region)
{

    if (size_of_region <= 0) {
        perror("bad argument: size_of_region <= 0");
        m_error = E_BAD_ARGS;
        return -1;
    }

    const int PAGE_SIZE = getpagesize();
    size_of_region = (size_of_region + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
    int fd = open("/dev/zero", O_RDWR);

    // size_of_region (in bytes) needs to be evenly divisible by the page size
    ptr = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);


    if (ptr == MAP_FAILED) { 
        perror("mmap");
        m_error = E_BAD_ARGS;
        exit(1); 
    }

    // initialize the link list, the first node is a node_t
    base = (Node*)ptr;
    setNode(base, size_of_region - sizeof(Node));


    // close the device (don't worry, mapping should be unaffected)
    close(fd);
    return 0;
}

void * mem_alloc(int size, int style) {
    size = (size + sizeof(Align) - 1) / sizeof(Align) * sizeof(Align);

    uint need = size + sizeof(Header);

    Node *n, *prevp = NULL, *cn, *cp;
    uint bestsize;

    switch (style)
    {
    case M_FIRSTFIT:
        for (n=base; n!=NULL ;prevp=n,n=n->s.next) {
            if (n->s.size + sizeof(Node) >= need) {
                break;
            }
        }
        break;

    case M_BESTFIT:
        cn = NULL; cp = NULL;
        bestsize = 0xffffffff;
        for (cn=base; cn!=NULL; cp=cn, cn=cn->s.next) {
            if (cn->s.size < bestsize && cn->s.size + sizeof(Node) >= need) {
                n = cn;
                prevp = cp;   
            }
        }
        break;
    
    case M_WORSTFIT:
        cn = NULL; cp = NULL;
        bestsize = 0;
        for (cn=base; cn!=NULL; cp=cn, cn=cn->s.next) {
            if (cn->s.size > bestsize && cn->s.size + sizeof(Node) >= need) {
                n = cn;
                prevp = cp;   
            }
        }
        break;
    
    default:
        break;
    }

    if (n == NULL) {
        m_error = E_NO_SPACE;
        return NULL;
    }

    uint remain = n->s.size - need;
    dprintf("remain = %u\n", remain);
    Node *newnode;
    if (remain >= sizeof(Node) + sizeof(Align)) {
        newnode = (void*)n + need;
        dprintf("size = %d, need = %d, sizeof(Node) = %lu\n", size, need, sizeof(Node));
        setNode(newnode, n->s.size - need);
        connect(newnode, n->s.next);
    } else {
        newnode = n->s.next;
    }

    if (prevp == NULL) {
        base = newnode;
    } else {
        connect(prevp, newnode);
    }

    dprintf("base = %p, ptr = %p, base->s.size = %u\n", base, ptr, base->s.size);
    
    Header *h = n;
    setHeader(h, size);

    dprintf("header->magic = %u, header = %p\n", h->s.magic, h);

    return (void*)h + sizeof(Header);
}


void mem_dump() {
    memdump_id = 0;

    dprintf("------------mem_dump--------------\n");
    if (ptr < (void*)base) {
        dblock(ptr, base, 0);
    }
    Node *p, *prevp = NULL;
    for (p=base; p!=NULL; p=p->s.next) {
        if (prevp!=NULL) dblock((void*)prevp, p, 0);
        dblock((void*)p, (void*)p+sizeof(Node)+p->s.size, 1);
    }
    dprintf("----------------------------------\n");
}


// int mem_free(void *ptr) {
//     dbgp(ptr); dbgp(head);
//     // return 0;
//     node_t *p = ptr - sizeof(node_t);
    
//     assert(p->magic == MAGIC_H);

//     node_t *cur = p;

//     // merge with left free space
//     if (p->prev != NULL && !isheader(p->prev)) {
//         cur = p->prev;
//         cur->size += p->size + sizeof(node_t);
//         connect(cur, p->next);
//     }

//     // merge with right free space
//     if (p->next != NULL && !isheader(p->next)) {
//         node_t *pnext = (node_t*)p->next;
//         cur->size += pnext->size + sizeof(node_t);
//         connect(cur, pnext->next);
//     }
    
//     // change the flag to node_t
//     cur->magic = MAGIC_N;
//     return 0;
// }

int mem_free(void *ptr) {
    dprintf("ptr = %p\n", ptr);

    Header *p = (void*)ptr - sizeof(Header);
    if (p->s.magic != MAGIC) {
        m_error = E_BAD_POINTER;
        return;
    }

    Node *t = ptr - sizeof(Header);
    setNode(t, p->s.size + sizeof(Header) - sizeof(Node));

    // merge with left
    Node *lp = NULL, *rp = NULL;
    for (Node *n=base; n!=NULL; n=n->s.next) {
        if ((void*)n + n->s.size + sizeof(Header) < t) {
            lp = n;
        } else {
            rp = n;
            break;
        }
    }

    dprintf("lp=%p, rp=%p\n", lp, rp);

    if (lp == NULL) {
        connect(t, base);
        base = t;
    } else if ((void*)lp + sizeof(Node) + lp->s.size == t) {
        lp->s.size += sizeof(Node) + t->s.size;
        t = lp;
    } else {
        connect(t, rp);
        connect(lp, t);
    }

    // dprintf("%p %p\n", (void*)t + sizeof(Node) + t->s.size + 1, rp);
    dprintf("%u\n", rp->s.size);
    if (rp != NULL && (void*)t + sizeof(Node) + t->s.size == rp) {
        t->s.size += sizeof(Node) + rp->s.size;
        connect(t, rp->s.next);
    }

}

///////////////////////////////////////////////////////////////

int main(int argc, char const *argv[])
{
    mem_init(1);
    mem_dump();
    int *a = mem_alloc(sizeof(int), M_FIRSTFIT);
    mem_dump();
    mem_free(a);
    mem_dump();

    dprintf("m_error = %d\n", m_error);
    return 0;
}