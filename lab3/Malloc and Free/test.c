#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mem.h"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define setindent(x) do { for (int i=0; i<x; ++i) putchar(' '); } while(0)

#define MAGIC 0x19260817
#define MAGIC_N 0X12345678
#define MIN_ALLOC 8

#define F_INITIALIZE_Z
#define setgreen fprintf(stderr, "\033[32;1m")
#define setwhite fprintf(stderr, "\033[39;0m")
#define setred fprintf(stderr, "\033[31;1m")
#define setblue fprintf(stderr, "\033[34;1m")
#define setyellow fprintf(stderr, "\033[33;1m")

// #define DEBUG

#ifdef DEBUG
#define dbgd(x) do { setblue; fprintf(stderr, "%s -> %d\n", #x, x); setwhite;} while (0)
#define dbgp(x) do { setyellow; fprintf(stderr, "%s -> %p\n", #x, x); setwhite; } while (0);
#else
#define dbgd(x)
#define dbgp(x)
#endif

int m_error;

typedef struct __node_t {
    void *prev, *next;
    int size, magic;
} node_t;

node_t *head;

typedef struct __node_t header_t;

void setnode(node_t *t, int size, void *prev, void *next);
void setheader(header_t *t, int size, void *prev, void *next);
void *test_alloc(int size);
void *insertBeforeNode(int size, node_t *head, node_t *b);
void connect(void *a, void *b);
int isheader(void *a);

/////////////////////////function/////////////////////////////////////

int isheader(void *a) { // use magic to tell if a points to a header
    header_t *pa = (header_t*)a;
    int g = pa->magic;
    return g == MAGIC;
}

void connect(void *a, void *b) {
    // a & b are two nodes
    if (a != NULL) {
        header_t *pa = (header_t*)a;
        pa->next = b;
    }
    if (b != NULL) {
        header_t *pb = (header_t*)b;
        pb->prev = a;
    }
    
}

void setnode(node_t *t, int size, void *prev, void *next) {
    t->size = size;
    t->prev = prev;
    t->next = next;
    t->magic = MAGIC_N;
}

void setheader(header_t *t, int size, void *prev, void *next) {
    t->size = size;
    t->magic = MAGIC;
    t->prev = prev;
    t->next = next;
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
    void *ptr = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);


    if (ptr == MAP_FAILED) { 
        perror("mmap");
        m_error = E_BAD_ARGS;
        exit(1); 
    }

    // initialize the link list, the first node is a node_t
    dbgd(size_of_region);
    head = (node_t*)ptr;
    setnode(head, size_of_region - sizeof(node_t), NULL, NULL);


    // close the device (don't worry, mapping should be unaffected)
    close(fd);
    return 0;
}

void *test_alloc(int size) {
    // for test only
    return (void*)head + sizeof(node_t);
}

void *insertBeforeNode(int size, node_t *head, node_t *b) {
    dbgp(head); dbgp(b);

    int totsize = size + sizeof(header_t);

    // decide whether to add new node
    int f_newnode = b->size - totsize >= sizeof(node_t) + MIN_ALLOC;
    
    // b will be changed, so save b here
    void *bprev = b->prev, *bnext = b->next;
    int bsize = b->size;

    // set new header
    header_t *newheader = (header_t*)b;
    setheader(newheader, size, bprev, b);
    connect(bprev, newheader);

    if (f_newnode) {
        node_t *newnode = b + totsize;
        dbgp(newnode); dbgp(newheader);
        setnode(newnode, bsize - totsize, NULL, NULL);
        connect(newnode, bnext);
        connect(newheader, newnode);
        dbgp(head); dbgp(newheader);
        dbgp(newnode);
        dbgp(head->next);
        dbgd(newheader->size);
    } else {
        connect(newheader, bnext);
    }

#ifdef F_INITIALIZE_Z
    memset(b + sizeof(node_t), 0, size);
#endif

    return head;
}

header_t *makeheader(void *ptr, node_t *b, int size) { // deprecated
    header_t *hptr = ptr;
    setheader(hptr, size, b->prev, b);
    
    b->prev = hptr;
    return hptr;
}

void *mem_alloc(int size, int style) {

    // size is alligned to 8 bytes
    if (size < MIN_ALLOC) size = MIN_ALLOC;


    int need = size + sizeof(header_t);
    // if (style == M_FIRSTFIT) {

        for (node_t *t=head; t!=NULL; t=t->next) {
            if (isheader(t)) continue;
            if(t->size + sizeof(node_t) >= need) {
                dbgp(t);
                head = insertBeforeNode(size, head, t);
                return t;
            }
        }
    // }
    return NULL;
}

int mem_free(void *ptr) {
    // header_t *t = ptr - sizeof(header_t);
    // int 
    

    return 0;
}

void mem_dump() {

    // redirect stdout to stderr, backup stdout
    int ostd = -1; dup2(STDIN_FILENO, ostd);
    dup2(STDERR_FILENO, STDOUT_FILENO);

    int allocm = 0, freem = 0;
    
    puts("");
    puts("============mem dump: output mem alloc info ============");
    int i = 0;

    for (node_t *t=head; t!=NULL; t=t->next) {
        ++i;
        printf("[%d] ", i);
        if (isheader(t)) {
            setred;
            printf("mem allocated here: ");
            allocm += t->size;
        } else {
            setgreen;
            printf("mem available here: ");
            freem += t->size;
        }
        dbgp(t);
        dbgd(t->size);
        printf("%p to %p, size %d\n", (void*)t+sizeof(node_t), (void*)t+sizeof(node_t)+t->size-1, t->size);
    
    }
    
    // reset color
    setwhite;
    printf("=====tot mem available: %d, tot mem allocated: %d=====\n\n", freem, allocm);


    // reset stdout
    dup2(ostd, STDOUT_FILENO);
}

int main(int argc, char const *argv[])
{
    mem_init(4096);
    mem_dump();

    int *a = mem_alloc(sizeof(int) * 4, M_BESTFIT);

    mem_dump();
    return 0;
}
