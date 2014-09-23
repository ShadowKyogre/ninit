#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer {
  char *x;        /* actual buffer space */
  unsigned int p; /* current position */
  unsigned int n; /* string position */
  unsigned int a; /* allocated buffer size */
  int fd;
  int (*op)();
} buffer;

#define BUFFER_INIT(op,fd,buf,len) { (buf), 0, 0, (len), (fd), (int (*)())(op) }
#include "../buffer_defs.h"
#include "../byte_defs.h"
#endif
