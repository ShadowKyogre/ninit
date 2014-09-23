#include <signal.h>
extern void sighandler(int sig);

void set_sigaction(int sig) /*EXTRACT_INCL*/{
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler=sighandler; 
  sa.sa_flags=SA_RESTART | SA_NOCLDSTOP;
  sigaction(sig, &sa, 0);
}
