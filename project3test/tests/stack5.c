/* stack should grow automatically on a page fault, until 5 pages above heap */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void *)0)

#define assert(x)                                 \
    if (x)                                        \
    {                                             \
    }                                             \
    else                                          \
    {                                             \
        printf(1, "%s: %d ", __FILE__, __LINE__); \
        printf(1, "assert failed (%s)\n", #x);    \
        printf(1, "TEST FAILED\n");               \
        exit();                                   \
    }

int main(int argc, char *argv[])
{
    int ppid = getpid();
    int pid = fork();
    if (pid == 0)
    {
        uint sz = (uint)sbrk(0);
        sz = (sz + 4096 - 1) / 4096 * 4096;
        char *STACK = (char *)(159 * 4096);

        // if the fault is not handled, we will not reach the print
        while (STACK >= (char *)sz + 20 * 4096)
        {
            *STACK = 'a';
            STACK -= 4096;
        }

        char *a = malloc(15 * 4096);
        assert(a != NULL);
        printf(1, "a=%x, sz=%x\n", a, sz);
        char *b = (void*)a-1;
        *b = 'a';
        printf("b=%x, sz=%x\n", b, sz);

        printf(1, "TEST PASSED\n");
        // printf(1, "TEST FAILED\n");
        // kill(ppid);
        exit();
    }
    else
    {
        wait();
    }
    
    exit();
}
