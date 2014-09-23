#include <unistd.h>
#include <fcntl.h>
#include <alloca.h>
#include <stdio.h>
#include <sys/stat.h>
#include "ninitfeatures.h"
#include "error_table.h"
#include "uid.h"

#define U "unable to "

static char *e() { return error_string(table, errno); }
static void nomem() { die(111,"out of memory"); }
static void ex(char *s0, char *s1) { die(111,U,s0,": ",s1,": ",e()); }

static void doit(char *to, char *line) {
  char *x, **arg, *target, *tmp;
  char *type, *mid, *name, *alias;
  unsigned long uid, gid, mode;
  int in, out, len;
  static char *uid_global, *gid_global, *mode_global, *mid_global, V;

  len=splitmem(0,line,':'); if (len <7) return;
  arg=alloca((len+1) * sizeof(char*)); if (arg==0) nomem();
  splitmem(arg,line,':');

  type	= arg[0]; 
  x=arg[1]; if (!*x==0 && uid_global) x=uid_global; 
  if (*x) uid=atoulong(x); else uid = -1;

  x=arg[2]; if (!*x==0 && gid_global) x=gid_global; 
  if (*x) gid=atoulong(x); else gid = -1;

  x=arg[3];	if (!*x && mode_global) x=mode_global; scan_8ulong(x,&mode);
  mid = arg[4];	if (!*mid && mid_global) mid=mid_global;
  name= arg[5]; 
  x=arg[6]; alias = (*x) ? x : name;

  len = str_len(to) + str_len(mid) + str_len(name);
  x=alloca(2*len + 32); if (x==0) nomem();
  target = x;
  x += str_copy(x,to);
  x += str_copy(x,mid);
  x += str_copy(x,name);
  while (target[0]=='/' && target[1]=='/') target++;
  tmp = (*type=='x') ? x+2 : 0;

  switch(*type) {
  case 'p':
    if (SYS_mknod(target,S_IFIFO|0600,0) == -1)
      if (errno != EEXIST) ex("mknod",target);
    if (V) msg("pipe:\t", target);
    break;

  case 'd':
    if (mkdir(target,0700) == -1)
      if (errno != EEXIST) ex("mkdir",target);
    if (V) msg("mkdir:\t", target);
    break;

  case 'c':
  case 'x':
    if ((in=open(alias, O_RDONLY)) <0) ex("open",alias);

    if (*type == 'c') out = open(target, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    else out = open_tmpfd(target, tmp, 0600);
    if (out <0) ex("open",target);

    x=alloca(8192); if (x==0) nomem();
    for (;;) {
      len=read(in,x,8192);
      if (len==0) break;
      else if (len==-1) ex("read",alias);
      else if (len != write(out,x,len)) { die(111,U,"write",target); }
    }

    close(in);
    if (fsync(out)) ex("fsync",target);
    if (close(out)) ex("close",target);
    if (tmp && rename(tmp,target))
      { die(111,U,"rename: ", tmp, " -> ", target, ": ", e()); }
    if (V) msg(alias, "\t-> ", target);
    break;

  case 'g':
    x=arg[1]; uid_global  = (*x) ? x : 0;
    x=arg[2]; gid_global  = (*x) ? x : 0;
    x=arg[3]; mode_global = (*x) ? x : 0;
    x=arg[4]; mid_global  = (*x) ? x : 0;
    return;

  case 'v':
    if (arg[1][0]) V=1;
    else V=0;

  default:
    return;
  }
  if (SYS_chown(target,uid,gid) <0) ex("chown",target);
  if (chmod(target,mode) <0)    ex("chmod",target);
}

int main(int argc, char **argv) {
  char *to, **arg, *s=0;
  if (argc<2) { die(100,"usage: install-bin Dir < File\n  "
    "install-bin Dir  c:::755:mid:file::  x:::755:mid:name:source:  ...\n"
    "File contains lines:\ntype:uid:gid:mode:middle:target:source:\n"
    "type is one of the letters: vpdcxg\n"
    "type g sets global uid:gid:mode:middle\n"); }

  to = argv[1];
  errmsg_iam("install-bin");
  umask(077);
  
  if (argc == 2) {
    int len;
    if (GLOBAL_READ(0,s, len,100000))  ex("read","stdin");
    close(0);
    s[len]=0;
    
    len = splitmem(0,s,'\n');
    arg = alloca((len+1) * sizeof(char*)); if (arg==0) nomem();
    splitmem(arg,s,'\n');
  } else
    arg = argv+2;

  for (; (s=*arg); ++arg) doit(to, s);
  return 0;
}
