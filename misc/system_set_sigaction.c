#define SIGACTION_FUNCTIONS
#include "../process_defs.h"

typedef void (*sighandler_t)(int);
typedef struct { unsigned long sig[SIGSET_MASK_LEN_N]; } sigset_t;

/* copy/paste from dietlibc!  Felix, you are great! */
struct sigaction {
#if defined(__alpha__) || defined(__ia64__) || defined(__hppa__)
  sighandler_t sa_handler;
  unsigned long sa_flags;
  sigset_t sa_mask;
#elif defined(__mips__)
  unsigned long sa_flags;
  sighandler_t sa_handler;
  sigset_t sa_mask;
  void (*sa_restorer)(void);
  int32_t sa_resv[1];
#else	/* arm, i386, ppc, s390, sparc, saprc64, x86_64 */
  sighandler_t sa_handler;
  unsigned long sa_flags;
  void (*sa_restorer)(void);
  sigset_t sa_mask;
#endif
};

extern void sighandler(int sig);
extern void __restore();
extern void __restore_rt();
extern int rt_sigaction();
#include "../byte_defs.h"

void system_set_sigaction(int sig) /*EXTRACT_INCL*/ {
  struct sigaction sa;
  byte_zero(&sa, sizeof(sa));
  sa.sa_handler=sighandler;
  sa.sa_flags=SA_FLAGS_number;
#ifndef INIT_SKIP_SIGRETURN  
  sa.sa_restorer=&(SA_RESTORER_function); 
#endif
  rt_sigaction(sig, &sa, 0, sizeof(sigset_t));
}
