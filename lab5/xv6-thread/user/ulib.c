#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"


#define LOCK_ACQUIRED 1
#define LOCK_FREE 0

void spinlock_init(spinlock_t *lock)
{
    *lock = LOCK_FREE;
}

void spinlock_acquire(spinlock_t *lock)
{
    while (*lock || xchg(lock, LOCK_ACQUIRED))
    {
        asm("pause");
    }
}

void spinlock_release(spinlock_t *lock)
{
    *lock = LOCK_FREE;
}


char*
strcpy(char *s, char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, void *vsrc, int n)
{
  char *dst, *src;
  
  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

int
thread_create(void (*start_routine)(void*), void *arg) 
{
  // fork();
  // critical !!!
  spinlock_t lock;
  spinlock_init(&lock);
  spinlock_acquire(&lock);

  // because the stack is not guaranteed to be page aligned, we need
  // more size to ensure it is
  void *stack = malloc(4096 * 2);
  spinlock_release(&lock);

  if (stack == NULL) return -1;

  // page roundup, ensure stack is page aligned
  stack = ((uint)stack + 4095) / 4096 * 4096;

  int pid = clone(start_routine, arg, stack);
  return pid;
}

// int
// thread_join()
// {
//   void *stack = malloc(sizeof(void*));
//   if (join(&stack) < 0) return -1;

//   spinlock_t lock;
//   spinlock_init(&lock);
//   spinlock_acquire(&lock);
//   free(stack);
//   spinlock_release(&lock);

//   return 0;
// }