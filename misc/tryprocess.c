#include "../ninitfeatures.h"
#include <unistd.h>
#include <signal.h>
#include "try_helper.h"

#define ioctl libc_ioctl
#include <termios.h>
#undef ioctl
#include <sys/ioctl.h>

#define w_sn_(a,b)   w(a); wn(b); w("\n")
#define w_sp_(a,b,c) w(a); wn((void *)(&b) - (void *)(&c)); w("\n")
#define w_sns(a,b,c) w(a); wn(b); w(c)
#define w_ss_(a,b)   w(a);  w(b); w("\n")
#define w_o(X)       w(fmt_o((unsigned char)X))
#define w_to(t,X)    w(t); w_o(X)

#include "../struct_root.h"
INIT_ROOT_DEFINE(ch, char*);
INIT_ROOT_DEFINE(ui, unsigned int);

#define UL sizeof(unsigned long)
#define MASK_LEN  (_NSIG/(8 * UL))
#define WORD(X)   ((X-1)/(8*UL))
#define MASK(X)   (((unsigned long)1) << (X-1) % (8*UL))

int main() {
  int i,k;
  struct sigaction sa;
  unsigned long uu[256];
  char *ss[4] = { "kbreq", "ctrlaltdel", "powerS", "levelU" };

  w("#define NINIT_SERVICES_CODED\t\"");
  for (k=4, i=0; i<4; i++) {
    w_o(k);
    k += 1 + s_len(ss[i]);
  }
  for (i=0; i<4; i++) {
    w(ss[i]);
    if (i<3) w("\\000");
  }
  w("\"\n");

  if (SIGWINCH < 256 && SIGINT < 256 && SIGPWR < 256 && SIGHUP < 256) {
    w("#define NINIT_SIGNAL_HANDLER_CODED\t\"");
    w_o(SIGWINCH); w_o(SIGINT); w_o(SIGPWR); w_o(SIGHUP);
    w("\"\n");
  }

  if (SIGTERM < 256 && SIGALRM < 256 && SIGSTOP < 256 &&
      SIGCONT < 256 && SIGHUP  < 256 && SIGINT  < 256 && SIGKILL < 256) {
    w("#define NSVC_SIGNAL_CODED\t\"");
    w_to("t",SIGTERM); w_to("a",SIGALRM); w_to("p",SIGSTOP);
    w_to("c",SIGCONT); w_to("h",SIGHUP);  w_to("i",SIGINT);  
    w_to("k",SIGKILL);
    w("\"\n");
  }

  w_sn_("#if 0\n\tdev_t\t", sizeof(dev_t));
  w_sn_("\tgid_t\t", sizeof(gid_t));
  w_sn_("\tuid_t\t", sizeof(uid_t));
  w_sn_("\tmode_t\t", sizeof(mode_t));

  w_sn_("#endif\n#define X_TIOCGPGRP\t", TIOCGPGRP);
  w("\n#ifdef SIGACTION_FUNCTIONS\n\n");

#ifndef SA_RESTORER
#define SA_RESTORER   0x04000000
#endif

  w_sn_("#define SA_RESTART\t", SA_RESTART);
  w_sn_("#define SA_NOCLDSTOP\t", SA_NOCLDSTOP);

  uu[0] = SA_RESTART | SA_NOCLDSTOP | SA_RESTORER;
#ifdef INIT_SKIP_SIGRETURN
  w("#define INIT_SKIP_SIGRETURN\n");
  uu[0] = SA_RESTART | SA_NOCLDSTOP;
#endif
  w_sn_("#define SA_FLAGS_number\t", uu[0]);

#if defined(__i386__)  
  w("#define SA_RESTORER_function\t__restore");
  if (uu[0] & SA_SIGINFO) w("_rt");
#endif

#if defined(__x86_64__)
  w("#define SA_RESTORER_function\t__restore_rt");
#endif
  w("\n");

  w("\n#if 0\n");
  /*  w_sn_("#define STAT_SIZE\t", sizeof(struct stat)); */
  w_sn_("#define SIGATION_SIZE\t", sizeof(sa));
  w_sn_("#define SIGSET_T_SIZE\t", sizeof(sa.sa_mask));
  w_sn_("#define SIGSET_WORD_N\t", WORD(SIGCHLD));
  w_sn_("#define SIGSET_MASK_N\t", MASK(SIGCHLD));

  w_sp_("\tsa_handler offset\t", sa.sa_handler, sa);
  w_sp_("\tsa_flags offset\t",	 sa.sa_flags, sa);
  w_sp_("\tsa_restorer offest\t",sa.sa_restorer, sa);
  w_sp_("\tsa_mask offset\t",    sa.sa_mask, sa);
  w("#endif\n\n");

  for (i=0; i<256; i++) uu[i] = 0;
  uu[WORD(SIGCHLD)] = MASK(SIGCHLD);
  w_sn_("#define SIGSET_MASK_LEN_N\t", MASK_LEN);
  w_sns("#define STATIC_SIGCHLD_MASK unsigned long sigchld_mask[",
	MASK_LEN, "] = { ");

  for (i=0;; ) {
    wn(uu[i]); 
    if (++i ==  MASK_LEN) break;
    w(", ");
  }

  w(" }\n"
    "\n#else\n"
    "#include \"struct_root.h\"\n");

  if (sizeof(struct ch) > sizeof(struct ui)) {
    w("#define INIT_NAME_IS_UINT\n");
    w_sn_("#define PROCESS_SIZE ", sizeof(struct ui));
    w("INIT_ROOT_DEFINE(process, uint32t) *root;\n");
  } else {
    w("#undef INIT_NAME_IS_UINT\n");
    w_sn_("#define PROCESS_SIZE ", sizeof(struct ch));
    w("INIT_ROOT_DEFINE(process, char*) *root;\n");
  }

  w("#endif\n");
  return 0;
}
