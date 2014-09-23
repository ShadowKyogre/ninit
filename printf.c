#include <unistd.h>
#include <errno.h>

struct mini_buffer { 
  char *x;
  unsigned int p, n;
  int fd;
};

static char buf_space[512];
struct mini_buffer b = { buf_space, 0, sizeof(buf_space), 1 };

static void s_write(int fd, char *s, int n) {
  int w;
  while (n) {
    w = write(fd,s,n);  
    if (w==0) _exit(1);
    if (w==-1) {
      if (errno == EINTR) continue;
      _exit(1);
    }
    s += w;
    n -= w;
  }
}

void out_flush() { s_write(b.fd, b.x, b.p); b.p=0; }
void out_char(char ch) { if (b.p==b.n) out_flush(); b.x[b.p] = ch; b.p++; }
void outs(char *s) { while (*s) { out_char(*s); ++s; } }

static void bad(char *fmt, char *s) {
  if (b.fd==1) out_flush(); 
  b.fd=2;
  outs("\nbad format: "); outs(fmt); outs("\nstart: ");
  outs(s); outs("\n"); out_flush(); _exit(2);
}

char *conv_esc(char *x, char *pr_char) {
  char *e0 = "ntrabfvEe\\";
  char *e1 = "\n\t\r\a\b\f\v\033\033\\";
  char ch;
  int k;
  for (k=0; k<10; k++)
    if (e0[k] == *x)
      { *pr_char = e1[k]; return x; }
  
  for (k=0, ch=0; '0'<=*x && *x<'8' && k<3; k++, x++)
    ch = ch*8 + (*x-'0');
  if (k == 0) return 0;
  *pr_char = ch;
  return --x;
}

int main(int argc, char **argv) {
  char *fmt, *s, *x, ch;
  if (argc<2) _exit(100);
  fmt = argv[1];
  argv += 2;

  for (s=fmt; (ch=*s); s++) {
    if (ch != '%' && ch != '\\') goto print_char;
    if ((ch = *++s) == 0) goto bad_fmt;

    if (s[-1] == '\\') {
      if ((x=conv_esc(s,&ch)) == 0) goto bad_fmt;
      s = x;
    print_char:
      out_char(ch);
    } else {	/* % */
      switch (ch) {
      case '%':	goto print_char;
      case 's':
      case 'b': 
	if ((x=*argv++)) { 
	  if (ch=='s') outs(x);
	  else {	/* %b */
	    char *y, *tmp;
	    for (tmp=x; (ch=*x); x++) {
	      if (ch == '\\') {
		if ((y=conv_esc(++x,&ch)) == 0) bad(tmp, x-1);
		x = y;
	      }
	      out_char(ch);
	    }
	  }
	  break;
	}
	out_flush(); b.fd=2;
	outs("\nmissing argument");
      default:
	bad_fmt:
	bad(fmt,s-1);
      }
    }
  }
  out_flush();
  fsync(1);
  _exit(close(1));
}
