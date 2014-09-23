#include "../ninitfeatures.h"

int fmt_argv(int fd, char **argv, const char *sep) /*EXTRACT_INCL*/ {
  int len=0;
  for (; argv[len]; len++) {
    errmsg_puts(fd, argv[len]);
    errmsg_puts(fd, (char *)sep);
  }
  return len;
}
