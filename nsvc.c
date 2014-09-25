#define SVC_PROGRAM
#include <fcntl.h>
#include <time.h>
#include <sys/file.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "ninit.h"
#include "nsvc_buffer.h"

extern int ioctl();

static char **Names_skip, **NamesX, *buf, *initroot, *initsysdir, *nsvc_other, respmode;
static int NamesI, do_update, tty = -111;

static void e(int n) { buffer_flush(buffer_1); _exit(n); }
static void die_xx(char *s) { errmsg_iam("nsvc"); err(2,s,0); _exit(1); }

#include "get_services.h"
#include "findservice.h"
#include "open_inout.h"
#include "tryservice_nsvc.h"

static char *get_name() {
  char *x, *s, **skp;
 again:
  if (NamesI >= 0) {
    x = (NamesI <= maxprocess) ? root_name(NamesI) : 0;
    ++NamesI;
  } else {
    x = *NamesX;
    ++NamesX;
  }
  if (x)
    for (skp=Names_skip; (s=*skp); skp++)
      if (!str_diff(s,x)) goto again;
  return x;
}

static void close_inout() {
  close(infd); infd = -1;
  close(outfd); outfd = -1;
}

static int __findservice(char *service) {
  int idx;
  get_services();
  idx=findservice(service);
#ifdef CLEAN_SERVICE
  if (idx<0) idx=findservice(cleanservice(service));
#endif
  return idx;
}

void print_pid(char *service, pid_t pid) {
  errmsg_iam("nsvc");
  carp(service,(pid < 0) ? ": no such service" : ": service not running");
}

/* return PID, -1 if service is not loaded */
static pid_t __readpid(char *service, int verbose) {
  pid_t idx=__findservice(service);
  if (idx >= 0) idx = root[idx].pid;
  if (verbose && idx<2) 
    print_pid(service, idx);
  return idx;
}

static char *fu_x(uint32t u, int len) {  /* len must be max 10 !!! */
  char *x = fu(u);
  int y = str_len(x);
  while (y < len) { *--x = ' '; y++; }
  return x;
}

static void fs_x(const char *s, int len) {
  int k = str_len(s);
  outs(s);
  if (k >= len) outc('\t');
  else while (k++ < len) outc(' ');
}

static void fmt_time(unsigned long up) {
  unsigned long u;
  u = up % 60; up /= 60;
                                   fmt_color(fu_x(u, 4),'6'," ");
  if (up) { u = up % 60; up /= 60; fmt_color(fu_x(u, 2),'4'," "); }
  if (up) { u = up % 24; up /= 24; fmt_color(fu_x(u, 2),'5'," "); }
  if (up)                          fmt_color(fu_x(up,4),'2',"");
}

static int fmt_service(int i, int flagtty) {
  struct process *pr = root + i;
  pid_t pid = pr->pid;
  time_t cr, uptime;
  static time_t now;

  if (tty==-111) {
#if defined(INIT_SYSTEM)
    pid_t dummy;
    tty = (ioctl(1, X_TIOCGPGRP, &dummy) == 0);
#else
    tty=isatty(1);
#endif
  }

  if (!now) now=time(0);
  uptime = now - pr->startedat;
  if (uptime<0) uptime=0;

  if (flagtty || tty) {
    char *name=root_name(i);
    if (flagtty<=4 && tty) {
      char flags[8], *x, ch;
      int len;

      if (errmsg_argv0) { outs(errmsg_argv0); outs(": "); }
      fs_x(name, 16);

      switch (pid) {
        case 0: x="  down"; ch='7'; break;
        case 1: x="finish"; ch='2'; break;
        default:  
	  if (pr->pr_respawn) ch='1'; else ch='4';
	  x=fu_x(pid,6);
      }
      fmt_color(x,ch," ");

      len=0;
      if (pr->pr_respawn)	flags[len++] = 'R';
      if (pr->cron)	flags[len++] = 'C';
      if (pr->pr_end)	flags[len++] = 'E';
      flags[len] = 0 ;
      fs_x(flags, 3);

      fmt_time(uptime);

      if ((cr = pr->cron)) {
	fmt_color("\tnext cron",'1',":");
	if (cr > now) fmt_time(cr-now);
	else outs("  waititing");
	outc('\t');
	fmt_color(fu(cr), '8', "");
      }
      outc('\n');
    }
    else
      msg(name);
  } else {
    errmsg_iam(0);
    msg(fu(pid)," ",fu(uptime));
  }

  if (pid==0) return 2; else if (pid==1) return 3; else return 0;
}

