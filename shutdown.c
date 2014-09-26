/*
 * Notes:
 * - uncomment `#define USE_INIT' below if you want to use shutdown
 *   with ninit. If defined, shutdown will try to bring down the services 
 *   halt (for -h or -o) or reboot (-r) before killing all other processes.
 * - If you do not use ninit shutdown will bring your system down similar
 *   to SysV-Inits shutdown with -n
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <alloca.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdarg.h>
#include "ninitfeatures.h"
#include "uid.h"
#include "check_opt.h"
#define Y(...) exec_cmd(__VA_ARGS__, 0)

#define USE_INIT

static char *me;
static int cfg_verbose;

void sighandler(int sig) { (void)sig; }

static void exec_argv(char **argv) {
  execve(argv[0], argv, environ);
  errmsg_iam(me);
  carp("execve ", argv[0]," failed"); 
}

static void exec_cmd(char *cmd, ...) {
  char *argv[25], **arg=argv; /* XXX can overflow */
  const char *s;
  va_list a;
  pid_t pid;
  int count_args = 0;
  
  *arg++ = cmd;
  count_args++;
  va_start(a,cmd);
  do {
    s=va_arg(a,const char*);
    *arg++ = (char *)s;
    count_args++;
  } while (s && count_args < 25);

  while ((pid = fork()) < 0) nano_sleep(0,500000000);
  if (pid == 0) {
    exec_argv(argv);
    _exit(0);
  } else
    while (pid != waitpid(-1,0,0)); /* also collect zombies */
}

static void deep_sleep(int t) {
  time_t deedline = time(0) + t;
  if (t<=0) return;
  if (cfg_verbose) msg("sleeping ", fu(t), "...");
  while (1) {
    nano_sleep(1,0);
    if (time(0) >= deedline) break;
  }
}

static void die_xx(char *s) { errmsg_iam(me); carp(s); _exit(1); }

#ifdef USE_INIT
#include "ninit.h"

static int infd, outfd;
static char *buf, do_update, *nsvc_other;
static unsigned int cfg_twice;
static pid_t my_pid;

#include "get_services.h"
#include "open_inout.h"
#undef CLEAN_SERVICE
#include "tryservice_nsvc.h"

static void try_srv(char *service, char *type) {
  if (tryservice(service, type[0]))
    carp("Could not ", type, " service: ", service);
}

void ninit_servicesDown(char **skip) {
  int idx;
  for (idx=0; idx<=maxprocess; idx++) {
    char *what = "not running";
    pid_t pid = root[idx].pid;
    char *name = root_name(idx);

    if (root[idx].pr_respawn) try_srv(name, "respawn off");
    if (root[idx].cron) {
      nsvc_other = "0";
      try_srv(name, "cron off");
      nsvc_other = 0;
    }
    if (pid > 1) {
      char **x;
      for (x=skip; *x; x++)
	if (!str_diff(*x, name)) { pid=my_pid; what="skipped"; break; }

      if (my_pid != pid) {
	what = "done";
	if (kill(pid, SIGTERM) == 0) kill(pid, SIGCONT);
	else what = "failed";
      }
    }
    if (cfg_verbose) msg("\t--> ", name, "\t\t", what);
  }
}

void ninit_shutdown(unsigned int twice, char **skip) {
  char *x;
  INIT_ENV_GET_HOME(x,"NINIT_HOME","INIT_HOME");
  if (x==0) x=INITROOT;
 again:
  open_inout(x);
  msg("Shutting down ninit services");  

  get_services();
  ninit_servicesDown(skip);

  if (twice) {
    close(infd);
    close(outfd);
    mem.l = 0;
    mem.r = mem.a;
    root = 0;
    maxprocess = -1;
    deep_sleep(twice);
    twice = 0;
    goto again;
  }
}
#endif

static void printUsage() {
  carp("usage: ",me," -[rhoqvt"
#ifdef USE_INIT
       "mST"
#endif
       "] [-E program [arg(s)]]\n"
       "\t-r:\treboot after shutdown\n"
       "\t-h:\thalt after shutdown\n"
       "\t-o:\tpower-off after shutdown\n"
       "\t-q:\tquiet mode; ignores SIGINT signal\n"
#ifdef USE_INIT
       "\t-m:\tonly shutdown the ninit part\n"
#endif
       "\t-v:\tbe verbose\n"
       "\t-E prog:\texecute prog after KILLALL;"
       "  must be last option!\n"
       "\t-s secs:\tstarting delay\n"
       "\t-t secs:\tdelay between SIGTERM and SIGKILL\n"
#ifdef USE_INIT
       "\t-T secs:\tif secs is nonzero shutdown twice ninit part\n"
       "\t-S abcd:\tskip to shutdown the service abcd\n"
#endif
       );
  _exit(1);
}

