#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char const *argv[])
{
    char a[4096];
    memset(a, 'a', sizeof(a));
    printf(1, "%s\n", a);
    exit();
}