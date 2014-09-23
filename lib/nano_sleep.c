#include "../int_defs.h"
#include <time.h>
void nano_sleep(uint32t sec, uint32t nsec) /*EXTRACT_INCL*/ {
  struct timespec ts = { sec, nsec };
  nanosleep(&ts, 0);
}
