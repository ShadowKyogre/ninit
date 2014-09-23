#include "../int_defs.h"

static char fu_buffer[7*12];
/* format up to 6 numbers */

char *fu(uint32t u) /*EXTRACT_INCL*/ {
  static char k=7;
  char *p = fu_buffer + 12*k;
  *--p = 0;
  do {
    *--p = '0' + (u%10);
    u /= 10;
  } while (u);
  if (--k == 1) k=7;
  return p;
}
