#include <sys/types.h>
#define N	((void *)-1)

extern void *SYS_brk(void *x);
static void *cur = N;

void *sbrk(ssize_t incr) {
  void *t=0, *old=N;

  if (cur == N) {
  again:
    cur = SYS_brk(t);
    if (cur == N) return cur;
  }

  if (old == N) {
    old = cur;
    if (incr) {
      t = old + incr;
      goto again;
    }
  }

  return old;
}