static void dumphistory() {
  int j;
  if (!maxhistory) {
    carp("ninit compiled without history support");
    return;
  }
  errmsg_iam(0);
  for (j=0; j<maxhistory; j++)
    if (--history[j] <= maxprocess) {
      fmt_service(history[j], 4+root[history[j]].pr_circular);
      root[history[j]].pr_circular = 1;
    }
}

static void dumpdependencies(char* service, unsigned int skip) {
  int i,idx;
  idx=__findservice(service);
  if (idx<0) { print_pid(service, -1); return; }

  errmsg_iam(0);
  for (i=0; i<=maxprocess; ++i) {
    if (root[i].father==idx && i!=idx) {
      char *name = root_name(i);
      int k=skip;
      while (k-->0) outs("  ");
      msg(name);
      if (skip<16) dumpdependencies(name,skip+1);
      else msg(buf,"  ...",name);
    }
  }
}

static void wait_childs(int len) {
  unsigned wtime;
  pid_t i, pids[len];
  char *x;

  for (len=0; (x=get_name());)
    if ((pids[len]=__readpid(x, 0)) > 1) ++len;

  close_inout();
  wtime = 4*atoulong(nsvc_other);

  while (1) {
    for (i=len; i>0;)
      if (kill(pids[--i],0) && errno != EPERM)
	pids[i] = pids[--len];
    if (len==0) _exit(0);
    if (wtime--==0) _exit(1);
    nano_sleep(0,250000000);
  }
}

static int try_srv(char *s, char mode) {
  int r;
  char *save = nsvc_other;
  if (mode == 'r') 
    nsvc_other = (respmode) ? "c" : 0;	/* must be odd char !!! */ 
  r = tryservice(s, mode);
  nsvc_other = save;
  return r;
}

static void get_timestamp() {
  char *x = nsvc_other;
  unsigned int now;
  if (x[-1] == 'C') {
    if  (x[0] == '+') ++x;
    else return;
  }
  if ((now = atoulong(x)))
    nsvc_other = fu(time(0) + now);
}

