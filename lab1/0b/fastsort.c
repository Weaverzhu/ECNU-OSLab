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

int makeUpNewRoom(rec_t *arr, int len) {
    static rec_t *tmp;
    tmp = (rec_t*)malloc(sizeof(rec_t) * len);
    memcpy(tmp, arr, sizeof(rec_t) * len);
    free(arr);
    arr = malloc(sizeof(rec_t) * (len + BLOCK));
    free(tmp);
    return len + BLOCK;
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


int main(int argc, char const *argv[])
{
    char *outFile = strdup(noSuchFile), c, *inputFile = strdup(noSuchFile);
    if (argc != 2) usage();
    outFile = strdup(argv[2]);
    inputFile = strdup(argv[1]);

    int inputfd = open(inputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

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
    len = makeUpNewRoom(arr, len);
    int rd;
    while (1) {
        rd = read(inputfd, &r, sizeof(rec_t));
        if (rd == 0) break; // EOF
        
    }
    

    return 0;
}
