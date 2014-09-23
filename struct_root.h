/*
  if you have problems with time define 
  startedat and cron bellow to be time_t instead of int32t
*/

union pr_flags {
  struct {
    char b0 : 1;
    char b1 : 1;
    char b2 : 1;
  } b;
  char ch;
};

#define pr_respawn	pr_flags.b.b0
#define pr_end		pr_flags.b.b1
#define pr_finish	pr_flags.b.b2

#define INIT_ROOT_DEFINE(Process,Type) \
struct Process {\
  Type name;\
  pid_t pid;\
  union pr_flags pr_flags;\
  char pr_circular;\
  unsigned short father;\
  int32t startedat;\
  int32t cron;\
}

struct memalloc {
  void *x;
  uint32t l,r,a;
};
