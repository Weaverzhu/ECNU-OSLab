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
    struct __node_t *next;
} node_t;


typedef struct __header_t {
    int size, magic;
} header_t;

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
    head->next = NULL;
    head->size = size_of_region - sizeof(node_t);

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
        node_t *newhead = head + totsize;
        newhead->size = head->size - totsize;
        newhead->next = head->next;
        memset(head, 0, totsize); // clear the memory
        return 
    } else {

    }
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
