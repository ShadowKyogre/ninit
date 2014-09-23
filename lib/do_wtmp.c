#include <unistd.h>
#include <fcntl.h>
#include <utmp.h>

void do_wtmp(struct utmp *utmp) /*EXTRACT_INCL*/ {
  int fd;
  if ((fd=open(_PATH_WTMP, O_WRONLY | O_APPEND)) >= 0) {
    write(fd, utmp, sizeof(struct utmp));
    close(fd);
  }
}
