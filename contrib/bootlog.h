/* return -1 on error */
static int xx_write(int fd, void *buf, size_t len) {
  char *x = buf;
  ssize_t w;
  while (len) {
    w = write(fd, buf, len);
    if (w <= 0) {
      if (w < 0 && errno == EINTR) continue;
      return -1;
    }
    x += w;
    len -= w;
  }
  return 0;
}

/* return 0 if closed or error, -1 temporary error */
static int do_io(void *buf, int len) {
  int r = read(0,buf,len);
  if (r<0)
    if (errno != EINTR) r = 0;
  if (r>0) xx_write(1,buf,r);
  return r;
}

static int mk_backup() {
  if (flag_rename) {
    if (rename(name, flag_rename) && errno != ENOENT) return -1;
    flag_rename = 0;
  }
  return 0;
}

static void write2(char *s) { write(2,s,str_len(s)); }

int main(int argc, char **argv) {
  unsigned long len=0;
  int pid, pi[2];
  
  for (;;) {
    char *p;
    argc--;
    argv++;
    if ((p=argv[0]) == 0 || *p != '-') break;
    while (*++p)
      switch (*p) {
      case 'a': m |= O_APPEND; break;
      case 't': m |= O_TRUNC; break;
      case 'c': m |= O_CREAT; break;
      case 'r': ++flag_rename; break;
      case '2': ++flagstderr; break;
      case '1': ++flagstdout; break;
      default:
	goto usage;
      }
  }

  if (argc<3) {
  usage:
    write2("usage: bootlog [-12ctar] size logfile program args...\n");
    _exit(1);
  }
  if (scan_ulong(argv[0], &len) == 0) goto usage;
  if ((flagstderr | flagstdout) == 0) {
    ++flagstdout;
    ++flagstderr;
  }
  name=argv[1];

  for (pid=0; pid<3; pid++)
    if (fcntl(pid,F_GETFL,0) == -1) goto do_it;

  if (pipe(pi)) goto do_it;
  while ((pid=fork()) < 0);

  if (pid==0) {
    close(pi[1]);
    while ((pid=fork()) < 0);
    if (pid==0) {
      dup2(pi[0],0);
      close(pi[0]);
      loop(len);
    }
    _exit(0);
  }  else {
    close(pi[0]);
    waitpid(pid, 0, 0);
    if (flagstdout) { dup2(pi[1],1); }
    if (flagstderr) { dup2(pi[1],2); }
    close(pi[1]);
  }

 do_it:
  argv += 2;
  pathexec_run(argv[0], argv, environ);
  write2("bootlog: ");
  write2(argv[0]);
  write2(": exec error\n");
  _exit(127);
}
