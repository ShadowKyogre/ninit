/* serdo.c 
   modified by Nikola Vladov
   unset VAR
   exit n
   exec file
   # comment
   ps -ax # comment ps
   long line \
      continues here
   echo something
   . file
   VAR=val ... command ...
   killall5 -signumber
 */

#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include "../ninitfeatures.h"
#include "../error_table.h"

struct cmd { 
  char *pos;
  char *x;
  char last;
  char eof;
};

static unsigned int env_free, last_cmd;
static char continueonerror;
static int batch(char *s);

#include "put_env.h"

static char *e() { return error_string(table, errno); }

#define str_equal(A,B) !str_diff(A,B)
#define byte_equal(A,l,B) !byte_diff(A,l,B)
#define is_space(c) *c==' ' || (unsigned int)(char)(*c - 9)<5
#define carpsys(...) err(2,__VA_ARGS__,": ",e(),(char*)0)
#define diesys(n,...) do { err(2,__VA_ARGS__,": ",e(),(char*)0); _exit(n); } while(0)

#define DQUOTE '"'
#define SQUOTE '\''

static void pr_argv(int fd, char **argv) {
  char *s;
  while ((s=*argv++)) {
    unsigned int len = str_len(s);
    if (*argv) s[len++] = ' ';
    errmsg_put(fd,s,len);
  }
  errmsg_puts(fd,"\n");
  errmsg_puts(fd,0);
}

static int pr_fail(char **argv) {
  carpsys(argv[0]);
  if (argv[1]) pr_argv(2, argv);
  return -1;
}

static void put__env(char *s) { if (put_env(s)) die(1, "Out of memory"); }

static int spawn(char **argv) {
  int i;
  char *s0=argv[0], *s1=argv[1], cfg_exec=0, ignore=0;

  if (str_equal(s0,"cd")) {
    if (chdir(s1)) return pr_fail(argv); 
    return 0;
  } else if (str_equal(s0,"export") || str_equal(s0,"unset")) {
    while (*++argv) put__env(*argv);
    return 0;
#ifdef SERDO_WANT_echo
  } else if (str_equal(s0,"echo")) {
    pr_argv(1, argv+1);
    return 0;
#endif
  } else if (s1) {
    i = x_atoi(s1);
    if (str_equal(s0,"exit")) { _exit(i); }
    else if (str_equal(s0,"exec")) { cfg_exec++; ++argv; }
    else if (s0[0]=='.' && s0[1]==0) { return batch(s1); }
#ifdef SERDO_WANT_killall5
    else if (str_equal(s0,"killall5")) { return kill(-1,-i); }
#endif
  }

  if (argv[0][0] == '-') { argv[0] += 1; ignore = 1; }

  if (last_cmd && !cfg_exec) {
    struct timespec ts = { 0, 500000000 };
    while ((i=fork()) < 0) nanosleep(&ts, 0);
  } else i=-1; /* don't fork */

  if (i <= 0) {
#ifdef SERDO_WANT_environ_in_command
    for (; argv[1]; put__env(*argv++)) {
      s0 = argv[0];
      if (!s0[str_chr(s0,'=')]) break;
    }
#endif
    pathexec_run(*argv,argv,environ);
    pr_fail(argv);
    if (!i) _exit(-1); /* child */
    return (ignore) ? 0 : -1;
  }

  if (waitpid(i,&i,0)==-1) diesys(1,"waitpid failed");
  if (ignore) return 0;
  if (!WIFEXITED(i)) return -1;
  return WEXITSTATUS(i);
}

static void get_word(struct cmd *c) {
  char ch, *x, *s = c->pos;

  while (is_space(s) || (*s=='\\' && s[1]=='\n')) ++s;
  if (!*s) { c->eof |= 1; return; }
  c->last=0;
  c->x=x=s;
  if (*x == '#') c->last |= 2;

  while ((ch=*s)) {
    if (ch==DQUOTE || ch==SQUOTE) {
      while (*++s) {		/* unterminated quoted string <==> *s == 0 */
	if (*s != ch) *x++ = *s;
	else {
	  if (s[-1] == '\\') x[-1] = ch;
	  else { s++; break; }
	}
      }
      if (!*s) die(2, "syntax error: unexpected EOF");
    }
    else if (ch==' ' || ch=='\n' || ch=='\t') break;
    else if (ch=='\\' && s[1]=='\n') { s += 2; continue; }
    else if (ch=='#') { for (; *s && *s != '\n';) s++; break; }
    else *x++ = *s++;
  }
  while (*s==' ' || *s=='\t') ++s;
  if (*s=='\n') { ++s; c->last |= 1; }

  c->pos = s;
  if (!*s) c->last |= 1;
  *x = 0;
}

static char *get_argv0(struct cmd *c) {
  while (1) {
    get_word(c);
    if (c->eof) break;
    if (c->last & 2) continue;
    return c->x;
  }
  return 0;
}

static int get_argv(struct cmd *c) {
  unsigned int k=0, len=16;
  char **argv, first=0;

  argv = alloca((len+2)*sizeof(char*));
  do {
    if (first) get_word(c);
    else first |= 1;
    if (c->eof) break;
    if (c->last < 2) { /* comment */
      if (k >= len) {
	char **tmp = argv;
	len *= 2;
	argv = alloca((len+2) * sizeof(char*));
	byte_copy(argv, k*sizeof(char*), tmp);
      }
      argv[k++] = c->x;
    }
  } while (!c->last);
  argv[k] = 0;

  if (!get_argv0(c)) --last_cmd;
  return spawn(argv);
}

static int execute(struct cmd *c) {
  int r = 0;
  if (get_argv0(c)) {
    ++last_cmd;

    while (1) {
      r = get_argv(c);
      if (r!=0 && !continueonerror) break;
      if (c->eof) break;
    }
  }
  return r;
}

static int batch(char *s) {
  struct cmd c;
  int len, fd=open(s,O_RDONLY);
  if (fd==-1 || GLOBAL_READ(fd,c.pos, len,32768)) {
    carpsys("could not open ",s);
    return 1;
  }
  close(fd);
  c.pos[len] = 0;
  c.eof = 0;
  return execute(&c);
}

int main(int argc,char* argv[],char* env[]) {
  if (argc<2) {
#ifdef SERDO_EXEC_script
    if (!access("script",O_RDONLY)) *argv-- = "script"; else
#endif
      { ops: die(1,"usage: serdo [-c] file"); }
  }

  if (str_equal(argv[1],"-c")) {
    ++continueonerror;
    ++argv;
  }
  if (*++argv == 0) goto ops;
  errmsg_iam("serdo");

  for (env_free=0; env[env_free];) ++env_free;
  ++env_free;
  environ = alloca((env_free + SERDO_MAX_NEW_ENVIRON) * sizeof(char *));
  byte_copy(environ, env_free * sizeof(char *), env);
  env_free = SERDO_MAX_NEW_ENVIRON;

  return batch(*argv);
}
