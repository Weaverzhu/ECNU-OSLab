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
   ppid = getpid();

   int arg = 101;
   void *arg_ptr = &arg;

   spinlock_init(&lock);
   spinlock_init(&lock2);
   spinlock_acquire(&lock);
   spinlock_acquire(&lock2);

   int i;
   for (i = 0; i < num_threads; i++) {
      int thread_pid = thread_create(worker, arg_ptr);
      assert(thread_pid > 0);
   }

   size = (unsigned int)sbrk(0);

   while (global < num_threads) {
      spinlock_release(&lock);
      sleep(100);
      spinlock_acquire(&lock);
   }

   global = 0;
   sbrk(10000);
   size = (unsigned int)sbrk(0);
   spinlock_release(&lock);

   while (global < num_threads) {
      spinlock_release(&lock2);
      sleep(100);
      spinlock_acquire(&lock2);
   }
   spinlock_release(&lock2);


   for (i = 0; i < num_threads; i++) {
      int join_pid = thread_join();
      assert(join_pid > 0);
   }

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   spinlock_acquire(&lock);
   assert((unsigned int)sbrk(0) == size);
   global++;
   spinlock_release(&lock);

   spinlock_acquire(&lock2);
   assert((unsigned int)sbrk(0) == size);
   global++;
   spinlock_release(&lock2);

   exit();
}

