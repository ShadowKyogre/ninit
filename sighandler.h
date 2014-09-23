static volatile char got_sig[4];

void sighandler(int sig) {
  if (sig == SIGCHLD)	return;
#ifdef NINIT_SIGNAL_HANDLER_CODED
  {
    unsigned char *Sig = (unsigned char *)NINIT_SIGNAL_HANDLER_CODED;
    int k;
    for (k=0; k<4; k++)
      if ((unsigned char)sig == Sig[k])
	{ got_sig[k] = 1; return; }
  }
#else

  if (sig == SIGWINCH)  got_sig[0]=1;
  if (sig == SIGINT)    got_sig[1]=1;
#ifdef INIT_SYSVINIT_SIGNALS
  if (sig == SIGPWR)    got_sig[2]=1;
  if (sig == SIGHUP)    got_sig[3]=1;
#endif
#endif /* NINIT_SIGNAL_HANDLER_CODED */

#ifdef INIT_SYSVINIT_SIGNALS
  if (sig == SIGUSR1)   { close(initctl); initctl = -5; }
#endif
}
