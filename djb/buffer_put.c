#include <errno.h>
#include "buffer.h"

#ifdef USE_BUFFER_LARGE_WRITE
#define X(a) (a>0xffffff)?0xffffff:a
#else
#define X(a) a
#endif

static int allwrite(buffer *b, const char *buf, unsigned int len) {
  int w;
  b->p = 0;
  while (len) {
    w = b->op(b->fd, buf, X(len));
    if (w == -1) {
      if (errno == EINTR) continue;
      return -1;
    }
    buf += w;
    len -= w;
  }
  return 0;
}

int buffer_flush(buffer *b) /*EXTRACT_INCL*/ {
  return allwrite(b,b->x,b->p);
}

int buffer_put(buffer *b, const char* s, unsigned int len) /*EXTRACT_INCL*/ {
  if (b->a-b->p < len) {
    if (buffer_flush(b)==-1) return -1;
    if (b->a < len) return allwrite(b, s, len);
  }
  byte_copy(b->x + b->p, len, s);
  b->p += len;
  return 0;
}
