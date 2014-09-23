/* bootlog.c */
/* diet -Os gcc -o bootlog bootlog.c -Wall -W */

#include <unistd.h>
#include <alloca.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h> /* rename */
#include <sys/mman.h>
#include <sys/wait.h>
#include "../ninitfeatures.h"

static int xx_write(int fd, void *buf, size_t len);
static int do_io(void *buf, int len);
static int mk_backup();

struct mem {
  struct mem *x;
  unsigned int p;
};

static struct mem *last, *root;
static char *flag_rename, *name;
static int flagstderr, flagstdout, m;
int fd = -1;

#include "../pagesize_defs.h"
#define mem_size	sizeof(struct mem)
#define alloc_size	(page_size - mem_size)

void *mmap_alloc() {
  struct mem *m;
  m=mmap(0,page_size,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
  if (m==(struct mem*)-1) return 0;
  /* kernel must zeroed m->p and m->x */

  if (last) last->x = m;
  else root = m;
  last = m;
  return ((void *)m) + mem_size;
}

static void flush_root() {
  if (mk_backup()) return;
  if (fd < 0) fd = open(name, O_WRONLY | m, 0644);
  if (fd < 0) return;
  while (root) {
    void *x = root->x;
    xx_write(fd, ((void *)root) + mem_size, root->p);
    root->p = 0;
    if (root == last) break;
    munmap(root, page_size);
    root = x;
  }
}

static void loop(unsigned long len) {
  char *buf = 0;
  int r;
  if (flag_rename) {
    char *d, *s = name;
    d = flag_rename = alloca(str_len(name) + 5);
    while (*s) *d++ = *s++;
    d[0] = '~';
    d[1] = 0;
  }
  while (len) {
    if (buf == 0 && (buf=mmap_alloc()) == 0) break;

    if (last->p >= alloc_size) {
      flush_root();
      if (last->p) { buf = 0; continue; }
    }

    r = do_io(buf + last->p, alloc_size - last->p);
    if (r==0) break;
    if (r<0) continue;

    if ((unsigned long)r > len) r = len;
    last->p  += r;
    len -= r;
  }

  if (buf==0 || len==0) {
    char tmp[1024];
    while (do_io(tmp,sizeof(tmp)));
  }

  mk_backup();
  flag_rename = 0;
  flush_root();
  fsync(fd);
  close(fd);
}

#include "bootlog.h"
