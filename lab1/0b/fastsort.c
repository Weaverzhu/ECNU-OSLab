#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <getopt.h>


#define BLOCK 10000

char msg[100];

rec_t* makeUpNewRoom(rec_t *arr, int *plen) {
    static rec_t *tmp;
    int len = *plen;
    if (len) {
        tmp = (rec_t*)malloc(sizeof(rec_t) * len);
        memcpy(tmp, arr, sizeof(rec_t) * len); // move to tmp
        free(arr);
    }
    
    arr = (rec_t*)malloc(sizeof(rec_t) * (len + BLOCK)); // allocate a bigger array
    if (len) {
        memcpy(arr, tmp, sizeof(rec_t) * len);
        free(tmp); // clear the tmp
    }
    *plen = len + BLOCK;
    return arr;
}

const char noSuchFile[] = "/no/such/file";

void usage()
{
	fprintf(stderr, "Usage: fastsort inputfile outputfile\n");
	exit(1);
}

void err(char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

int cmp(const void *a, const void *b) {
    rec_t *pa = (rec_t*)a, *pb = (rec_t*)b;
    return pa->key - pb->key;
}

int main(int argc, char const *argv[])
{
    char *outFile = strdup(noSuchFile), c, *inputFile = strdup(noSuchFile);


    if (argc != 3) usage();
    outFile = strdup(argv[2]);
    inputFile = strdup(argv[1]);

    int inputfd = open(inputFile, O_RDONLY);

    if (inputfd < 0) {
        sprintf(msg, "Error: Cannot open file %s\n", inputFile);
        err("failed to open inputfile");
    }

    int outputfd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    if (outputfd < 0) {
        sprintf(msg, "Error: Cannot open file %s\n", outFile);
        err(msg);
    }


    rec_t *arr, r;
    int len = 0, size = 0;
    
    arr = makeUpNewRoom(arr, &len);
    if (arr == NULL) {
        puts("FUCK");
        return 0;
    }
    int rd;
    while (1) {
        rd = read(inputfd, &r, sizeof(rec_t));
        if (rd == 0) break; // EOF
        if (size == len) arr = makeUpNewRoom(arr, &len);
        arr[size++] = r;
        
    }
    qsort(arr, size, sizeof(arr[0]), cmp);
    for (int i=0; i<size; ++i) {
        write(outputfd, &arr[i], sizeof(rec_t));
    }
    close(inputfd);
    close(outputfd);
    // puts("completed");
    fprintf(stderr, "sort completed\n");
    return 0;
}
