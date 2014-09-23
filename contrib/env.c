#include <unistd.h>
#include <alloca.h>
#include "../ninitfeatures.h"
#include "../djb/buffer.h"
#include "../error_table.h"

static unsigned long env_free;
#include "put_env.h"

#define BUFFER_1_SIZE 1200
buffer b = BUFFER_INIT(write, 2, 0, BUFFER_1_SIZE);

#undef die
#define die(n,...) err_b(&b,__VA_ARGS__,0); buffer_flush(&b); _exit(n)

int main(int argc,char *argv[]) {
  int i;
  char *nenv, **env;

  b.x=alloca(BUFFER_1_SIZE);	/* XXX if it fails bummer */
  errmsg_iam("env");
  if (environ==0) environ = argv+argc;
  for (env=environ; *env; ++env) env_free++;

  env=alloca((argc+env_free+3) * sizeof (char*));
  if (env==0) { die(1, "Out of memory"); }

  byte_copy(env, (env_free+1) * sizeof(char *), environ);
  environ = env;
  env_free = argc;

  for (i=1; i<argc; ++i) {
    char *v=argv[i];
    if (v[0]=='-') {
      if (v[1]==0) {
	environ[0]=0;
      } else {
	for (++v; *v; ++v)
	  switch (*v) {
	  case 'i': environ[0]=0; break;
	  case 'u':
	    if (v[1]) { nenv = v+1; goto do_it; }
	    else if (argv[++i]) { nenv = argv[i]; goto do_it; }
	  default:
	    die(1, "usage: env [-] [-i] [-u] [NAME=VALUE...] "
		 "[program args...]");
	  }
      }
    } else {
      if (v[str_chr(v,'=')]) { nenv = v; goto do_it; } 
      else {
	pathexec_run(v,argv+i,environ);
	die(127, "unable ro run: ",v,": ",error_string(table,errno));
      }
    }
    continue;

  do_it:
    put_env(nenv);
  }

  for (b.fd = 1; *environ; ++environ) {
    buffer_puts(&b, *environ);
    buffer_puts(&b, "\n");
  }
  buffer_flush(&b);
  return 0;
}
