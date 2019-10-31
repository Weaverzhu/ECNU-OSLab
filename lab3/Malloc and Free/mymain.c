#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "mem.h"

int main()
{
    mem_init(4090);
    mem_init(4096);
    assert(m_error == E_BAD_ARGS);


    int *a = mem_alloc(sizeof(int) * 8, M_BESTFIT);
    a[7] = 1;
    mem_dump();

    char *b = mem_alloc(7, M_FIRSTFIT); // align to 8
    char *c = mem_alloc(4080, M_WORSTFIT);
    assert(m_error == E_NO_SPACE);
    c = mem_alloc(20, M_WORSTFIT);
    mem_dump();

    mem_free(a);
    mem_free(c);
    mem_dump();

    mem_free((void*)b+1);
    assert(m_error == E_BAD_POINTER);

    mem_free(b);
    mem_dump();

    exit(0);
}
