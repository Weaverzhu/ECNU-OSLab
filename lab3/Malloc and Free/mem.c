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

typedef struct
{
    int size;
    int magic;
} header_t;

typedef struct __node_t
{
    int size;
    struct __node_t *next;
} node_t;


node_t *head;
int mem_init(int size_of_region)
{
    static int called = 0;
    if (called || size_of_region <= 0) {
        m_error = E_BAD_ARGS;
        return -1;
    }
    ++called;
    
    int PAGE_SIZE = getpagesize();
    size_of_region = (size_of_region + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
    node_t *head = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE, -1, 0);

    if (head == NULL) return -1;

    head->size = 4096 - sizeof(node_t);
    head->next = NULL;

    return 0;
}

int mem_free(void *ptr)
{
    header_t *hptr = (header_t *)ptr - 1;
    if (hptr->magic != MAGIC)
    {
        m_error = E_BAD_POINTER;
    }
}

void *mem_alloc(int size, int style) {
    // test method
    for (node_t *t=head; t!=NULL; t=t->next) {
        if (t->size >= size) {
            if (t==head) {

            }
        }
    }


    /////////////////////////////////
}