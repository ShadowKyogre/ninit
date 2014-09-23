/* again DJB; see http://cr.yp.to */
#include <unistd.h>
#include <errno.h>
#include <alloca.h>
#include <stdlib.h>

#include "../ninitfeatures.h"

void pathexec_run(char *file,char **argv,char **envp) /*EXTRACT_INCL*/ {
  char *path,*tmp;
  int savederrno=0;
  unsigned int len, next, file_len;

  if (file[str_chr(file,'/')]) { execve(file,argv,envp); return; }
  file_len = str_len(file) + 1;  

  path = env_get("PATH");
  if (!path) path = "/bin:/usr/bin";
  tmp = alloca(4 + str_len(path) + file_len);
  if (!tmp) { errno=ENOMEM; return; }

  for (;;) {
    next = str_chr(path,':');
    len = next;

    if (next==0) { tmp[0] = '.'; len = 1; }
    else { byte_copy(tmp,next,path); }
    tmp[len] = '/';
    byte_copy(tmp+len+1, file_len, file);

    execve(tmp,argv,envp);
    if (errno != ENOENT) {
      savederrno = errno;
      if ((errno != EACCES) && (errno != ENOEXEC) && (errno != ENOTDIR)) return;
    }

    if (path[next]==0) {
      if (savederrno) errno = savederrno;
      return;
    }
    path += (next+1);
  }
}
