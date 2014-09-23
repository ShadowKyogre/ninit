#define SIGACTION_FUNCTIONS
#include "../process_defs.h"
extern int rt_sigprocmask();

void system_child_block(int type) /*EXTRACT_INCL*/ {
  /* SIG_BLOCK, SIG_UNBLOCK */
  STATIC_SIGCHLD_MASK;
  rt_sigprocmask(type,&sigchld_mask,0,sizeof(sigchld_mask));
}
