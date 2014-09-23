#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "ninitfeatures.h"
#include "initreq.h"
#include "djb/buffer.h"

#define out(...) err_b(buffer_out,__VA_ARGS__,(char*)0);
#define outs(X) buffer_puts(buffer_out, X)
#define outc(X) buffer_putc(buffer_out, X)

char buffer_out_space[4096];
buffer b_out = BUFFER_INIT(write, -1, buffer_out_space, 4096);

buffer *buffer_out = &b_out;

struct ientry {
  char *id;
  char *level;
  char *action;
  char **argv;
};

char *home, *initscript=0, *app_f=0;
int fd;

void step(char *s)
  { out("test -d ",s," || mkdir ",s," ; cd ",s," || ops ",s); } 

void out_octal(unsigned char x) {
  outc('\\'); outc('0' + (x>>6)); 
  outc('0' + ((x>>3)&7)); outc('0' + (x&7));
}

void out_txt(char *s) {
  unsigned char x;
  while ((x=*s++)) {
    if (('a' <= x && x<= 'z') ||
	('A' <= x && x<= 'Z') || x==' ' || x=='/') outc(x);
    else out_octal(x);
  }
}

void append(char *m) {
  outs("printf \"");
  out_txt(m);
  outs("\\012\"");
  if (app_f)
    { out(" >> ",app_f); } 
}

void rm_all() { out("rm_old"); }

void stop_respawn() {
  app_f = "setup";
  append("#!/bin/sh\n"
	 "all=`/bin/nsvc -L`\n"
	 "/bin/nsvc -C0 $all\n"
	 "exec /bin/nsvc -r $all");
  out("chmod 755 setup ; rm -f sync");
}

void mk_run(char **xx) {
  out("ln -s ",*xx++," run");
  app_f = 0;
  if (*xx==0) return;
  outs("printf \"");
  while (*xx) { out_txt(*xx++); out_txt("\n"); }
  outs("\" > params\n");
}

void check_file(char *mode, char *file) 
{  out("test ",mode," ",file," || echo '*** Missing file:' ",home,"/",file);}

void check_utmp(char *utmp_file) {
  out("if test ! -f ",utmp_file," ; then");
  out(" echo '*** WARNING ***  Missing file:' ", utmp_file);
  out(" echo 'ninit stores in last file the variable INIT_RUNLEVEL'");
  out(" echo 'ninit does not use this variable.  Some other programs'");
  out(" echo 'can use it (scripts from /etc/init.d/).  If you have'");
  out(" echo 'some troubles, create this file latter!'");
  out("fi\n");
}

void die_mem() { die(111,"Out of memory"); }

