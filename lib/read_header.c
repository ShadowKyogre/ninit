#include <unistd.h>
#include <fcntl.h>

#define MEM_BUF 160

char *read_header(const char *name) /*EXTRACT_INCL*/ {
  static char buf[MEM_BUF+1];
  int fd, k;

  fd =open(name, O_RDONLY);
  if (fd < 0) return 0;
  k =read(fd, buf, MEM_BUF);
  close (fd);
  if (k < 0) return 0;

  buf[k] = 0;
  return buf;
}