int main(int argc,char *argv[]) {
  int i,idx,ret=0;
  pid_t pid;
  char *x, **skp;

  __b_1.x = alloca(BUFFER_1_LEN);
  buf = alloca(BUFFER_TMP_LEN);
  mem.a = INIT_ALLOC_BUFFER;

  x = env_get("NINIT_MEMORY");
  if (x) mem.a = atoulong(x);
  mem.r=mem.a;
  if (mem.a==0 || mem.a > 64000 || (mem.x=alloca(mem.a))==0) _exit(1);

  errmsg_iam("nsvc");
  infd = -1; outfd = -1;
  INIT_ENV_GET_HOME(initroot,"NINIT_HOME","INIT_HOME");
  INIT_ENV_GET_HOME(initsysdir,"NINIT_SYSDIR","INIT_SYSDIR");

  Names_skip = alloca(argc * sizeof(char *));
  for (skp=Names_skip; (x=argv[1]); argv++) {
    if (x[0] != '-' || x[1] != 'S') break;
    *skp++ = x+2;
  }
  *skp = 0;

  {
    int uid=getuid(), euid=geteuid();
    if (initroot && (uid != euid)) { ops: die_xx("\ago away!"); }
    if (initroot==0) initroot=INITROOT;
	if (initsysdir==0) initsysdir=INITSYSDIR;

    if (argv[1] == 0) { setuid(uid); goto REMOVE; }
    open_inout(initroot);
    
    if (uid != euid) {
      if (setuid(uid)) goto ops;
      get_services();
      close_inout();
    }
  }

  nsvc_other = argv[1] + 2;

  if (argv[2]==0) {
    char *a1 = argv[1];
    get_services();
    close_inout();

    if (a1[0] == '-') {
      switch (a1[1]) {
      case 'V': goto REMOVE;
      case 'L':
	errmsg_iam(0);
	while (get_name()) fmt_service(NamesI-1, 1);
	e(0);
      case 'H':
	dumphistory();
	e(0);
      case 'D':
	msg("memory usage: ",fu(mem.l + root_names_len)," = ",
	    fu(mem.l),"+",fu(root_names_len)," = services+names [",
	    fu(maxprocess+1),"]");
	e(0);
      }
    }

    i=__findservice(a1);
    if (i==-1) { print_pid(a1, -1); e(1); }
    ret=fmt_service(i,0);
    
    if (i >= 0 && tty && root[i].pid > 1) {
      INIT_ARGS2(argv, "./procfs", fu(root[i].pid));
      goto PROCFS;
    }
  } else {
    char *x;
    unsigned int sig=0;

    x = argv[2];

    if (argv[1][0]=='-') {
      char flagdown=1, try_flag='r';
      respmode = 0;

      switch (argv[1][1]) {
      case 'P': 
	ret=try_srv(x, 'P');
	if (ret) carp("Could not set PID of service ",x);
	break;

      case 'D':	dumpdependencies(x,0); break;
      case 'E':
      case 'Z':
      REMOVE:
	argv[0] = "./remove";	/* ~~XXX: make sys as environ!~~ */
      PROCFS:
	close_inout();
	buffer_flush(buffer_1);
	ret = chdir(initsysdir);
	if (!ret) ret=execve(argv[0], argv, environ);
	break;
      default:

	if (argv[3] == 0 && !str_diff(argv[2], "ALL")) {
	  get_services();
	  argc = maxprocess + 4;
	} else {
	  NamesX = argv + 2;
	  NamesI--;
	}

	switch (argv[1][1]) {
	case 'W':
	  wait_childs(argc);
	case 'g':
	  while ((x=get_name())) {
	    pid=__readpid(x,1);    
	    if (pid > 1) { errmsg_iam(0); msg(fu(pid)); }
	    else ret = 1;
	  }
	  break;
	case 'u': respmode=1;
	case 'o':
	  try_flag='s';
	  goto try_it;

	case 'C': try_flag='c'; /* cron */
	case 'R': respmode=1;

	try_it:
	case 'r': flagdown=0;
	case 'd':
	  get_timestamp();
	  while ((x=get_name())) {
	    idx=try_srv(x, try_flag);
	    if (idx==0) {
	      if (try_flag=='s') idx=try_srv(x,'r');
	      else if (flagdown) {
		pid=__readpid(x,0);
		if (pid>1 && 0==kill(pid,SIGTERM)) kill(pid,SIGCONT);
	      }
	    }
	    if (idx) { print_pid(x,-1); ret=1; }
	  }
	  break;
	  
	default: 	  
#ifdef NSVC_SIGNAL_CODED
	  {
	    unsigned char *S, *Sig = (unsigned char *)NSVC_SIGNAL_CODED;
	    for (S=Sig; *S; S += 2)
	      if (argv[1][1] == *S) { sig = S[1]; break; }
	  }
#else
#define kk(C,S) case C: sig=S; break
	  kk('t',SIGTERM); kk('a',SIGALRM); kk('p',SIGSTOP); kk('c',SIGCONT);
	  kk('h',SIGHUP);  kk('i',SIGINT);  kk('k',SIGKILL);
#endif
	} /* inner switch */
      }   /* outer switch */ 
    }

    if (sig) {
      while ((x=get_name())) {
	pid=__readpid(x,1);
	if (pid < 2) 
	  ret=1;
	else if (kill(pid,sig)) {
	  char* s;
	  switch (errno) {
	  case EINVAL: s="invalid signal"; break;
	  case EPERM: s="permission denied"; break;
	  case ESRCH: s="no such pid"; break;
	  default: s="unknown error";
	  }
	  carp(x,": could not send signal ",fu(sig)," to PID ",fu(pid),": ",s);
	  ret=1;
	}
      }
    }
  }
  e(ret);
  return 0;
}
