#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#define N 500 // hash table size
#define MAXS 100 // max allocate size

//------------------ some tools
#define setgreen fprintf(stderr, "\033[32;1m")
#define setwhite fprintf(stderr, "\033[39;0m")
#define setred fprintf(stderr, "\033[31;1m")
#define setblue fprintf(stderr, "\033[34;1m")
#define setyellow fprintf(stderr, "\033[33;1m")

// #define DEBUG

#ifdef DEBUG
#define dprintf(x...) fprintf(stderr, ##x)
#else
#define dprintf(...)
#endif
//------------------------------

char *pos[N], *std[N]; // test ptr table & std ptr table (using malloc in stdlib.h)
int size[N], m_error; // sizeof ptr allocated

int main(int argc, char const *argv[])
{
    clock_t st = clock();
    int seed = atoi(argv[1]); // random seed
    int n = atoi(argv[2]);  // num of operations
    int totsize = atoi(argv[3]); 

    srand(seed);

    assert(mem_init(totsize) == 0);

    int m = N, suc = 0, fail = 0;
    for (int i=0; i<n; ++i) {
        int p = rand() % m;
        int alloc = rand()%5;

        if (!alloc) { // free 
            
            if (pos[p] != NULL) {
                assert(std[p] != NULL);
                assert(mem_free(pos[p]) == 0);
                free(std[p]);
                printf("suc op %d, try to free\n", i);
                pos[p] = std[p] = NULL;
            } else {
                printf("nothing to do, op %d, try to free a null ptr\n", i);
            }

        } else { // allocate
            if (pos[p] != NULL) {
                assert(std[p] != NULL);
                mem_free(pos[p]);
                free(std[p]);
                pos[p] = std[p] = NULL;
            }
            
            size[p] = rand() % 100 + 1;
            int method = rand()%3;
            pos[p] = mem_alloc(size[p] * sizeof(char), rand()%3);
            if (m_error == 0) {
                printf("suc op %d, try to allocate %d, method %d\n", i, size[p], method);
                ++suc;
                for (int j=0; j<size[p]-1; ++j)
                    pos[p][j] = rand()%26 + 'a';
                pos[p][size[p]-1] = 0;
                std[p] = malloc(size[p] * sizeof(char));
                memcpy(std[p], pos[p], size[p]); // make a copy in std
            } else {
                setred;
                printf("failed op %d, try to allocate %d, method %d\n", i, size[p], method); // failed to allocate
                setwhite;
                ++fail;
                m_error = 0;
            }
        }
    }

    for (int i=0; i<N; ++i) {
        if (pos[i] != NULL) {
            int ret = memcmp(pos[i], std[i], size[i]);
            if (ret) {
                puts("Wrong answer, memory illegal modified");
                return 0;
            }
        }
    }
    printf("Accepted!, time used %.2fms\n, %d out of %d operations ran successfully\n", (1.0 * clock() - st) / CLOCKS_PER_SEC * 1000, suc, suc+fail);
    return 0;
}
