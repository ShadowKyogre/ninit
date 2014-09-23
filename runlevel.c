#include <unistd.h>
#include <utmp.h>
#include <time.h>
#include <alloca.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include "ninitfeatures.h"
#include "utmp_defs.h"

static unsigned long env_free, env_allocated;
#include "contrib/put_env.h"

static void init_utmp(struct utmp *u) {
  byte_zero(u, sizeof(struct utmp));
  str_copy(u->ut_line,"~");
  str_copy(u->ut_id,"~~");
  u->ut_tv.tv_sec=time(0);
}

int main(int argc, char **argv) {
  pid_t prev=0;
  int i,fd;
  char **env, ch, prevlevel[20], ninit_runlevel[20];
  char boot_time=0;
  struct utmp u;
  struct utsname uname_buf;
  off_t pos=0, runlevel_pos=0;

  str_copy(prevlevel, "PREVLEVEL=N");
  str_copy(ninit_runlevel,  "NINIT_RUNLEVEL=N");

  fd=open(_PATH_UTMP, (argc>1) ? O_RDWR : O_RDONLY);
  while (utmp_io(fd, &u, F_RDLCK)) {
    if (u.ut_type == BOOT_TIME) boot_time = 1;
    if (u.ut_type == RUN_LVL) {
      prev = u.ut_pid;
      runlevel_pos = pos;

      if (argc < 2) {
	char m[] = "N N\n";
	i = prev / 256; if (i) m[0] = i;
	i = prev % 256; if (i) m[2] = i;
	write(1, m, 4);
	return 0;
      }
    }
    pos += sizeof(struct utmp);
  }

  if (argc < 2) _exit(1);
  prev %= 256;
  ch = argv[1][0];

  if (boot_time == 0) {
    init_utmp(&u);
    u.ut_type=BOOT_TIME;
    str_copy(u.ut_user,"reboot");
    if (lseek(fd,pos,SEEK_SET) == pos)
      utmp_io(fd,&u,F_WRLCK);
  }

  pos = runlevel_pos;
  if (pos == 0 && lseek(fd,0,SEEK_SET) == 0) {
    while (utmp_io(fd, &u, F_RDLCK)) {
      if (u.ut_type == RUN_LVL) break;
      pos += sizeof(struct utmp);
    }
  }

  init_utmp(&u);
  u.ut_type=RUN_LVL;
  str_copy(u.ut_user,"runlevel");
  if (prev==0) prev='N';
  u.ut_pid = (256*prev) + ch;

  if (lseek(fd,pos,SEEK_SET) == pos)
    utmp_io(fd,&u,F_WRLCK);
  close(fd);
    
  if (uname(&uname_buf) == 0)
    str_copyn(u.ut_host, uname_buf.release, 32); /* XXX overflow ? */
  do_wtmp(&u);
  
  /* ------ exnivon + exeve ----- */
  if (environ==0) environ = argv+argc;
  for (env=environ; *env; ++env) env_allocated++;

  env_free = argc + 12;
  env=alloca((env_allocated + env_free + 1) * sizeof(char*));
  if (env==0) return -1;
  byte_copy(env, (env_allocated+1)*sizeof(char *), environ);
  environ = env;
  
  prevlevel[10] = prev;
  ninit_runlevel[15] = ch;

  put_env(prevlevel);
  put_env(ninit_runlevel);
  put_env(ninit_runlevel+6);
  put_env("INIT_VERSION=" SYSVINIT_VERSION);
  put_env("CONSOLE=/dev/console");
  put_env("PATH=/bin:/usr/bin:/sbin:/usr/sbin");
  
  for (i=2; i<argc; ++i) {
    char *v=argv[i];
    char *nenv=0;
    if (v[0]=='-') {
      if (v[1]==0) {
	environ[0]=0;
      } else {
	switch (*++v) {
	case 'i': environ[0]=0; goto do_it;
	case 'u':
	  if (v[1]) { nenv = v+1; goto do_it; }
	  else if (argv[++i]) { nenv = argv[i]; goto do_it; }
	default:
	  goto ready;
	}
      }
    } else {
      if (v[str_chr(v,'=')] == 0) goto ready;
      nenv = v;
    }

  do_it:
    if (nenv) put_env(nenv);
    continue;
  }

 ready:
  argv += i;
  if (argv[0]) {
    execve(argv[0], argv, environ);
    return 100;
  }
  return 0;
}
