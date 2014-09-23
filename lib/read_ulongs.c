#include "../ninitfeatures.h"

int read_ulongs(char *name, unsigned long *u, int len) /*EXTRACT_INCL*/ {
  char *x = read_header(name);
  int dummy;
  if (x==0) return 0;
  return scan_ulongs(x, u, len, scan_ulong, ':', &dummy);
}
