#include <stdarg.h>
#include "../ninitfeatures.h"
#include "../djb/buffer.h"

#if 0
void err(int fd, const char *m, ...) /*EXTRACT_INCL*/
#endif

#ifdef ERRMSG_BUFFER
#define P(S)	buffer_puts(X,S)
#define E()	err_b(buffer *X, const char *m, ...)

#else
#define P(S)	errmsg_puts(X,S)
#define E()	err(int X, const char *m, ...)
#endif

extern const char *errmsg_argv0;

void E() {
  const char *s=m;
  va_list a;
  va_start(a,m);

  if (errmsg_argv0) {
    P(errmsg_argv0);
    P(": ");
  }

  while (s) {
    P(s);    
    s=va_arg(a,const char*);
  }
  P("\n");
#ifndef ERRMSG_BUFFER
  P(0);
#endif
  va_end(a);
}
