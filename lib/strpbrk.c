#include "../ninitfeatures.h"

char *strpbrk(const char *s, const char *accept) /*EXTRACT_INCL*/ {
  register int i,l=str_len(accept);
  for (; *s; s++)
    for (i=0; i<l; i++)
      if (*s == accept[i])
	return (char*)s;
  return 0;
}
