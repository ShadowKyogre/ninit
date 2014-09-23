#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <alloca.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <grp.h>
#include "ninitfeatures.h"
#include "uid.h"

static char *service, *home, *sysdir;

static void exec_setup(char *s) {
  if (access(s, X_OK) == 0) {
    char *x = fu(getpid());
    pid_t pid=fork();
    if (pid==-1) return;
    if (pid==0) {
      char *qq[6];
      INIT_ARGS6(qq, s,service,home,sysdir,x,0);
      execve(s,qq,environ);
      _exit(1);
    }
    waitpid(pid,0,0);
  }
}

static struct error_table table[] = {
  {ENOEXEC,	"Exec format error"},
  {ENOENT,	"No such file"},
  {EACCES,	"Permission denied"},
  {ENOMEM,	"Out of memory"},
  {0,0}
};

static void die_exec() {
  char *x=0, *y = ": ";
  struct error_table *t=table;
  for (; t->s; t++) 
    if (t->n == errno) { x = t->s; break; }
  if (x==0) { x = fu(errno); y = ": errno="; }
  errmsg_iam(0);
  carp(home,"/",sysdir,"/run",": Could not start ",
       home,"/",service,"/run",y,x);
  _exit(1); 
}

#define NI_TEXT_FILES	2
#define MAX_groups 33
enum { NI_params,NI_pidfile };

int main(int Argc, char **Argv) {
  char **argv,*argv_0,**xx,*s;
  char *tmp[10];
  static char **aa[NI_TEXT_FILES];
  static unsigned long uid[MAX_groups + 2];
  unsigned long ul[1];
  int i,fd,len;
  
  errmsg_iam("run");
  if (Argc<3) 
    { carp("usage: ",errmsg_argv0," service home sys"); _exit(1); }
  
  service=Argv[1];
  home=   Argv[2];
  sysdir= Argv[3];
  
  if (chdir(home) || chdir(service)) die_exec();
 
  if (Argv[0][0] != '-') {
    if (getppid() == 1) {
      ioctl(0, TIOCNOTTY, 0);
      setsid();
      opendevconsole();
      // ioctl(0, TIOCSCTTY, 1);
      // tcsetpgrp(0, getpgrp());
    }

    if (Argv[0][0] == 'e') { /* "end" */
      execve(Argv[0],Argv,environ);
      _exit(100);
    }

    exec_setup("sys-rsetup");
    
    if (!access("wait", R_OK) ||
	!access("environ", R_OK) ||
	!access("softlimit", R_OK) ||
	!access("cron", R_OK)) {
      Argv[0] = "run-wait";
      
      if (chdir(home) || chdir(sysdir)) return 1;
      execve("./run-wait",Argv, environ);
      if (chdir(home) || chdir(service)) return 1;  /* exec fails; go on */
    }
  }
  
  exec_setup("rsetup");
  
  INIT_ARGS2(tmp, "params","pidfile");
  for (i=0; i < NI_TEXT_FILES; i++) {
    int offset=6;
    if (i) offset=0;
    
    if ((fd = open(tmp[i], O_RDONLY)) >=0) {
      if (GLOBAL_READ(fd,s, len,12000)) _exit(1);
      close(fd);
      s[len]=0;
      
      len = splitmem(0,s,'\n');
      xx=alloca((len+offset+1) * sizeof(char*));  if (xx==0) _exit(1);
      xx += offset;
      aa[i] = xx;
      
      splitmem(xx,s,'\n');
      if (i==0 && xx[--len][0]==0) xx[len]=0;
      skip_comments(xx);
    }
  }
  
  if ((xx=aa[NI_params])) argv = xx;
  else { argv = tmp+7; *argv = 0; }
  
  for (len=129; ; len *= 2) {
    argv_0=alloca(len);		if (!argv_0) _exit(1);
    if ((i=readlink("run",argv_0,len))<0) {
      if (errno!=EINVAL) die_exec();	/* not a symbolic link */
      argv_0="./run";
      break;
    }
    if (i<len) { argv_0[i]=0; break; }
  }
  
  if ((xx=aa[NI_pidfile])) {
    argv -= 5;
    INIT_ARGS5(argv, service, xx[0], "-H", home, argv_0);
    argv_0  = "/sbin/ninit-pidfile";
  }
  
  s = argv_0 + str_rchr(argv_0, '/');
  if (s[0]) ++s;
  else s = argv_0;
  *--argv = s;
  
  dup2_inout("in", 0,O_RDONLY);
  dup2_inout("out",1,O_WRONLY);
  
  if (read_ulongs("sleep",ul,1))
    nano_sleep(*ul, 200000000);
  if ((s=read_header("nice"))) {
    if (*s == '-') { i=-1; s++; } else i=1;
    if (scan_ulong(s, ul)) { i *= (int)ul[0]; nice(i); }
  }

  len = read_ulongs("uid",uid,33);
#ifdef RUN_WANT_GID_FILE
  if (len < 2 && read_ulongs("gid",uid+1,1)) len=2;
#endif

  if (len > 1) {
    gid__t g[len];
    for (i=0; i<len; i++) g[i] = uid[i];
    SYS_setgroups(len-1,g+1);
  }
  if (uid[1]) setgid(uid[1]);
  if (uid[0]) setuid(uid[0]);

  exec_setup("setup");
  if (read_ulongs("alarm",ul,1)) alarm(*ul);

  for (fd=3; fd<1024; ++fd) close(fd);
  
  if (!access("pause", R_OK)) kill(getpid(), SIGSTOP);
  execve(argv_0,argv,environ);
  die_exec();
  return 1;
}
