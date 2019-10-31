/*has been converted*/
/* check for coalesce free space (last chunk)*/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "mem.h"

#define HEADER (16) // Assumes Headers will never be BIGGER than this
#define SLACK (32)

int main()
{
    mem_init(4090);
    char *a = mem_alloc(10, M_BESTFIT);
    char *b = mem_alloc(20, M_WORSTFIT);
    char *c = mem_alloc(30, M_FIRSTFIT);
    mem_free(b);
    mem_dump();
    exit(0);
}
