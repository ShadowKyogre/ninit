#include <unistd.h>
#include <errno.h>
#include <alloca.h>
#include <fcntl.h>
#include "ninitfeatures.h"
#include "process_defs.h"

static int infd, outfd;

#include "open_inout.h"
#include "tryservice.h"

int main(int argc, char* argv[]) {
  char *x, *initroot, **arg;
  struct process pr[6];
  unsigned long pidfile;
  int count=0, pid, ret=1;

  if (argc<4) die(1,
	"usage: ninit-pidfile",
	" service /var/run/daemon.pid [-H home] /sbin/daemon ...");
  errmsg_iam("ninit-pidfile");

  if (unlink(argv[2]) && errno != ENOENT)
    die(1, "could not remove pid file: ", argv[2]);

  arg = argv + 3;
  x = arg[0];

  if (argc > 5 && x[0] =='-' && x[1] =='H' && x[2] ==0) {
    initroot = arg[1];
    arg += 2;
  } else
    initroot = INITROOT;

  while ((pid = fork()) < 0) nano_sleep(0, 500000000); /* 0.5 sec */

  if (pid == 0) {
    x = arg[0];

    arg[0] = x + str_rchr(x,'/');
    if (arg[0]) arg[0]++;
    else        arg[0]=x;

    execve(x,arg,environ);
    die(3, "execve failed on: ", x);
  }

  while (1) {
    if (read_ulongs(argv[2], &pidfile, 1)) {
      if (str_len(argv[1]) > 320) ret=1;
      else {
	ret = tryservice(initroot,argv[1],"P",fu(pidfile), pr);
	ret = (ret != sizeof(pr[0]) || pr->pid != (pid_t)pidfile);
      }
      break;
    }
    if (++count>=30) break;
    else
      nano_sleep(1, 0);
  }

  if (ret) carp("could not set PID of service ", argv[1]);
  return ret;
}
