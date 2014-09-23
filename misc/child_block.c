#include <sys/types.h>
#include <signal.h>

void child_block(int type) /*EXTRACT_INCL*/{	/* SIG_BLOCK, SIG_UNBLOCK */
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(type, &mask, 0);
}
