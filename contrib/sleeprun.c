#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include "../ninitfeatures.h"
#include "../error_table.h"

int main(int argc, char **argv) {
  int fd;
  unsigned long now, interval, last, ul[2] = { 0, 0 };
  char tmp[32],*p;
  if (argc<3) 
    die(1, "usage: sleeprun SleepFile interval [program args...]\n"
	    "usage: sleeprun -aNumber program [args...]");
  errmsg_iam(argv[0]);

  p = argv[1];
  if (p[0] == '-' && p[1] == 'a') {
    scan_ulong(p+2, &interval);
    if (interval) alarm(interval);
    argv += 2;
    goto do_it;
  }

  if (read_ulongs(argv[1], ul, 2) > 0) {
    errno=0;
    if (kill(ul[0],0)==0 || errno != ESRCH) {
      tmp[fmt_ulong(tmp, ul[0])] = 0;
      carp("WARNING: a program with PID ",tmp," is running now");
    }
  }

  last=ul[1];
  scan_ulong(argv[2], &interval);
  now = (unsigned long)time(0);

  last += interval;
  if (last > now) {
    nano_sleep(last-now, 0);
    now = (unsigned long)time(0);
  }

  if ((fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644)) >=0) {
    p = tmp;
    p += fmt_ulong(tmp, getpid()); *p++ = ':';
    p += fmt_ulong(p, now); *p++ = 0;
    write(fd,tmp,p-tmp);
    close(fd);
  }

  argv+=3;
 do_it:
  if (argv[0]) {
    char *argv_0 = argv[0];
    argv[0] = argv_0 + str_rchr(argv_0,'/');
    if (argv[0][0]) argv[0]++;
    else argv[0]=argv_0;

    pathexec_run(argv_0,argv,environ);
    carp("unable to run: ",argv_0,": ",error_string(table, errno));
    return 127;
  }
  return 0;
}
