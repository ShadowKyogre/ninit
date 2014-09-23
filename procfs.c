#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>
#include "ninitfeatures.h" 
#include "nsvc_buffer.h"

static buffer b = BUFFER_INIT(read, 0, 0, BUFFER_TMP_LEN);

static void print_procfs(char *pidnum) {
  char *x, *qq[8], *buf = b.x;
  int fd,k;
  unsigned char ch;

  if (chdir("/proc/") || chdir(pidnum)) return;
  
  INIT_ARGS4(qq, "cmdline","environ","status","statm");
  for (k=0; k<4; k++) {
    x = qq[k];

    fd = open(x, O_RDONLY);
    if (fd>=0) {
      b.fd = fd;
      fmt_color(x,'5',":");

      if (k==2) {
	int got=0, ok_line=0;
	char first[24];

	msg("\tcat /proc/", pidnum, "/status");
	while (buffer_getc(&b, (char *)&ch)>0) {
	  if (got<3) { first[got++]=ch; continue; }
	  if (ok_line) outc(ch);
	  if (ch=='\n') { got=0; ok_line=0; continue; }
	  
	  if (got>3) continue;
	  if (!str_diffn(first,"Uid",3) ||
	      !str_diffn(first,"Gid",3) ||
	      !str_diffn(first,"Gro",3) ||
	      !str_diffn(first,"Sta",3)) ok_line=1;
	  first[got]=ch;
	  first[++got]=0;
	  if (ok_line) outs(first);
	}
      } else {	/* k=0,1 */
	outc('\n');
	while (buffer_getc(&b, (char *)&ch) >0) {	  
	  switch (ch) {
	  case 0: msg("\260"); break;
	  case 1: ch=':';
	  default:
	    if (ch != '\n' && (ch < 32 || ch > 127)) {
	      x = fu(ch); *--x = '.';
	      fmt_color(x,'1',"");
	    }
	    else outc(ch);
	  }
	}
      }
      close(fd);
    }
  }

  INIT_ARGS3(qq, "exe","cwd","root");
  for (fd=0; fd<3; fd++) {
    x = qq[fd];
    k = readlink(x, buf, BUFFER_TMP_LEN);
    if (k>0 && k < BUFFER_TMP_LEN) {
      buf[k] = 0;
      fmt_color(x,'5',":\t");
      msg(buf);
    }
  }
}

int main(int argc, char **argv) {
  if (argc>1) {
    __b_1.x = alloca(BUFFER_1_LEN);
    b.x = alloca(BUFFER_TMP_LEN);

    print_procfs(argv[1]);
    buffer_flush(&__b_1);
  }
  return 0;
}
