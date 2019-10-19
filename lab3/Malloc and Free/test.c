#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem.h"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define MAGIC 19260817

int m_error;

typedef struct __node_t {
    int size;
    void *prev, *next;
} node_t;



typedef struct __header_t {
    int size, magic;
    void *prev, *next;
} header_t;

void setnode(node_t *t, int size, void *prev, void *next);
void setheader(header_t *t, int size, void *prev, void *next);
void *test_alloc(int size);
node_t *insertBeforeNode(int size, node_t *head, node_t *b);


//////////////////////////////////////////////////////////////

void setnode(node_t *t, int size, void *prev, void *next) {
    t->size = size;
    t->prev = prev;
    t->next = next;
}

void setheader(header_t *t, int size, void *prev, void *next) {
    t->size = size;
    t->magic = MAGIC;
    t->prev = prev;
    t->next = next;
}



node_t *head;
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

    // initialize the link list
    head = ptr;
    setnode(head, size_of_region - sizeof(node_t), NULL, NULL);


    // close the device (don't worry, mapping should be unaffected)
    close(fd);
    return 0;
}

void *test_alloc(int size) {
    // for test only
    return (void*)ptr + sizeof(node_t);
}

node_t *insertBeforeNode(int size, node_t *head, node_t *b) {
    int totsize = size + sizeof(header_t);
    if (b == head) {
        // set the new head node
        node_t *newhead = (void*)head + totsize;
        setnode(newhead, head->size - totsize, head->prev, head->next);

        // clear the memory
        memset(head, 0, totsize); 

        // return new head node
        return newhead;
    } else {
        node_t *newnode = (void*)b + totsize;
        setnode(newnode, b->size - totsize, b->prev, b->next);

        // clear the memory
        memset(b, 0, totsize);
        return head;
    }

    return head;
}

void *mem_alloc(int size, int style) {
    int need = size + sizeof(header_t);
    if (style == M_FIRSTFIT) {
        
        for (node_t *t=head; t!=NULL; t=t->next) {
            if (t->size >= need) {
                header_t header;
                header.magic = MAGIC;
                header.size = size;
                

            }
        }
    }
}

int mem_free(void *ptr) {
    header_t *t = ptr - sizeof(header_t);
    // int 

    return 0;
}

int main(int argc, char const *argv[])
{
    mem_init(4096);
    int *a = test_alloc(sizeof(int));
    printf("%p %d\n", ptr, sizeof(header_t));
    printf("%d\n", a[1021]);
    return 0;
}
