/* use __errno_location instead of errno */
#define WANT_THREAD_SAFE

/* on i386, Linux has an alternate syscall method since 2002/12/16 */
/* on my Athlon XP, it is twice as fast, but it's only in kernel 2.5 */
/* 20040118: enabling this breaks User Mode Linux!  It's their fault. */
#define WANT_SYSENTER
