#ifndef INIT_MMAP

#define depends_alloca(N)	alloca(N)
#define read_alloca(N)		alloca(N)
#define INIT_FREE_MEMBLOCKS(X)	X
#define free_execve(...)	execve(__VA_ARGS__)


#else
#include <sys/mman.h>
#include <sys/shm.h>	/* for PAGE_SIZE */
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define depends_alloca(N)	mmap_alloca(N)
#define read_alloca(N) \
  ((read_buf) ? read_buf : (read_buf=mmap_alloca(N)))

#define INIT_FREE_MEMBLOCKS(X)	read_buf=0; mmap_free(); X
#define free_execve(...)	mmap_free(); execve(__VA_ARGS__)

#define mem_x_size ((sizeof(struct memalloc) + 16) & ~15)
static struct memalloc *mroot;
static void *read_buf;

static void *mmap_alloca(uint32t n) {
  n = (n + 16) & ~15;
  if (mroot == 0 || n + mem_x_size >= mroot->r) {
    struct memalloc *m;
    uint32t len = (n + (PAGE_SIZE + mem_x_size)) & ~(PAGE_SIZE-1);
    m = mmap(0,len,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
    if (m == (struct memalloc *)-1) return 0;
    m->x = mroot;	/* previous block */
    m->a = len;
    m->r = len;
    mroot = m;
  }
  mroot->r -= n;
  return (void *)mroot + mroot->r;
}
#undef mem_x_size

static void mmap_free() {
  while (mroot) {
    void *x = mroot->x;
    munmap(mroot,mroot->a);
    mroot = x;
  }
}
#endif
