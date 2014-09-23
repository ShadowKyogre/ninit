#define INIT_PROGRAM
#include "ninit.h"

/* load a service into the process data structure and return index or -1
 * if failed */
static int loadservice(char *s) {
  struct process tmp;
  int fd;
  if (*s==0 || *s=='#') return -1;
  fd=findservice(s);
  if (fd>=0) return fd;
  if (chdir(initroot) || chdir(s)) return -1;
#ifdef INIT_CREATE_INOUT_PIPE
  if (SYS_mknod("log/in",S_IFIFO|0600,0)==0) symlink("log/in","out");
#endif
  byte_zero(&tmp, sizeof(tmp));
  tmp.startedat = time(0);
  tmp.pr_respawn = (access("respawn", R_OK)==0);
  return addprocess(&tmp, s);
}

static void childhandler();
static int do_poll(struct pollfd *pfd, int timeout) {
  int k, n = timeout & 3;
  for (k=0; k<n; k++) {
    pfd[k].events = POLLIN;
    pfd[k].revents = 0;
  }
  CHILD_UNBLOCK;
  k=poll(pfd,n,timeout);
  CHILD_BLOCK;
  if (k<0) {
    if (errno==EINTR) childhandler();
#ifdef INIT_POLL_FAILED
    else { write(1,"poll failed!\n",13); sulogin(); }
#endif
  }
  return k;
}

static int startbyind(int service, int father) {
  struct process *pr = root + service;
  char *name = process_name(pr);
  pid_t pid;
  struct timespec req = {0,500000000};

  if (pr->pid || pr->pr_circular) return service;
  pr->pr_circular=1;
  pr->father=father;
  pr->cron=0;
#if INIT_HISTORY > 0
  for (pid = INIT_HISTORY; pid>1; --pid) history[pid] = history[pid-1];
  history[1]=service+1;
#endif
  if (chdir(initroot)) return -1;
  while ((pid=fork()) < 0) nanosleep(&req,0);
  if (pid==0) {
    CHILD_UNBLOCK;
    if (chdir(initsysdir)==0) {
      char *aa[5];
      INIT_ARGS5(aa, pr->pr_end ? "end" : "./run",name,initroot,initsysdir,0);
      if (time(0) == pr->startedat) nanosleep(&req,0);
      free_execve("./run",aa,environ);
    }
    _exit(1);
  }
  pr->pid=pid;
  pr->startedat=time(0);
  if (chdir(name)==0) {
    int fd;
    if (pr->pr_end) pr->pr_end = 0;
    else pr->pr_end = (access("end",X_OK)==0);

    if ((fd=open("sync", O_RDONLY)) >= 0) {
      struct pollfd pfd[2];
      unsigned int u;
      byte_zero(pfd, sizeof(pfd));
      read(fd, pfd, sizeof(pfd)-1);
      close(fd);

      if ((u=atoulong((void *)pfd)) == 0) u = -1;
      pfd->fd = -1;
      do {
	do_poll(pfd, 1001); /* must be 4k+1 */
	if (pr->pid < 2) break;
      } while (u--);
    }
  }
  return service;
}

static int startbyname(char *name,int father) {
  char *s;
  int fd,len,service=loadservice(name);
  if (service<0) return -1;
  name = root_name(service);
#ifdef INIT_BLACK
  if (father>=0)
    for (len=1; (s=Argv[len]); len++)
      if (*s=='#' && !str_diff(s+1,name))
	return service;
#endif
#ifdef INIT_LOG_SERVICE
  len = str_len(name);
  s = alloca(len+5);	if (s==0) return -1;
  byte_copy(s,len,name);
  byte_copy(s+len,5,"/log");
  startbyname(s,service);
#endif

  if (chdir(initroot) || chdir(name)) return -1;
  if ((fd = open("depends", O_RDONLY)) >= 0) {
    char *p, exitasapl=0;
    if (GLOBAL_READ(fd,s, len,32000)) { close(fd); return 0; }
    close(fd);
    s[len]=0;
    for (p=s; ;p=++s) {
      while (*s && *s!='\n') s++;
      if (*s==0) ++exitasapl;
      *s=0;
      startbyname(p,service);
      if (exitasapl) break;
    }
  }
  return startbyind(service,father);
}

/* return -1 on error */
static int restartservice(char *s) {
  int n=loadservice(s);
  if (n>=0 && root[n].pid<2) {
    if (start_time) root[n].cron = start_time;
    else {
      root[n].pid=0;
      circsweep();
      n=startbyname(s,-1);
    }
  }
  start_time = 0;
  return n;
}

static void handlekilled(int ind) {
  struct process *pr = root + ind;
  next_cron = 0;
  pr->pr_finish = 1;
  pr->pid = 0;
  if (pr->pr_end || pr->pr_respawn) {
    circsweep();
    startbyind(ind,pr->father);
  } else {
    pr->startedat=time(0);
    pr->pid += 1;
  }
}

static void childhandler() {
  int ind;
  pid_t pid;
  if (do_update) return;
  while ((pid=waitpid(-1,0,WNOHANG))) {
    if (pid < 0) {
#ifdef INIT_EXIT_WARNING
      static char saidso;
      if (!saidso) { write(2,"all services exited.\n",21); ++saidso; }
#endif
      return;
    }
    for (ind=0; ind<=maxprocess; ind++)
      if (root[ind].pid==pid) {
	handlekilled(ind);
	break;
      }
  }
}

static void do_initctl(struct pollfd *pfd) {
  int fd;
  if (restartservice("sysvinit") >= 0) {
    if ((fd=open(INIT_FIFO,O_RDWR)) >= 0) 
      initctl=fd;
  } else ++initctl;
  pfd[1].fd = initctl;
  fcntl(pfd[1].fd,F_SETFD,FD_CLOEXEC);
}