int main(int argc, char **argv) {
  char *x;
  unsigned int cfg_downlevel=2; /* 0:1:2 == reboot:halt:poweroff */
  unsigned int cfg_delay = 3;
  static unsigned int cfg_sleep;
  static int cfg_ninitonly, cfg_quiet;
  static char **prog;

#ifdef USE_INIT
  static char **skip, **skp;
  skip = alloca((argc+5) * sizeof(char *));
  skp = ++skip;

  mem.a = INIT_ALLOC_BUFFER;
  buf = alloca(BUFFER_TMP_LEN);
#endif

  if (getuid() != 0) die_xx("You are not root, go away!");
  me = argv[0];
  if (argc<2) printUsage();

  /* parse commandline options */
  argv++;
  while (argv[0] && argv[0][0]=='-') {
    char *y=argv[0] + 1;
    for (; *y; y++) {
      switch(*y) {
      case 'q': /* ignore SIGINT */	++cfg_quiet; break;
      case 'v': /* verbose mode */	++cfg_verbose; break;
      case 'r': /* reboot */	cfg_downlevel = 0; break;
      case 'h': /* halt */	cfg_downlevel = 1; break;
      case 'o': /* poweroff */	cfg_downlevel = 2; break;
      case 't': /* delay between SIGTERM and SIGKILL */
	chk_opt(argv,x); cfg_delay = x_atoi(x); goto again;
      case 's': /* sleep before shutdown services */
	chk_opt(argv,x); cfg_sleep = x_atoi(x); goto again;
#ifdef USE_INIT
      case 'm': /* exit after ninit down */
	cfg_ninitonly = 1; break;
      case 'T': /* delay between two services down */
	chk_opt(argv,x); cfg_twice = x_atoi(x); goto again;
      case 'S': /* skip services */
	chk_opt(argv,x); *skp++ = x; goto again;
      case 'E': /* execve prog arg(s) */
	chk_opt(argv,x); prog = argv; *prog = x; goto last_option;
#endif
      default:
	printUsage();
      }
    }
  again:
    argv++;
  }

#ifdef USE_INIT
 last_option:
  *skp = 0;
  *--skip = cfg_downlevel ? "halt" : "reboot";
#endif

  opendevconsole();

  if (cfg_ninitonly == 0) {
    switch (cfg_downlevel) {
    case 0: x="reboot"; break;
    case 1: x="halt"; break;
    case 2: x="power-off";
    }
    msg(me, "\007: System is going down for ",x);
  }
  sync();

  /* catch some signals; getting killed after killing the controlling terminal
   * wouldn't be such a great thing... 
   */
  set_sa(SIGQUIT); set_sa(SIGCHLD); set_sa(SIGHUP );
  set_sa(SIGTSTP); set_sa(SIGTTIN); set_sa(SIGTTOU);

  if (cfg_quiet) set_sa(SIGINT);
  deep_sleep(cfg_sleep);

#ifdef USE_INIT
  x = env_get("NINIT_MEMORY");
  if (x) mem.a = atoulong(x);
  mem.r=mem.a;
  if (mem.a==0 || mem.a > 64000 || (mem.x=alloca(mem.a))==0) _exit(1);
  my_pid = getpid();

  sync();

  ninit_shutdown(cfg_twice, skip);
  if (cfg_ninitonly == 0 && prog == 0) {
    msg("Starting service: ", *skip);
    try_srv(*skip, "start");
  }
  close(infd);
  close(outfd);
  sync();

  if (cfg_ninitonly) {
    if (prog) exec_argv(prog);
    _exit(0);
  }
  deep_sleep(cfg_delay);
#endif

  /* kill all processes still left */
  sync();
  msg("Sending all processes the ", "TERM signal...");
  kill(-1, SIGTERM);
  deep_sleep(cfg_delay);

  sync();
  msg("Sending all processes the ", "KILL signal...");
  kill(-1, SIGKILL);

  if (prog) { sync(); exec_argv(prog); }
  sync();

  /* maximal 20 arguments !!! */
  if (!access("/sbin/quotaoff", X_OK)) Y("/sbin/quotaoff", "-a");
  Y("/sbin/swapoff", "-a");	sync();
  Y("/bin/umount", "-a");
  Y("/bin/mount", "-o", "remount,ro", "/");
  sync();

  /* and finally reboot, halt or power-off the system */
  switch (cfg_downlevel) {
  case 0: cfg_downlevel = RB_AUTOBOOT; break;
  case 1: cfg_downlevel = RB_HALT_SYSTEM; break;
  case 2: cfg_downlevel = RB_POWER_OFF;
  }

  set_reboot(cfg_downlevel);
  return 0;
}
