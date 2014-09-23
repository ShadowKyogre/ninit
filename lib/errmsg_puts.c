#include "../ninitfeatures.h"
void errmsg_puts(int fd, const char *buf) /*EXTRACT_INCL*/ {
  if (buf==0) { errmsg_put(fd, buf, 0); return; }
  if (buf[0]) errmsg_put(fd, buf, str_len(buf));
}