static void read_infd() {
  char *b1, *other, *buf;
  struct process *pr;
  int idx, len, offset;
  uint32t u32;

  buf = read_alloca(384);
  if (buf==0 || (len=read(infd,buf,380)) <2) return;

  b1 = buf + 1;
  buf[len] = 0; buf[len+1] = 0;
  offset = str_len(buf) + 1; 
  other  = buf + offset;
  u32 = atoulong(other);
  next_cron = 0;

  switch (buf[0]) {
  case 'D':
    if (*b1=='M') t_write(mem.x,mem.a);
    else {
      if (*b1=='1') do_update = 1;
      t_write(mem.x+mem.r, mem.a-mem.r);	/* names + history */
      t_write(root, mem.l);			/* precess */
    }
    break;
  case 'U':   /* U\0argv[0]  or  Uservice\0 */
    if (*b1==0) {
      char *aa[3]; INIT_ARGS3(aa, other, "", 0);
      execve(other,aa,environ);
    } else
      if (len >= offset + PROCESS_SIZE &&
	  addprocess(other,b1) >= 0) goto ok;
    goto error;
  case 's':
    start_time = u32;
    if (restartservice(b1) < 0) goto error;
ok:	 t_write("1",1); break;
error:	 t_write("0",1); break;
  
  default:
    if ((idx=findservice(b1)) <0) goto error;
    pr = root + idx;

    switch(buf[0]) {
    case 'r': pr->pr_respawn = (other[0] != 0); goto write_pr;
    case 'P':
      if ((pid_t)u32<2 || kill(u32,0)) goto error;
      pr->pid = u32; goto write_pr;
    case 'c': pr->cron = u32;
    case 'p':
write_pr:
      t_write(pr, sizeof(struct process));
    }
  }
}

int main(int argc, char *argv[]) {
  struct pollfd pfd[12];
  time_t now;
  int i;
#ifdef INIT_BLACK
  Argv=argv;
#endif
  mem.r = INIT_ALLOC_BUFFER;
  for (i=1; i<argc; i++)
    if (argv[i][0]=='-') {
      char *p = argv[i]+2;
      switch (p[-1]) {
      case 'M': mem.r = atoulong(p); break;
#ifdef INIT_HOME
      case 'H': initroot = p; break;
      case 'S': initsysdir = p; break;
#endif
      default: p[-2] = '#';	/* -sshd --> #sshd */
      }
    }

  mem.r &= 0xfff8;	/* max 64K */
  mem.a = mem.r + (INIT_HISTORY+1) * sizeof(unsigned short);
  mem.x = alloca(mem.a);	/* if it fails bummer */
  history = mem.x + mem.r;
  byte_zero(history, (INIT_HISTORY+1) * sizeof(unsigned short));
  ((unsigned char*)history)[1] = INIT_HISTORY;

  if (getpid()==1) {
    set_reboot(0);
    i = open("/dev/console",O_RDWR|O_NOCTTY);
    if (i>0) { dup2(i,0); close(i); }
    ioctl(0, KDSIGACCEPT, SIGWINCH);
  }

  set_sa(SIGCHLD);
  set_sa(SIGINT);    /* ctrl-alt-del */
  set_sa(SIGWINCH);  /* keyboard request */
#ifdef INIT_SYSVINIT_SIGNALS
  set_sa(SIGPWR);    /* ifdef INIT_POWERSTATUS start powerZ */
  set_sa(SIGHUP);    /* ifdef INIT_SIGHUP start levelU */
  set_sa(SIGUSR1);   /* ifdef INIT_SIGUSR1 reopen /dev/initctl */
#endif
  CHILD_BLOCK;

  if (chdir(initroot) ||
      (infd=open("in",O_RDWR)) < 0 ||
      (outfd=open("out",O_RDWR|O_NONBLOCK)) < 0) {
#ifdef INIT_PIPES_ERROR
    write(1,"pipes error\n",12); sulogin();
#endif
  } else {
    int fd=open(".sync", O_RDONLY);
    while ((i=read(fd,pfd,sizeof(pfd))) > 0) write(1,pfd,i);
    close(fd);
  }

  fcntl(infd,F_SETFD,FD_CLOEXEC);
  fcntl(outfd,F_SETFD,FD_CLOEXEC);

  if (argv[argc-1][0] == 0) {
    ++do_update;
    t_write("8",1);
  } else {
    for (i=1; i<argc; i++) restartservice(argv[i]);
    if (maxprocess < 0) restartservice("default");
  }

  pfd[0].fd = infd;
  pfd[1].fd = -1;

  for (;;) {
    for (i=0; i<4; i++)
      if (got_sig[i]) {
	char *xx = NINIT_SERVICES_CODED;
	got_sig[i]=0; restartservice(xx + xx[i]);
      }
    childhandler();

    if ((time(&now)) > next_cron) {
      time_t t;  next_cron = now + 1800;
      for (i=0; i<=maxprocess; i++) {
	struct process *pr = root + i;
	if (pr->pid > 1 && kill(pr->pid, 0)) handlekilled(i);

	if (pr->pid < 2 && (t=pr->cron) > 0) {
	  if (t < now) restartservice(root_name(i));
	  if (t < next_cron) next_cron = t;
	}
      }
    }

    if ((i=do_poll(pfd, 5002)) == 0) {	/* timeout; must be 4k+2 */
      INIT_FREE_MEMBLOCKS(do_update=0);
      if (initctl < -1) do_initctl(pfd);
      continue;
    }
    if (i>0) {
      if (pfd[0].revents) read_infd();
      if (pfd[1].revents) restartservice("sysvinit");
    }
  }
}
