#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int mem_init(int size_of_region)
{
    // open the /dev/zero device
    int fd = open("/dev/zero", O_RDWR);

    // size_of_region (in bytes) needs to be evenly divisible by the page size
    void *ptr = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    // close the device (don't worry, mapping should be unaffected)
    close(fd);
    return 0;
}

int main(int argc, char const *argv[])
{
    printf("%d\n", getpagesize());
    return 0;
}
