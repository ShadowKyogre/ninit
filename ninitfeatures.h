/* bellow is the variable INIT_PRIVATE_SETUP
   Fill private changes there! */

#ifndef NINITFEATURES_H
#define NINITFEATURES_H

#define INITROOT		"/etc/ninit"
#define INITSYSDIR		"sys"

#define INIT_POWERSTATUS	"/etc/powerstatus"
#define SYSVINIT_VERSION	"2.86"

	/* comment next line if you don't have system flock */
#define HAVE__NR_flock

	/* checks the environ vars: NINIT_HOME and INIT_HOME.
	   default is only the first */
// #define INIT_TWO_ENV_HOME

	/* if your kernel has sigreturn function uncomment next line */
// #define INIT_SKIP_SIGRETURN


	/* ------ setup for serdo ----- */

	/* simple echo; don't support esacpes: \a \t ... */
#define SERDO_WANT_echo

	/* see manpage killall5(8) */
#define SERDO_WANT_killall5

	/* VAR1=val1 VAR2=val2 ... command ...;  see sh(1) */
#define SERDO_WANT_environ_in_command

	/* max total new variables for export */
#define SERDO_MAX_NEW_ENVIRON 128

	/* Next is a big security hole!  It's similar to dot in PATH 
	   Idea: some user type: echo /bin/rm -rf / > /tmp/script
	   and the sysadmin execs serdo in /tmp		*/
// #define SERDO_EXEC_script


	/* ------ setup for run ----- */

	/* default maximal timout for waiting a service to finish */
#define RUN_MAXWAIT 600

	/* if defined check gid file in the service directory */
// #define RUN_WANT_GID_FILE


	/* ------ setup for nsvc ----- */

	/* /etc/ninit/ops/// --> ops; only for nsvc		*/
#define CLEAN_SERVICE 


	/* ----- setup for ninit ----- */
#ifdef INIT_PROGRAM

	/* must be between 0 and 255.  length of history;	*/
#define INIT_HISTORY 31

	/* if a service has dir log it is startded first.
	   you can skip this and include all log in depends.	*/
#define INIT_LOG_SERVICE

	/* ninit automatic creates in/out named pipes for log-services */
#define INIT_CREATE_INOUT_PIPE

	/* next tree lines are debug warnings.  
	   you can comment them if ninit works stable. */
#define INIT_PIPES_ERROR

	/* warning if poll filed; never happen */
#define INIT_POLL_FAILED

	/* say that all services are exited.  more simple: nsvc -L */
#define INIT_EXIT_WARNING

	/* ninit will use different service dir with option -H.
	   example: /sbin/ninit -H/etc/ninit.test
	   The {in,out} pipes must be links to /etc/ninit/{in,out}.
	   This is because they are coded in nsvc and ninit-reload.	*/
#define INIT_HOME

	/* sigaction on SIGUSR1, SIGPWR, SIGHUP.  Comment next to uncatch
	   above signals.  Catching them makes ninit 68 bytes longer.	*/
#define INIT_SYSVINIT_SIGNALS

	/* skip services with names '-blabla' or '#blabla' on startup	*/ 
// #define INIT_BLACK

	/* unblock/block SIGCHLD near to main poll loop			*/
// #define INIT_CHILD_BLOCK

	/* if you use ninit with -Mn and n>3500 uncomment next line.
	   see total/used memory with:
	   ninit-reload -m | wc -c
	   nsvc -D		*/
// #define INIT_TIMEOUT_WRITE

	/* uses mmap for depends instead of alloca.  If your depeneds
	   files are too large try this ;-)	*/
// #define INIT_MMAP
#endif


/* one service (i386) uses 21 + strlen(service) bytes RAM */
#ifndef INIT_ALLOC_BUFFER
#ifdef  INIT_PROGRAM
#ifndef INIT_MMAP
#define INIT_ALLOC_BUFFER 1536
#else
#define INIT_ALLOC_BUFFER 2800
#endif
#endif

