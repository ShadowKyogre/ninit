#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "../ninitfeatures.h"

int open_tmpfd(char *target, char *tmp, int mode) /*EXTRACT_INCL*/ {
  unsigned long len;
  char *x;
  int fd;
  for (len=0 ;; len++) {
    x = tmp + str_copy(tmp, target);
    if (len) x += fmt_ulong(x,len);
    x[0] = '~'; x[1] = 0;
    fd = open(tmp, O_WRONLY|O_CREAT|O_EXCL, mode);
    if (fd >=0 || errno != EEXIST) break;
  }
  return fd;
}
