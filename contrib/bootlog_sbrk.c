/* bootlog.c */
/* diet -Os gcc -o bootlog bootlog.c -Wall -W */

#include <unistd.h>
#include <alloca.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h> /* rename */
#include <sys/wait.h>
#include "../ninitfeatures.h"
#define BRKINCR 1024

static int xx_write(int fd, void *buf, size_t len);
static int do_io(void *buf, int len);
static int mk_backup();

static char *root, *last, *end;
static char *flag_rename, *name;
static int flagstderr, flagstdout, m;
int fd = -1;

static void *setbrk(ssize_t incr) {
  void *x = sbrk(incr);
  if ((void *)-1 == x) return x;
  if (root == 0) root = last = x;
  end = x + incr;
  return 0;
}

static void flush_root() {
  if (mk_backup()) return;
  if (fd < 0) fd = open(name, O_WRONLY | m, 0644);
  if (fd < 0 || root==0) return;
  xx_write(fd, root, last-root);
  last = root;
  setbrk((ssize_t)BRKINCR - (end-root));
}

static void loop(unsigned long len) {
  int r;
  if (flag_rename) {
    char *d, *s = name;
    d = flag_rename = alloca(str_len(name) + 5);
    while (*s) *d++ = *s++;
    d[0] = '~';
    d[1] = 0;
  }
  while (len) {
    if (last >= end) flush_root();
    if (last >= end && setbrk(BRKINCR)) break;

    r = do_io(last, end - last);
    if (r==0) break;
    if (r<0) continue;

    if ((unsigned long)r > len) r = len;
    last  += r;
    len -= r;
  }

  if (last >= end || len==0) {
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
