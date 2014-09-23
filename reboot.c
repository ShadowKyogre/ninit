#include <unistd.h>
#include <sys/reboot.h>
#include "ninitfeatures.h"
#include "uid.h"

#define USAGE "ninit-reboot: Aborted.\nSay \"ninit-reboot (RESTART|ENABLE_CAD|DISABLE_CAD|HALT|POWER_OFF)\" if you really mean it.\n"

void usage(void) {
  write(2, USAGE, str_len(USAGE));
  _exit(1);
}

int main(int argc, char *argv[]) {
  unsigned int m=0;
  if (argc!=2)
    usage();

  sync();
  sync();
  sync();
  nano_sleep(1, 0);

  if      (!str_diff(argv[1], "RESTART"))      m=RB_AUTOBOOT;
  else if (!str_diff(argv[1], "HALT"))         m=RB_HALT_SYSTEM;
  else if (!str_diff(argv[1], "ENABLE_CAD"))   m=RB_ENABLE_CAD;
  else if (!str_diff(argv[1], "DISABLE_CAD"))  m=RB_DISABLE_CAD;
  else if (!str_diff(argv[1], "POWER_OFF"))    m=RB_POWER_OFF;
  else usage();

  set_reboot(m);
  set_reboot(RB_HALT_SYSTEM);

  close(0);  
  close(1); 
  close(2);

  while(1) nano_sleep(10,0);
}
