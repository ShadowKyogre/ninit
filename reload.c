#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/poll.h>
#include <limits.h>
#include "ninit.h"

#define USAGE "Usage:\n" \
"\tninit-reload ","-v","\n" \
"\tninit-reload ","-m > dump_memory_file\n" \
"\tninit-reload ","-d > dump_data_file\n" \
"\tninit-reload ","[Options] /path/to/ninit [arg(s)...]\n", \
"\nOptions:\n\t[-t time_diff]\t[-f data_file]\t" \
"[-u]\t\t[-v]\n\t" \
"[-R service]\t[-r number]\t[-a auto-size]\n\t" \
"[-E file]\t[-e var=name]\t[-e variable]\t[-e -]\n"

#define UNKNOWN_PROCESS_SIZE

static char *buf, *flag_file;
static unsigned int verbose;
static char do_update, get_memory, get_data;

#define UL sizeof(uint32t)

void die_xx(char *s) { errmsg_iam("ninit-reload"); err(2,s,0); _exit(1); }

static int safe_read_code(char code) {
  int r, k;
  for (k=0; k<50; k++) {
    r = read(outfd,buf,BUFFER_TMP_LEN);
    if (r == 1 && buf[0] == code) return 0;
    if (r <= 0) nano_sleep(0, 20000000); /* 0.020 sec */
    else break;
  }
  return -1;
}
static void safe_read() { 
  if (safe_read_code('1')) die_xx("wrong responce from ninit");
}
static void safe_read_execve(char code, char *s) {
  if (safe_read_code(code) == 0) return;
  die(1, "Ops! I got exec error from ",s);
}
static void safe_write(int len) {
   if (len != write(infd, buf, len)) die_xx("error writing to ninit");
}
static int safe_cat(char *dest, char *src, int len) {
  int k = str_len(dest);
  if (k<len) k += str_copyn(dest+k, src, len-k);
  dest[k] = 0;
  return k;
}
static void put_arg(char *string) {
  uint32t *len = (uint32t *)buf;
  buf[UL] = 0;
  *len = safe_cat(buf+UL, string, 300);
  safe_write(*len + UL);
  safe_read();
}

#include "check_opt.h"
#include "get_services.h"
#include "open_inout.h"

static char *do_AutoSize(uint32t len) {
  uint32t u;
  char *x;
  if (len>300) len=300;
  u = (maxprocess + len + 8) * (PROCESS_SIZE + 12);

  x = fu(u & ~15);
  *--x = 'M';  *--x = '-';

#ifndef INIT_TIMEOUT_WRITE
  if (u > PIPE_BUF - 256)
    carp("\n\a*** WARNING *** for option ",x, " ninit must be build\n"
	 "with INIT_TIMEOUT_WRITE flag.  See ninitfeatures.h");
#endif
  return x;
}

static char *align_name(char *s) {
  static char x[20];
  int len = str_len(s);
  byte_set(x,sizeof(x),' ');
  if (len<=16) len=18 - len;
  else { len=1; x[0] = '\t'; }
  x[len] = 0;
  return x;
}

