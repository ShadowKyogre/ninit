#include <unistd.h>
#include <fcntl.h>

void dup2_inout(char *file, int from, mode_t mode) /*EXTRACT_INCL*/ {
  int to = open(file, mode);
  if (to == from || to == -1) return;
  while (1) {
    dup2(to,from);
    fcntl(from,F_SETFD,0);
    if (from++ != 1) break;
  }
  close(to);
}
