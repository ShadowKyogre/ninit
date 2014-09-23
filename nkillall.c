#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <alloca.h>
#include <utmp.h>
#include <errno.h>
#include "ninitfeatures.h"
#include "process_defs.h"
#include "utmp_defs.h"
#include "uid.h"
#include "check_opt.h"

static int cfg_verbose;

void sighandler(int sig) { (void)sig; }

static void deep_sleep(char *x) {
  time_t t = x_atoi(x);
  time_t deedline = time(0) + t;
  if (t<=0) return;
  if (cfg_verbose>1) msg("Sleeping: ", x);
  while (1) {
    nano_sleep(1,0);
    if (time(0) >= deedline) break;
  }
}

static void print_escape(char *x, int wall) {
  char ch, *y, *start, *from = "abefnrtv\\c", *to= "\a\b\033\f\n\r\t\v\\";
  unsigned long len;
  start = alloca(str_len(x) + 3);
  for (y=start; (ch=*x); x++) {
    if (ch == '\\') {
      unsigned long u;
      len = scan_8ulong(x+1, &u);
      if (len) { /* 012...9 */
	ch = u;
	x += len;
      } else {
	len = str_chr(from, x[1]);
	switch (from[len]) {
	  case 0: break;
	  case 'c': goto do_it;
	  default: /* \a\b... */ ++x; ch = to[len];
	}
      }
    }
    *y++ = ch;
  }
  *y++ = '\n';

 do_it:
  len = y-start;

  if (wall==0) write(1, start, len);
  else {
    struct utmp u;
    int fd, r;
    char line[8 + sizeof(u.ut_line)];

    fd = open(_PATH_UTMP, O_RDONLY | O_NOCTTY);
    if (fd < 0) return;

    while (utmp_io(fd, &u, F_RDLCK)) {
      if (u.ut_type != USER_PROCESS) continue;
      if (u.ut_user[0] == 0 || u.ut_pid < 2 ||
	  u.ut_line[0] == 0 || u.ut_line[0] == '/') continue; 

      y = line;
      y += str_copyn(y, "/dev/", 8);
      y += str_copyn(y, u.ut_line, sizeof(u.ut_line));
      y[0] = 0;
      if (line[str_chr(line, '.')]) continue;

      if (kill(u.ut_pid, 0) && errno == ESRCH) continue;
      r = open(line, O_WRONLY | O_NOCTTY | O_NDELAY);
      if (r < 0) continue;

      write(r, start, len);
      close(r);
    }
    close(fd);
  }
}

#define Kill_Help \
"Usage:\n",me, \
" -[vq] [-s secs] [-M|W mesg] [-signum] ... [-E program [arg[s]]\n" \
"\t-s secs:\tsleep secs\n" \
"\t-M mesg:\twrite mesg to stdout\n" \
"\t-W mesg:\twrite mesg to logged users\n" \
"\t-sugnum:\tsignal number\n" \
"\t-v[v]:\t\tbe verbose\n" \
"\t-q:\t\tquiet mode; ignores SIGINT signal\n\n" \
"Some signals can be codded. Only the first letter is important!\n" \
"\t-p:\tSIGSTOP\n" \
"\t-c:\tSIGCONT\n" \
"\t-h:\tSIGHUP\n" \
"\t-a:\tSIGALRM\n" \
"\t-i:\tSIGINT\n" \
"\t-t:\tSIGTERM\n" \
"\t-k:\tSIGKILL\n\n" \
"Option -E must be last.  Examples:\n  ", \
me," -s2 -M'Sending all processes SIGTERM ...' -term -cont \\\n" \
"\t-s6 -M'Sending all processes SIGKILL ...' -9 \\\n" \
"\t-s1 -E /path/to/program arg1 arg2 ...\n\n  ", \
me," -q -vv -s1 -15 -cont -s6 -kill -s1" \
" -E/path/to/prog arg(s) ..."

int main(int argc, char **argv) {
  char *x, *me = argv[0];
  char *last_msg = "\\c";
  errmsg_iam(me);
  if (argc<2) {
  help:
    errmsg_iam(0);
    carp(Kill_Help);
    _exit(1);
  }
  argv++;
  x = argv[0];
  if (x[0]=='-' && x[1]=='h') goto help; /* -h... --help */

  opendevconsole();

  /* ignore next signals */
  set_sa(SIGQUIT); set_sa(SIGCHLD); set_sa(SIGHUP );
  set_sa(SIGTSTP); set_sa(SIGTTIN); set_sa(SIGTTOU);

  while (argv[0] && argv[0][0]=='-') {
    int sig=0, cfg_wall=0;
    char *y = argv[0] + 1;
    if ((unsigned int)(*y - '0') < 10) { 
      sig = x_atoi(y);
      goto again;
    }
    
    switch(*y) {
    case 'v': cfg_verbose++; if (y[1]=='v') cfg_verbose++; break;
    case 'q': set_sa(SIGINT); break;
    case 's': chk_opt(argv,x); deep_sleep(x); break;
    case 'W': ++cfg_wall;
    case 'M': chk_opt(argv,x);
      if (x[0] == '%' && x[1] == 0) x = last_msg;
      else last_msg = x;
      print_escape(x, cfg_wall); 
      break;
    case 'E':
      chk_opt(argv,x); *argv = x;
      execve(argv[0], argv, environ);
      carp("Unable to exec: ", argv[0]);
      _exit(1);
    default:
#ifdef NSVC_SIGNAL_CODED
      {
	unsigned char *S, *Sig = (unsigned char *)NSVC_SIGNAL_CODED;
	for (S=Sig; *S; S += 2)
	  if ((unsigned char)*y == *S) { sig = S[1]; goto again; }
      }
#else
#define kk(C,S) case C: sig=S; break
	kk('t',SIGTERM); kk('a',SIGALRM); kk('p',SIGSTOP); kk('c',SIGCONT);
	kk('h',SIGHUP);  kk('i',SIGINT);  kk('k',SIGKILL);
#endif
      goto help;
    }

  again:
    if (sig > 0) {
      sync();
      if (cfg_verbose) msg("Sending signal: ", y);
      if (kill(-1, sig)) carp("Unable to send signal: ", y);
    }
    argv++;
  }
  sync();
  return 0;
}
