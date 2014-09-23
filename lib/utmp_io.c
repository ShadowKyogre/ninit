#include <unistd.h>
#include <fcntl.h>
#include <utmp.h>

#define UTMP_SIZE (sizeof(struct utmp))

/* type:  F_RDLCK or F_WRLCK */
struct utmp *utmp_io(int fd, struct utmp *ut, int type) /*EXTRACT_INCL*/ {
  int ret;
  struct flock fl;
  int (*op)() = (type==F_WRLCK) ? (int(*)())write : (int(*)())read;

  fl.l_whence	= SEEK_CUR;
  fl.l_start	= 0;
  fl.l_len	= UTMP_SIZE;
  fl.l_pid	= 0;
  fl.l_type	= type;
  
  if (fcntl(fd, F_SETLKW, &fl)) return 0;
  ret = op(fd, ut, UTMP_SIZE);

  fl.l_start	= -UTMP_SIZE;
  fl.l_type	= F_UNLCK;

  fcntl(fd, F_SETLK, &fl);

  if (ret != UTMP_SIZE) return 0;
  return ut;
}
