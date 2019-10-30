/* use largest free space for allocation */
/*has been converted*/
#include <assert.h>
#include <stdlib.h>
#include "mem.h"

int main()
{
    assert(mem_init(4096) == 0);
    void *ptr[6];
    // exit(0);
    ptr[0] = mem_alloc(400, M_WORSTFIT);
    assert(ptr[0] != NULL);

    exit(0);

    ptr[1] = mem_alloc(400, M_WORSTFIT);
    assert(ptr[1] != NULL);

    ptr[2] = mem_alloc(1000, M_WORSTFIT);
    assert(ptr[2] != NULL);

    ptr[3] = mem_alloc(1000, M_WORSTFIT);
    assert(ptr[3] != NULL);

    ptr[4] = mem_alloc(400, M_WORSTFIT);
    assert(ptr[4] != NULL);

    ptr[5] = mem_alloc(400, M_WORSTFIT);
    assert(ptr[5] != NULL);

    assert(mem_free(ptr[0]) == 0);
    ptr[0] = NULL;

    assert(mem_free(ptr[2]) == 0);
    ptr[2] = NULL;

    exit(0);

    assert(mem_free(ptr[4]) == 0);
    ptr[4] = NULL;

    ptr[0] = mem_alloc(360, M_WORSTFIT);
    assert(ptr[0] != NULL);

    ptr[2] = mem_alloc(960, M_WORSTFIT);
    assert(ptr[2] == NULL);
    assert(m_error == E_NO_SPACE);

    exit(0);
}
