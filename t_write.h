/* timeout write to outfd more than 4096 bytes */

static void t_write(void *buf, int len) {
#ifndef INIT_TIMEOUT_WRITE
  write(outfd,buf,len);
#else
  time_t deadline = time(0) + 1;
  while (len > 0) {
    int w = write(outfd,buf, (len > PIPE_BUF) ? PIPE_BUF : len);
    if (w==-1) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN && time(0) <= deadline) continue;
      return;
    }
    len -= w;
    buf += w;
  }
#endif
}
