#include <sys/file.h>
#define NANO_wait \
	if (nsec < 500000000) nsec *= 2;\
	else carp("could not acquire lock on: ",home,"/in");\
	nano_sleep(0,nsec)

static void open_inout(char *home) {
  char *x,*y;
  uint32t nsec = 3906249;
  int len=str_len(home);

  x=alloca(len+6);	  if (x==0) _exit(1);
  y=x+len;
  while (len--) x[len] = home[len];

  y[0]='/'; y[1]='i'; y[2]='n'; y[3]=0;
  infd = open(x, O_WRONLY);

  y[1]='o'; y[2]='u'; y[3]='t'; y[4]=0;
  outfd = open(x, O_RDONLY);

  if (infd<0 || outfd<0) die(1, "could not open ",home,"/[in|out]");

#ifdef HAVE__NR_flock
  while (flock(infd, LOCK_EX | LOCK_NB)) { NANO_wait; }
#else

#ifndef INIT_SYSTEM
  while (lockf(infd, F_LOCKW, 1)) { NANO_wait; }
#else
  { 
    struct flock fl;
    byte_zero(&fl, sizeof(fl));
    fl.l_whence=SEEK_CUR;
    fl.l_len = 1;
    fl.l_type = F_WRLCK;
    while (fcntl(infd, F_SETLKW, &fl)) { NANO_wait; }
  }
#endif
#endif
}

#undef NANO_wait
