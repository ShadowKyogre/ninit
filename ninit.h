#include "ninitfeatures.h"
#include "process_defs.h"

#ifndef INIT_NAME_IS_UINT
#define root_name(J)		root[J].name
#define process_name(P)		P->name
#define alloc_name_set(N)	N
#define process_name_set	name
#define root_names_len		(mem.x + mem.a - (void*)root[0].name)

#else
#define root_name(J)		(mem.x + root[J].name)
#define process_name(P)		(mem.x + P->name)
#define alloc_name_set(N)	((void*)N - mem.x) 
#define process_name_set	mem.r
#define root_names_len		(mem.a - root[0].name)
#endif

#ifdef  INIT_PROGRAM
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <linux/kd.h>
#include "initreq.h"
#endif

#if ((INIT_HISTORY<0) || (INIT_HISTORY>255))
#error ***** INIT_HISTORY must be between 0 and 255 *****
#endif

static int infd, outfd;
static int maxprocess=-1;
static struct memalloc mem;

#ifdef INIT_PROGRAM

#ifdef INIT_HOME
static char *initroot = INITROOT;
static char *initsysdir = INITSYSDIR;
#else
#define initroot INITROOT
#define initsysdir INITSYSDIR
#endif

#ifdef INIT_BLACK
static char** Argv;
#endif 

static char do_update;
static time_t next_cron;
static uint32t start_time;
static int initctl = -5;
static unsigned short *history;

#include "findservice.h"
#include "addprocess.h"
#include "t_write.h"
#include "mmap_alloca.h"
#include "sighandler.h"
#include "uid.h"

#ifndef INIT_CHILD_BLOCK
#define CHILD_BLOCK
#define CHILD_UNBLOCK
#else
#if INIT_SYSTEM
#define CHILD_BLOCK	system_child_block(SIG_BLOCK)
#define CHILD_UNBLOCK	system_child_block(SIG_UNBLOCK)
#else 
#define CHILD_BLOCK	child_block(SIG_BLOCK)
#define CHILD_UNBLOCK	child_block(SIG_UNBLOCK)
#endif
#endif

#endif
