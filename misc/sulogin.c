#include <unistd.h>
#include <stdlib.h>
extern char **environ;

void sulogin() /*EXTRACT_INCL*/ {
  char *argv[2] = {"sulogin",0};
  execve("/sbin/sulogin",argv,environ);
  _exit(1);
}
