#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include "ninitfeatures.h"

#define SERVICE Argv[1]
#define HOME	Argv[2]
#define SYSDIR	Argv[3]

static unsigned long env_free, env_len;
static int infd, outfd;

#include "process_defs.h"
#include "open_inout.h"
#include "tryservice.h"
#include "contrib/put_env.h"
#include "wait_services.h"

#define NI_TEXT_FILES 4
enum { NI_environ,NI_softlimit,NI_wait,NI_cron };

int main(int Argc, char **Argv) {
  char **env, **xx, *s;
  char *tmp[NI_TEXT_FILES];
  static char **aa[NI_TEXT_FILES];
  int i,fd,len;

  errmsg_iam("run-wait");
  if (Argc<4) {
  usage:
    carp("usage: ",errmsg_argv0," service home sys");
    _exit(1);
  }

  if (chdir(HOME) || chdir(SERVICE)) goto usage;
  INIT_ARGS4(tmp, "environ","softlimit","wait","cron");

  for (i=0; i < NI_TEXT_FILES; i++) {
    int offset=0;
    if ((fd = open(tmp[i], O_RDONLY)) >=0) {
      if (i==0) {
	if (environ==0) environ = Argv+Argc;
	for (env=environ; *env; ++env) env_len++;
	offset = env_len + 3;
      }

      if (GLOBAL_READ(fd,s, len,24000)) _exit(1);
      close(fd);
      s[len]=0;
      
      len = splitmem(0,s,'\n');			  if (i==0) env_free=len;
      xx=alloca((len+offset+1) * sizeof(char*));  if (xx==0) _exit(1);
      xx += offset;
      aa[i] = xx;
      splitmem(xx,s,'\n');
      skip_comments(xx);
    }
  }

  if (aa[NI_environ]) {
    env = aa[NI_environ] - (env_len + 3);
    byte_copy(env, (env_len+1)*sizeof(char *), environ);
    environ = env;
    
    env = aa[NI_environ];
    if (**env==0) environ[0]=0;
    for (; *env; env++) put_env(*env);
  }

  if (aa[NI_softlimit]) {
    errmsg_iam("run-wait: softlimit");
    softlimit(aa[NI_softlimit]);
  }

  if (aa[NI_wait]) wait_services(aa[NI_wait], HOME);

  if ((xx=aa[NI_cron])) {
    unsigned long cr[2];
    struct process pr[6];
    cron(xx, cr);
    if (cr[0]) {
      len = tryservice(HOME,SERVICE,"c",fu(cr[0]), pr);
      if (cr[1] == 0 && len == sizeof(pr[0]) && pr->pr_finish == 0) return 0;
    }
  }

  Argv[0] = "-run";
  if (!access("pause-wait", R_OK)) kill(getpid(), SIGSTOP);
  if (chdir(HOME) || chdir(SYSDIR)) return 1;
 
  execve("./run",Argv,environ);
  return 1;
}
