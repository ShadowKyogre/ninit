#include <time.h>
#include "ninitfeatures.h"

void cron(char **argv, unsigned long *ret) /*EXTRACT_INCL*/ {
  unsigned long a,b,n, cron=0, now=time(0);
  unsigned long u[3];
  int dummy;
  char *p;

  for (; *argv; argv++) {
    p = *argv;
    u[2] = 0;
    if (scan_ulongs(p,u,3, scan_sec,':',&dummy) < 2) continue;

    if ((a=u[0])==0 || a > (1<<29)) continue;
    n = now;
    b = u[1];
    
    if ((a % 604800) == 0) { b += 3*86400; n -= 3*86400; }
    b %= a;
    n = n + b - (n % a);
    while (n <= now) n += a;
    
    if (cron==0 || n < cron) { cron = n; ret[1] = u[2]; }
  }
  ret[0] = cron;
}
