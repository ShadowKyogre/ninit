#include "../ninitfeatures.h"

char *strstr(const char *haystack, const char *needle) /*EXTRACT_INCL*/ {
  unsigned int nl=str_len(needle);
  unsigned int hl=str_len(haystack);
  int i;
  if (!nl) goto found;

  for (i=hl-nl+1; i>0; --i) {
    if (*haystack==*needle && !byte_diff(haystack,nl,needle))
found:
      return (char*)haystack;
    ++haystack;
  }
  return 0;
}
