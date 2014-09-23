#include <asm/unistd.h>

#ifndef __NR_time

#include <time.h>
#include <sys/time.h>

time_t time(time_t *foo) {
  struct timeval tv;
  gettimeofday(&tv,0);
  if (foo) *foo=tv.tv_sec;
  return tv.tv_sec;
}
#endif
