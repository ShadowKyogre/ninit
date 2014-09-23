#include "../ninitfeatures.h"

unsigned int scan_sec(const char *src, unsigned long *ul) /*EXTRACT_INCL*/ {
  unsigned long tmp, u=0;
  char ch, *s = (char *)src;
  for (; *s; ) {
    s += scan_ulong(s, &tmp);
    ch = *s;
    if (ch > 96) ch -= 32; /* upper case */

    switch (ch) {
      case 'W': tmp *= 10080; s++; break;
      case 'D': tmp *= 1440; s++; break;
      case 'H': tmp *= 60; s++; break;
      case 0: break; 
      default: u += tmp; goto ready;
    }
    u += tmp;
  }
 ready:
  *ul = u * 60;
  return s-src;
}