int main(int Argc, char **Argv) {
  static char *initdefault;
  int seen_pf=-1, seen_pw=-1, seen_pn=-1, seen_po=-1;
  int seen_cad=-1, seen_kbr=-1;
  int seen_l0=-1, seen_l6=-1;
  char *p,*s,*service,**argv;
  int len,argc,k,j;
  struct ientry *qq,ee;
 
  s = Argv[1];
  if (s && s[0]=='-' && s[1]=='I' && s[2]) {
    initscript = s+2;
    Argv++; Argc--;
  }

  if (Argc<4 || Argv[2][0] == 0) 
    { die(100,"usage: ninit-inittab [-I/etc/initscript] /etc/inittab "INITROOT" output_file"); }
  home = Argv[2];

  if ((fd = open(Argv[1], O_RDONLY)) <0)
    { die(100, "error opening: ",Argv[1]); }

  if (GLOBAL_READ(fd,s, len,128000))
    { die(1, "error reading: ", Argv[1]); }
  close(fd);
  s[len]=0;

  if ((fd = open(Argv[3], O_CREAT|O_TRUNC|O_WRONLY, 0755)) <0)
    { die(100, "error opening: ",Argv[3]); }
  buffer_out->fd = fd;
      
  argc = splitmem(0,s,'\n');
  argv=alloca((len+1) * sizeof(char*));	if (argv==0) die_mem();
  splitmem(argv,s,'\n');

  for (len=0,k=0; argv[k]; k++) {
    s=argv[k];
    if (*s=='#' || *s==0) continue;
    ++len;
  }

  qq = alloca((len+2)*sizeof(struct ientry));	if (qq==0) die_mem();

  msg("\tvalid ",Argv[1]," entries");
  for (len=0,k=0; argv[k]; k++) {
    char **zz;
    s=argv[k];
    if (*s == '#' || *s==0) continue;
    byte_zero(&ee, sizeof(struct ientry));

    ee.id = s;
    s += str_chr(s,':'); if (*s==0) continue; *s++ = 0;
    ee.level = s;
    s += str_chr(s,':'); if (*s==0) continue; *s++ = 0;
    ee.action = s;
    s += str_chr(s,':'); if (*s==0) continue; *s++ = 0;

    msg(ee.id,":",ee.level,":",ee.action,":",s);
    if (!str_diff(ee.action, "initdefault"))
      initdefault = ee.level;
    if (*s==0) continue;

    zz = alloca(40 * sizeof(char*));	if (zz==0) die_mem();
    zz += 40;
    *--zz = 0;

    /* See if there is an "initscript" (except in single user mode). */
    if (initscript && str_diff("S",ee.level)) {
      /* Build command line using "initscript" */
      *--zz = s;
      *--zz = ee.action;
      *--zz = ee.level;
      *--zz = ee.id;
      *--zz = initscript;
      *--zz = "/bin/sh";
      ee.argv = zz;
    } else if (strpbrk(s, "~`!$^&*()=|\\{}[];\"'<>?")) {
      /* See if we need to fire off a shell for this command */
      *--zz = s;
      *--zz = "-c";
      *--zz = "/bin/sh";
      ee.argv = zz;
    } else {
      /* Split up command line arguments */
      int f;
      zz -= 22;
      ee.argv = zz;
      for (f = 0; f < 20; f++) {
	while(*s == ' ' || *s == '\t') s++;
	zz[f] = s;
	
	if (*s == 0) break;
	while (*s && *s != ' ' && *s != '\t' && *s != '#') s++;
	
	if (*s == '#' || *s == 0) {
	  f++;
	  *s = 0;
	  break;
	}
	*s++ = 0;
      }
      zz[f] = 0;
    }

    byte_copy(&qq[len], sizeof(ee), &ee);
    ++len;
  }
  for (k=0; k<len; k++)
    for (j=k+1; j<len; j++)
      if (!str_diff(qq[k].id, qq[j].id)) {
	msg("  \a*** WARNING *** ", Argv[1],": duplicated id: ",qq[k].id);
      }
  
  if (initdefault==0) { die(100, "unable to find initdefault level"); }

  /* -------------- change bellow this line ---------------- */ 
  out("#!/bin/sh\n"
      "# WARNING: This file was auto-generated.\n"
      "# Source: ",Argv[1],
      "\n\nexport PATH=/bin:/usr/bin\n"
      "umask 022\n"
      "ops () { echo cd error $1; exit 1; }\n"
      "rm_old () { rm -f run sync setup params respawn; }\n");

  check_utmp("/var/run/utmp");

  step(home);
  check_file("-d","sys");
  check_file("-x","sys/run");
  check_file("-x","sys/run-wait");
  check_file("-x","sys/update");
  check_file("-x","sys/procfs");
  check_file("-x","sys/remove");
  check_file("-p","in");
  check_file("-p","out");

  out("\nif test -f .services_ready; then\n"
      " echo Please read ",home,"/.services_ready\n"
      " exit 1\nfi\n"); 

  out("test -d default || mkdir default\n"
      "test -f default/depends && "
      "mv -f default/depends default/depends.old\n"
      "rm -f default/depends\n");

  step("sysvinit");
  rm_all();
  out("ln -sf /sbin/ninit-sysvinit run");
  app_f = "sysvinit-timeout";
  append("90:0\n"
	 "DO NOT remove or rename this file\n"
	 "unless you know what are you doing!");
  out("cd ..\n");

  step("powerS");
  rm_all();
  out("ln -sf /sbin/ninit-sysvinit run\n"
      "echo powerS > params");
  out("cd ..\n");

  step("update");
  rm_all();
  out("ln -sf /sbin/ninit-reload run");
  app_f = "params";
  append("-v\n-Rupdate\n-a30\n-u\n/sbin/ninit");
  out("cd ..\n",
      "ln -sf update level","U\n",
      "ln -sf update level","Q\n");

  step("ngetty");
  rm_all();
  out("ln -s /sbin/ngetty run");
  app_f = "params";
  append("1\n2\n3\n4\n5\n6");
  out("> respawn");
  app_f = "environ";
  append("\nTERM=linux");
  out("cd ..\n");

  
  for (k=0; k<len; k++) {
    char **xx;
    byte_copy(&ee, sizeof(ee), &qq[k]);
    xx=ee.argv;
    if (!str_diff(ee.action, "off")) {
      msg("\tskipping entry\t", ee.id,":",ee.level,":",ee.action,": ...");
      continue;
    }

    service = alloca(4+str_len(ee.id)); if (service==0) die_mem();
    service[0]='_';
    str_copy(service+1,ee.id);
    
    if (!str_diff(ee.action, "ctrlaltdel")) { seen_cad = k; }
    else if (!str_diff(ee.action, "kbrequest"))  { seen_kbr = k; }
    else if (!str_diff(ee.action, "sysinit") ||
	     !str_diff(ee.action, "boot") ||
	     !str_diff(ee.action, "bootwait") ||
	     !str_diff(ee.action, "once") ||
	     !str_diff(ee.action, "powerfail") ||
	     !str_diff(ee.action, "powerwait") ||
	     !str_diff(ee.action, "powerfailnow") ||
	     !str_diff(ee.action, "powerokwait")) {
      step(service);
      rm_all();
      if (ee.action[0] == 's' || ee.action[4] == 'w') out("> sync");
      mk_run(xx);						  
      out("cd ..\n");
      if (ee.action[0] != 'p') {
	app_f = "default/depends\n";
	append(service);
      }
      if (!str_diff(ee.action, "powerfail")) { seen_pf = k; }
      if (!str_diff(ee.action, "powerwait")) { seen_pw = k;}
      if (!str_diff(ee.action, "powerfailnow")) { seen_pn = k; }
      if (!str_diff(ee.action, "powerokwait")) { seen_po = k; }
    } 
    else {
      step(service); 
      rm_all();
      if (!str_diff(ee.action, "wait")) {
	p = ee.level;
	out("> sync");	  
	if (p[0] && p[1]==0) {
	  if (*p=='0') { seen_l0=k; stop_respawn(); }
	  if (*p=='6') { seen_l6=k; stop_respawn(); }
	  *--xx = p;
	  *--xx = "/sbin/ninit-runlevel";
	}
      }
      if (!str_diff(ee.action,"respawn")) {
	out("> respawn");
	if (strstr(ee.argv[0],"getty")) {
	  *--xx = ee.id;
	  *--xx = "/sbin/pututmpid";
	}
      }
      mk_run(xx);
      out("cd ..\n");
      
      if (!str_diff(ee.action, "wait")) {
	p = ee.level;
	if (p[0] && p[1]==0) out("ln -s ",service," level",p, "\n");
      }

      s = ee.level;
      s += str_chr(s, *initdefault);
      if (*s) {
	app_f = "default/depends\n";
	append(service);
      }
    }
  }

  s = "ln -sf _";
  if (seen_kbr>=0 && seen_l6>=0) out(s,qq[seen_l6].id," kbreq");
  if (seen_cad>=0 && seen_l0>=0) out(s,qq[seen_l0].id," ctrlaltdel");

  if (seen_l0>=0) out(s,qq[seen_l0].id," halt");
  if (seen_l6>=0) out(s,qq[seen_l6].id," reboot");

  /* XXX powerfail or powerwait ??? ; first may be ;-) */
  if (seen_pf>=0 && seen_pf < seen_pw) out(s,qq[seen_pf].id," powerF");
  if (seen_pw>=0 && seen_pw < seen_pf) out(s,qq[seen_pw].id," powerF");

  if (seen_pn>=0) out(s,qq[seen_pn].id," powerL");
  if (seen_po>=0) out(s,qq[seen_po].id," powerO");
  out("");

  out("test -d _l0 && echo INIT_HALT=POWERDOWN > _l0/environ\n"
      "if test -d _l6 ; then");
  app_f = "_l6/environ";
  append("#INIT_HALT=POWERDOWN\n"
	 "#INIT_HALT=HALT\n"
	 "INIT_HALT");
  out("fi\n");

  app_f = ".services_ready";
  append("*** WARNING ***\n"
	 "Someone made services in this directory.\n"
	 "If you want to overwrite them remove me.\n");
  
  buffer_flush(buffer_out);

  close(fd);
  return 0;
}
