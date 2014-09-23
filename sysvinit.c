#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <alloca.h>
#include "ninitfeatures.h"
#include "initreq.h"
#define IREQ sizeof(struct init_request)

static char read_buf[IREQ + 4];
struct init_request *req = (void *)read_buf;
static int infd,outfd,flag_fork;

#include "open_inout.h"
#include "uid.h"

void sighandler(int sig) { (void)sig; while (waitpid(-1,0,WNOHANG) > 0); }

static void do_it(char *A) {
  int r=0;
  if (flag_fork) { while ((r=fork()) == -1) nano_sleep(1,0); }
  if (r==0) {
    open_inout(INITROOT);
    msg("Starting service: ",A+1);
    write(infd, A, str_len(A));
    read(outfd, req, IREQ);
    
    close(infd); close(outfd);
    if (flag_fork) _exit(0);
  }
}

void print_env() {
  char *x = req->i.data;
  msg("request"," for INIT_CMD_SETENV");
  while (*x) {
    msg(x);
    x += str_len(x);
    x++;
  }
}

int main(int argc, char **argv) {
  struct pollfd pfd;
  unsigned long ul[2] = {0,0};
  int fd, timeout = -1;
  char power[] = "spowerS";

  errmsg_iam("ninit-sysvinit");
  
  if (argc>1 && !str_diff(argv[1], "powerS")) {
    char ch;
    msg("trying to read and remove: ", INIT_POWERSTATUS);

    fd=open(INIT_POWERSTATUS, O_RDONLY);
    if (fd>=0) { read(fd,&ch,1); close(fd); }
    unlink(INIT_POWERSTATUS);
    
    if ('a' <= ch && ch <= 'z') ch -= 32;
    switch (ch) {
      case 'L': case 'O': break;
      default: ch = 'F';
    }
    
    power[6] = ch;
    do_it(power);
    return 0;
  }

  fd = open(INIT_FIFO, O_RDWR | O_NONBLOCK);
  if (fd < 0) {
    SYS_mknod(INIT_FIFO,S_IFIFO|0600,0);
    return -1;
  }
  fcntl(fd,F_SETFD,FD_CLOEXEC);

  pfd.fd = fd;
  pfd.events = POLLIN;
  read_ulongs("sysvinit-timeout", ul, 2);

  if (ul[0] > 0)timeout = 1000 * ul[0];
  flag_fork = (ul[1] != 0);
  // msg(" timeout: ",fu(ul[0]), ",fork-mode: ",flag_fork ? "YES" : "NO");

  if (flag_fork) set_sa(SIGCHLD);

  while (1) {
    int flag_known = 0;
    switch (poll(&pfd,1,timeout)) {
    case -1:
      if (errno == EINTR) sighandler(SIGCHLD); break;
      return -1;	/* poll failed */
    case 1: {
      int r = read(fd, req, IREQ);
      if (r == (int)IREQ) {

#ifdef SYSVINIT_DUMP_INITREQ
	if ((r=open("/tmp/__##initreq", O_WRONLY | O_APPEND)) >= 0) 
	  {  write(r,req,IREQ);  close(r); }
#endif

	if (req->magic == INIT_MAGIC) {
	  char level[] = "slevelS";
	  char *A="";

	  switch (req->cmd) {
	  case INIT_CMD_SETENV:
	    print_env();
	    flag_known=1;
	    break;
	  case INIT_CMD_RUNLVL:
	    r = req->runlevel;
	    if ('a' <= r && r <= 'z') r -= 32;
	    level[6] = r;
	    A = level;
	    break;
	  case INIT_CMD_POWEROK:
	    A = power;
	    power[6] = 'O';
	    break;
	  case INIT_CMD_POWERFAIL:
	    A = power;
	    power[6] = 'F';
	    break;
	  case INIT_CMD_POWERFAILNOW:
	    A = power;
	    power[6] = 'L';
	  }

	  if (*A) { flag_known=1; do_it(A); }
	}	/* INIT_MAGIC	*/
      }		/* r == sizeof(...) */
      if (flag_known==0) carp("unknown", " request");
    }
    break;
    default:	/* timeout	*/
      return 0;
    }		/* poll		*/
  }
}
