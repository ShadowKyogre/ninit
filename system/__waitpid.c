#include <asm/unistd.h>

#ifndef __NR_waitpid

#include <sys/types.h>
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *status, int options) {
  return wait4(pid, status, options, 0);
}
#endif
