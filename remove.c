#include <unistd.h>
#include <alloca.h>
#include <fcntl.h>
#include "ninitfeatures.h"
/*
  convert:  xxx option A B C D 
  to: /sbin/ninit-reload arg(s)
  and start above program
  options: -V -E -Z[number]
 */
int main(int argc, char **argv) {
  char *x, **qq, **q;
  char *help_file = ".sync";
  char flag[3]={ '-', 'e', 0 };


  x = argv[1];
  if (x == 0) { help_file = ".nsvc_help"; goto write_it; }
  if (x[0] != '-') _exit(1);

  qq = alloca(sizeof(char *) * (2*argc + 30));
  q = qq;

  INIT_ARGS2(q, "/sbin/ninit-reload","-u");
  q += 2;

  switch (x[1]) {
  case 'Z': flag[1] = 'R';
  case 'E':
    if (x[2]) {
      x[1]='a';
      INIT_ARGS2(q,x,"-v");
      q += 2;
    }
    break;
  case 'V': {
    char buf[1024];
    int r, fd;

    write_it:
    fd = open(help_file, O_RDONLY);
    while ((r = read(fd, buf, sizeof(buf))) > 0)	/* close(fd); */
      write(1, buf, r);
  }
  default:
    _exit(0);
  }

  for (argv += 2; (x=*argv); argv++) {
    if (*x == 0) continue;	/* skip empty args */
    INIT_ARGS2(q,flag,x);
    q += 2;
  }

  *q++ = "/sbin/ninit";
  INIT_ENV_GET_HOME(x,"NINIT_HOME","INIT_HOME");

  if (x) {
    x -= 2;
    x[0] = '-';
    x[1]='H';
    *q++ = x;
  }
  *q = 0;

  carp("NINIT-remove: starting:");
  fmt_argv(1, qq, " ");
  errmsg_puts(1,"\n");
  errmsg_puts(1,0);

  /*  return 0; */
  execve(qq[0], qq, environ);
  _exit(1);
}
