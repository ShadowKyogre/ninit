#include <unistd.h>
#include <fcntl.h>
#include <alloca.h>
#include "ninitfeatures.h"

static int infd=-1, outfd=-1;
static uint32t env_free=0;
#include "contrib/put_env.h"

static void die_xx(char *s) {
  char *aa[3]; 
  INIT_ARGS3(aa, "/sbin/ninit","",0);
  write(2,"\nupdate: ",9);
  write(2,s,str_len(s));
  write(2,"\n",1);
  while (1) {
    execve(*aa,aa,environ);	/* really panic !!! */
    nano_sleep(0,800000000);
  }
}
static void safe_read(void *buf, int len) {
  if (read(infd,buf,len) != len) die_xx("error reading");
}
static void safe_write() {
  if (write(outfd,"1",1) != 1) die_xx("error writing");
}

int main(int Argc, char **Argv) {
  uint32t len,i,k;
  char **argv, **env, *s;

  s = Argv[0];
  s[str_rchr(s, '/')] = 0;
  chdir(s);

  infd=open("../in",O_RDWR);
  outfd=open("../out",O_RDWR|O_NONBLOCK);
  fcntl(infd,F_SETFD,FD_CLOEXEC);
  fcntl(outfd,F_SETFD,FD_CLOEXEC);

  if (infd<0 || outfd<0) {
    write(outfd,"0",1);
    die_xx("pipes error");
  }
  write(outfd,"7",1);
  
  safe_read(&len,sizeof(len)); 
  argv = alloca((len+5) * sizeof(char *));
  if (argv==0) die_xx("out of memory");
  safe_write();

  for (i=0; i<len; i++) {	/* argv */
    safe_read(&k, sizeof(k));
    if (k) {
      if ((s=alloca(k+1))==0) die_xx("out of memory");
      safe_read(s,k);
      s[k] = 0;
    } else s = "";
    argv[i] = s;
    safe_write();	
  }
  argv[i] = 0;

  /* ----- environ ----- */
  safe_read(&len,sizeof(len)); 
  if (len==0) { safe_write(); goto ready; }

  env_free=len+1;
  if (environ==0) environ = Argv+Argc;
  for (k=0; environ[k]; ) k++;
  env = alloca((len+k+5) * sizeof(char *));
  if (env==0) die_xx("out of memory");

  byte_copy(env, (k+1)*sizeof(char *), environ);
  environ=env;
  safe_write();

  for (i=0; i<len; i++) {	/* environ */
    safe_read(&k, sizeof(k));
    if (k==0) environ[0] = 0;
    else {
      if ((s=alloca(k+1))==0) die_xx("out of memory");
      safe_read(s,k);
      s[k] = 0;
      put_env(s);
    }
    safe_write();	
  }
  
 ready:
  safe_read(&k,sizeof(k));	/* k = "doit" */
  chdir("/");
  execve(*argv, argv, environ);
  die_xx("PANIC");
  return 1;
}
