#include "../ninitfeatures.h"
#if 0
struct error_table { int n; char *s; }; /*EXTRACT_UNMOD*/
#endif

char *error_string(struct error_table *table, int n) /*EXTRACT_INCL*/ {
  static char y[28];
  char *x=y;
  for (; table->s; table++) 
    if (table->n == n) return table->s;

  x += str_copy(x,"error=");
  x += fmt_ulong(x,n);
  *x = 0;
  return y;
}
