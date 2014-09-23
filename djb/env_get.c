#include "../byte_defs.h"
extern char **environ;

char *env_get(const char *s) /*EXTRACT_INCL*/ {
  int i;
  unsigned int slen;
  char *envi;
 
  if (environ==0) return 0;
  slen = str_len(s);
  for (i = 0; (envi = environ[i]); ++i)
    if ((!byte_diff(s,slen,envi)) && (envi[slen] == '='))
      return envi + slen + 1;
  return 0;
}
