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
#define MAGIC_N 0X12345678
#define MIN_ALLOC 8

#define F_INITIALIZE_Z

int m_error;

typedef struct __node_t {
    void *prev, *next;
    int size, magic
} node_t;

typedef struct __node_t header_t;

void setnode(node_t *t, int size, void *prev, void *next);
void setheader(header_t *t, int size, void *prev, void *next);
void *test_alloc(int size);
void *insertBeforeNode(int size, node_t *head, node_t *b);
void connect(void *a, void *b);
int isheader(void *a);

//////////////////////////////////////////////////////////////

int isheader(void *a) { // use magic to tell if a points to a header
    header_t *pa = (header_t*)a;
    int g = pa->magic;
    return g == MAGIC;
}

void connect(void *a, void *b) {
    // a & b are two nodes
    if (a != NULL) {
        header_t *pa = (header_t*)a;
        pa->next = a;
    }
    if (b != NULL) {
        header_t *pb = (header_t*)b;
        pb->prev = b;
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



void *head;
void *ptr;


int mem_init(int size_of_region)
{
    if (size_of_region <= 0) {
        perror("bad argument: size_of_region <= 0");
        m_error = E_BAD_ARGS;
        return -1;
    }

    const int PAGE_SIZE = getpagesize();
    size_of_region = (size_of_region + PAGE_SIZE - 1) / PAGE_SIZE;
    int fd = open("/dev/zero", O_RDWR);

    // size_of_region (in bytes) needs to be evenly divisible by the page size
    ptr = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);


    if (ptr == MAP_FAILED) { 
        perror("mmap");
        m_error = E_BAD_ARGS;
        exit(1); 
    }

    // initialize the link list, the first node is a node_t
    head = (node_t*)ptr;
    setnode(head, size_of_region - sizeof(node_t), NULL, NULL);


    // close the device (don't worry, mapping should be unaffected)
    close(fd);
    return 0;
}

void *test_alloc(int size) {
    // for test only
    return (void*)ptr + sizeof(node_t);
}

void *insertBeforeNode(int size, node_t *head, node_t *b) {
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
        setnode(newnode, bsize - totsize, NULL, NULL);
        connect(newnode, bnext);
        connect(newheader, newnode);
    } else {
        connect(newheader, bnext);
    }

#ifdef F_INITIALIZE_Z
    memset(b, 0, totsize);
#endif

    return head;
}

header_t *makeheader(void *ptr, node_t *b, int size) {
    header_t *hptr = ptr;
    setheader(hptr, size, b->prev, b);
    
    b->prev = hptr;
    return hptr;
}

void *mem_alloc(int size, int style) {

    // size is alligned to 8 bytes
    if (size < MIN_ALLOC) size = MIN_ALLOC;


    int need = size + sizeof(header_t);
    if (style == M_FIRSTFIT) {

        for (node_t *t=head; t!=NULL; t=t->next) {
            if (isheader(t)) continue;
            if(t->size + sizeof(node_t) >= need) {
                head = insertBeforeNode(size, head, t);
                return t;
            }
        }
    }
}

int mem_free(void *ptr) {
    header_t *t = ptr - sizeof(header_t);
    // int 
    

    return 0;
}

void mem_dump() {
    
}

int main(int argc, char const *argv[])
{
    mem_init(4096);
    int *a = test_alloc(sizeof(int));
    printf("%p %d\n", ptr, sizeof(header_t));
    printf("%d\n", a[1021]);
    return 0;
}
