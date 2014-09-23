#include <sys/types.h>
#include <sys/resource.h>
#include "ninitfeatures.h"

static char *arg;

static void doit(int resource)
{
  unsigned long u;
  struct rlimit r;

  if (getrlimit(resource,&r) == -1) {
    carp("g","etrlimit failed");
    return;
  }
  
  if (arg[0]=='=' && arg[1]==0)
    r.rlim_cur = r.rlim_max;
  else {
    if (arg[scan_ulong(arg,&u)]) {
      carp("invalid line: ", arg-1);
      return;
    }
    r.rlim_cur = u;
    if (r.rlim_cur > r.rlim_max)
      r.rlim_cur = r.rlim_max;
  }

  if (setrlimit(resource,&r) == -1)
    carp("s","etrlimit failed");
}

void softlimit(char **argv) /*EXTRACT_INCL*/ {
  while (*argv) {
    arg = argv[0] + 1;
    switch(argv[0][0]) {
      case 0: break;
      case 'a':
#ifdef RLIMIT_AS
        doit(RLIMIT_AS);
#endif
#ifdef RLIMIT_VMEM
        doit(RLIMIT_VMEM);
#endif
        break;
      case 'c':
#ifdef RLIMIT_CORE
        doit(RLIMIT_CORE);
#endif
        break;
      case 'd':
#ifdef RLIMIT_DATA
        doit(RLIMIT_DATA);
#endif
        break;
      case 'f':
#ifdef RLIMIT_FSIZE
        doit(RLIMIT_FSIZE);
#endif
        break;
      case 'l':
#ifdef RLIMIT_MEMLOCK
        doit(RLIMIT_MEMLOCK);
#endif
        break;
      case 'm':
#ifdef RLIMIT_DATA
        doit(RLIMIT_DATA);
#endif
#ifdef RLIMIT_STACK
        doit(RLIMIT_STACK);
#endif
#ifdef RLIMIT_MEMLOCK
        doit(RLIMIT_MEMLOCK);
#endif
#ifdef RLIMIT_VMEM
        doit(RLIMIT_VMEM);
#endif
#ifdef RLIMIT_AS
        doit(RLIMIT_AS);
#endif
	break;
      case 'o':
#ifdef RLIMIT_NOFILE
        doit(RLIMIT_NOFILE);
#endif
#ifdef RLIMIT_OFILE
        doit(RLIMIT_OFILE);
#endif
        break;
      case 'p':
#ifdef RLIMIT_NPROC
        doit(RLIMIT_NPROC);
#endif
        break;
      case 'r':
#ifdef RLIMIT_RSS
        doit(RLIMIT_RSS);
#endif
        break;
      case 's':
#ifdef RLIMIT_STACK
        doit(RLIMIT_STACK);
#endif
        break;
      case 't':
#ifdef RLIMIT_CPU
        doit(RLIMIT_CPU);
#endif
        break;
      default:
	carp("invalid line: ", argv[0]);
    }
    argv++;
  }
}
