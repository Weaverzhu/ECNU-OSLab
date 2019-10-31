
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"


int main(int argc, char *argv[])
{
    mem_init(4096);
    int *a = mem_alloc(4080, M_BESTFIT);
    mem_dump();
    mem_free(a);
    mem_dump();
    return 0;
}
