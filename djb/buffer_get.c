#include "buffer.h"
#include <errno.h>

int buffer_getc(buffer* b, char *s) /*EXTRACT_INCL*/ {
  if (b->p >= b->n) {
    int r;
    while ((r=b->op(b->fd, b->x, b->a)) <0)
      if (errno != EINTR) break;

    if (r<=0) return r;
    b->p = 0; 
    b->n = r;
  }
  *s = b->x[b->p]; 
  b->p++;
  return 1;
}