int main(int argc, char **Argv) {
  char *remove_P[argc], *env_c[argc], **env=0;
  char *auto_memory=0, **argv;
  time_t TimeDiff=0;
  int do_Remove=0, do_remove=0, remove_p[argc], AutoSize=0;
  int i,j,k,env_len=0;
  uint32t *len;
  char *initroot, *x;
 
  INIT_ENV_GET_HOME(initroot,"NINIT_HOME","INIT_HOME");
  buf = alloca(BUFFER_TMP_LEN);
  len = (uint32t *)buf;

  mem.a = INIT_ALLOC_BUFFER;
  if (argc < 2) { usage: die(1, USAGE); }
  if (initroot == 0) initroot = INITROOT;
  argv=Argv;

  argv++;
  errmsg_iam("ninit-reload");
  while (argv[0] && argv[0][0]=='-') {
    switch(argv[0][1]) {
    case 'v': verbose++; break;
    case 'u': do_update=1; break;
    case 'm': get_memory=1; break;
    case 'd': get_data=1; break;
    case 'f': chk_opt(argv,x); flag_file = x; break;
    case 't': chk_opt(argv,x); TimeDiff = x_atoi(x); break;
    case 'a': chk_opt(argv,x); AutoSize = x_atoi(x); break;
    case 'R': chk_opt(argv,x); remove_P[do_Remove++] = x; break;
    case 'e': chk_opt(argv,x);
      /* next clears all like env -i!  to remove '-' do; 
       * echo > /tmp/x; ninit-raload -E /tmp/x ... */
      if (x[0]=='-' && x[1]==0) x=""; 
      env_c[env_len++] = x; break;
    case 'E': chk_opt(argv,x);
      i=open(x, O_RDONLY);
      if (i<0) die(1, "unable to open ", x);
      else {
	if (GLOBAL_READ(i,x, j,12000)) _exit(1);
	close(i);
	x[j] = 0;

	j = splitmem(0,x,'\n');
	env = alloca((argc + j + 5) * sizeof(char *));
	if (env == 0) _exit(1);
	splitmem(env,x,'\n');
	skip_comments(env);
      }
      break;
    case 'r':
      chk_opt(argv,x);
      if ('0'<=x[0] && x[0]<='9') remove_p[do_remove++] = x_atoi(x);
      break;
    default:
      die(1, "Unknown Option: ",argv[0]);
    }
    argv++;
  }

  argc -= (argv-Argv);

  x = env_get("NINIT_MEMORY");
  if (x) mem.a = atoulong(x);
  mem.r=mem.a;
  if (mem.a==0 || mem.a > 64000 || (mem.x=alloca(mem.a))==0) _exit(1);

  if (env==0) env=env_c;
  else {
    char **xx=env+1, *s;
    for (k=1; (s=*xx); xx++) if (*s) env[k++] = s;
    byte_copy(env+k, env_len * sizeof(char *), env_c);
    env_len += k; 
  }

  if (flag_file==0) open_inout(initroot);
  else {
    if (*flag_file == 0) goto usage;
    infd=-1;
    outfd=open(flag_file, O_RDONLY);
    if (outfd<0) die(1, "could not open ",flag_file);
    process_size = lseek(outfd,0,SEEK_END);
    lseek(outfd,0,SEEK_SET);
  } 

  errmsg_iam(0);
  if (get_memory || get_data) {
    struct pollfd pfd;
    pfd.fd=outfd;
    pfd.events=POLLIN;
    buf[0] = 'D';
    buf[1] = (get_memory) ? 'M' : '1';
    write(infd,buf,2);

    for (;;) {
      j=poll(&pfd,1,400);
      if (j==-1) {
	if (errno==EINTR) { carp("interrupt signal"); }
    	else die_xx("poll fialed!");
      } else if (j==1) {
	i=read(outfd,buf,BUFFER_TMP_LEN);
	if (i>0) write(1,buf,i);
      } else {
	return 0;
      }
    }
  }
  
  get_services();

  if (verbose) {
    char *what;
    msg("services from: ",
	(flag_file) ? "" : initroot, 
	(flag_file) ? flag_file : "/out");
    for (j=0; j<= maxprocess; j++) {
      switch (root[j].pid) {
      case 1: what = "finish"; break;
      case 0: what = "down"; break;
      default: what = fu(root[j].pid);
      }
      x = root_name(j);
      msg(fu(j),".\t",x,align_name(x),what);
    }
    if (!do_update) return 0;
  }

  if (do_update) {
    if (!argv || !argv[0] || argv[0][0]!='/')
      die_xx("I need the full /path/of/ninit/program");
    
    k = str_len(argv[0]);
    if (k > 240) die_xx("too long file name.  must be <= 240.");
    
    if (access(argv[0], X_OK))
      die(1, "Ops! Check the mode of ", argv[0]);

    if (AutoSize) auto_memory=do_AutoSize(AutoSize);
    
    errmsg_puts(1,"\nreplacing current ninit with: ");
    fmt_argv(1,argv, " ");
    msg("",auto_memory);

    if (flag_file) {
      close(infd); close(outfd);
      open_inout(initroot);
    }

    buf[0]='U'; buf[1]='U'; buf[2]=0;

    if (argc>1 || auto_memory || env_len) {
      /* ----- to ninit ----- */
      safe_cat(buf, initroot, 200);
      k = safe_cat(buf, "/sys/update", 230);
      if (access(buf+2, X_OK))
	die(1, "Ops! Check the mode of ", buf+2);

      buf[1] = 0;
      buf[k++] = 0;
      if (verbose) msg("sending reload signal to ","ninit");
      safe_write(k);
      
      /* ----- from/to update ----- */
      safe_read_execve('7', "~/sys/update");
      *len=argc + 1 + (auto_memory != 0);
      safe_write(UL);
      safe_read();
      
      for (i=0; i<argc; i++) put_arg(argv[i]);	/* argv */
      if (auto_memory) put_arg(auto_memory);
      put_arg("");
      
      *len=env_len;
      safe_write(UL);
      safe_read();
      for (i=0; i<env_len; i++) put_arg(env[i]);
      
      *len = 1953066852;
      if (verbose) msg("sending reload signal to ","~/sys/update","\n");
      safe_write(UL);
    } else {
      k=safe_cat(buf, argv[0], 300);
      buf[1] = 0;
      buf[k++] = 0;
      if (verbose) msg("sending reload signal to ","ninit","\n");
      safe_write(k);	/* execve argv[0] */
    }      

    /* ----- from/to ninit ----- */
    safe_read_execve('8',argv[0]);
    if (verbose) msg("now trying to restore services:");
    
    for (j=0; j<= maxprocess; j++) {
      char *name=root_name(j);

      for (k=0; k<do_remove; k++)
	if (j==remove_p[k]) {
	  if (verbose) { msg("\t",name,align_name(name),"removed"); }
	  k=-1; break;
	}
      if (k==-1) continue;

      for (k=0; k<do_Remove; k++)
	if (!str_diff(remove_P[k],name)) {
	  if (verbose) { msg("\t",name,align_name(name),"removed"); }
	  k=-1; break;
	}
      if (k==-1) continue;

      root[j].pr_circular = 0;
      root[j].startedat += TimeDiff;
      buf[0]='U'; buf[1]=0;
      k = 1 + safe_cat(buf, name, 300);
      byte_copy(buf + k, PROCESS_SIZE, &root[j]);
      k += PROCESS_SIZE;

      safe_write(k);
      safe_read();
      if (verbose) { msg(fu(j),".\t",name,align_name(name),"done"); }
    }

    if (verbose) errmsg_puts(1,"\n");
    msg("Done!\tTry: nsvc -L");
    return(0);
  }
  goto usage;
  return 0;
}
