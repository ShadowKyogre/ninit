#include <errno.h>

static struct error_table table[] = {
  {EACCES,	"Permission denied"},
  {EINVAL,	"Invalid argument"},
  {EIO,		"I/O error"},
  {EISDIR,	"Is a directory"},
  {ELOOP,	"Too many symbolic links"},
  {ENAMETOOLONG,	"File name too long"},
  {ENOENT,	"No such file or directory"},
  {ENOEXEC,	"Exec format error"},
  {ENOMEM,	"Out of memory"},
  {ENOSYS,	"Function not implemented"},
  {ENOTDIR,	"Not a directory"},
  {EROFS,	"Read-only file system"},
  {ETXTBSY,	"Text file busy"},
  {ESPIPE,	"Illegal seek"},
  {0,0}
};
