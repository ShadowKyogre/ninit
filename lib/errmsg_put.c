#include <sys/uio.h>
#include "../ninitfeatures.h"

#ifndef ERRMSG_PUTS_LEN
#define ERRMSG_PUTS_LEN	15
#endif

void errmsg_put(int fd, const char *buf, unsigned int len) /*EXTRACT_INCL*/ {
  static struct iovec errmsg_iov[ERRMSG_PUTS_LEN];
  static int k;
  if (buf==0 || k==ERRMSG_PUTS_LEN) {
    if (fd>=0) writev(fd,errmsg_iov,k);
    k = 0;
  }
  if (buf && len) {
    errmsg_iov[k].iov_base = (char *)buf;
    errmsg_iov[k].iov_len = len;
    k++;
  }
}