#ifdef SVC_PROGRAM
#define INIT_ALLOC_BUFFER 3840
#endif
#endif

#ifndef INIT_ALLOC_BUFFER
#define INIT_ALLOC_BUFFER 8192
#endif

// #define INIT_PRIVATE_SETUP

#ifdef INIT_PRIVATE_SETUP
// #undef INIT_HOME
#undef INIT_BLACK
#undef INIT_SYSVINIT_SIGNALS

#undef INIT_PIPES_ERROR
#undef INIT_POLL_FAILED
#undef INIT_EXIT_WARNING

#undef INIT_LOG_SERVICE 
#undef INIT_CREATE_INOUT_PIPE
// #define SYSVINIT_DUMP_INITREQ
#endif

/* -------- stop changes here ;-) ---------- */
#ifdef INIT_PROGRAM
#ifndef INIT_HISTORY
#define INIT_HISTORY 0
#endif

#ifndef INIT_LOG_SERVICE
#undef INIT_CREATE_INOUT_PIPE
#endif
#endif

#ifndef INIT_TIMEOUT_WRITE
#if (INIT_ALLOC_BUFFER + 0 > 3600)
#define INIT_TIMEOUT_WRITE
#endif
#endif

#include <sys/types.h>
#include "all_defs.h"

#define GLOBAL_fstat_READ(fd,st,s, len,max_len) \
   system_fstat(fd,&st) || (unsigned int)(len=st.st_size) > max_len || \
   (s=alloca(len+1))==0 || read(fd,s,len) != len

#define GLOBAL_READ(fd,s, len,max_len) \
   (len=lseek(fd,0,SEEK_END)) < 0 || \
   len > max_len || lseek(fd,0,SEEK_SET) || \
   (s=alloca(len+1)) == 0 || read(fd,s,len) != len

extern char **environ;
extern const char *errmsg_argv0;

#define INIT_ARGS1(Z,a) Z[0]=a
#define INIT_ARGS2(Z,a,b) do { Z[0]=a; Z[1]=b; } while(0)
#define INIT_ARGS3(Z,a,b,c) do { Z[0]=a; Z[1]=b; Z[2]=c; } while(0)
#define INIT_ARGS4(Z,a,b,c,d) do { Z[0]=a; Z[1]=b; Z[2]=c; Z[3]=d; } while(0)
#define INIT_ARGS5(Z,a,b,c,d,e) do { Z[0]=a; Z[1]=b; Z[2]=c; Z[3]=d; Z[4]=e; } while(0)
#define INIT_ARGS6(Z,a,b,c,d,e,f) do { Z[0]=a; Z[1]=b; Z[2]=c; Z[3]=d; Z[4]=e; Z[5]=f; } while(0)
#define INIT_ARGS7(Z,a,b,c,d,e,f,g) do { Z[0]=a; Z[1]=b; Z[2]=c; Z[3]=d; Z[4]=e; Z[5]=f; Z[6]=g; } while(0)
#define INIT_ARGS8(Z,a,b,c,d,e,f,g,h) do { Z[0]=a; Z[1]=b; Z[2]=c; Z[3]=d; Z[4]=e; Z[5]=f; Z[6]=g; Z[7]=h; } while(0)

#define msg(...) err(1,__VA_ARGS__,(char*)0)
#define carp(...) err(2,__VA_ARGS__,(char*)0)
#define die(n,...) do { err(2,__VA_ARGS__,(char*)0); _exit(n); } while(0)
#define errmsg_iam(X) errmsg_argv0 = X

#ifndef INIT_TWO_ENV_HOME
#define INIT_ENV_GET_HOME(H,A,B) H=env_get(A)
#else
#define INIT_ENV_GET_HOME(H,A,B) H=env_get(A); if (!H) H=env_get(B)
#endif
#undef INIT_TWO_ENV_HOME

#define BUFFER_TMP_LEN	1500
#endif /* end of file */
