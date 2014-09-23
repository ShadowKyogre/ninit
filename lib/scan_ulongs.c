#include "../ninitfeatures.h"

unsigned int scan_ulongs(char *src, unsigned long *u, int len, unsigned int (*op)(), char sep, int *read_len) /*EXTRACT_INCL*/ {
  int j, k;
  char *p=src;

  for (k=0; k<len;) {
    j = op(p, u+k); if (j==0) break;
    ++k;
    p += j;
    if (*p != sep) break;
    ++p;
  }

  *read_len = (p-src);
  return k;
}
