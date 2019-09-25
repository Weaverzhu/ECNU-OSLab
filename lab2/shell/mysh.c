#include "config.h"
#include <fcntl.h>
#include <unistd.h>



int main(int argc, char const *argv[])
{
    while (1) {
        static char cmdline[SIZE];
        PRINT_HEADER;
        read(STDIN_FILENO, cmdline, SIZE);

    }
    return 0; 
}
