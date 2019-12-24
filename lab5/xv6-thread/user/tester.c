/* check that address space size is updated in threads */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 0;
unsigned int size = 0;
spinlock_t lock, lock2;
int num_threads = 30;


#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   
   int tid = thread_create(worker, NULL);
   global = 1;
   
   while (global != 5);
   printf(1, "in main\n");
   thread_join();
   exit();
}

void
worker(void *arg_ptr) {
   printf(1, "in worker\n");
   global = 5;
   // exit();
}

