#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <sys/reboot.h>
#include "../ninitfeatures.h"
#include "../uid.h"

#define TM_OUT 1200

int main(int argc, char **argv) {
  unsigned long ul;
  char *stable, *test;

  if (argc < 4) {
    carp("usage: conditional-init now /stable/init /test/init [args...]\n"
	 "\tnow must be the output of: date +%s");
    return 1;
  }

  errmsg_argv0 = "conditional-init";
  stable=argv[2];
  test=argv[3];
  
  if (scan_ulong(argv[1], &ul)==0 || ul + TM_OUT < (unsigned long)time(0)) {
    carp("booting (stable): ", stable);
    argv += 3;
    argv[0] = stable;
    execve(stable, argv, environ);
    return 0;
  }

  if (fork()==0) {
    unsigned long deadline = (unsigned long)time(0) + TM_OUT;
    while ((unsigned long)time(0) < deadline ) nano_sleep(1,0);
    sync();
    set_reboot(RB_AUTOBOOT);
    return 2;
  }

  argv += 3;
  argv[0]=test;
  carp("booting (test): ", test);
  execve(test, argv, environ);
  nano_sleep(TM_OUT,0); /* only if test fails */
  set_reboot(RB_AUTOBOOT);
  return 1;
}
