/* pututmpid.c

   usage: pututmpid [-w] c2 child arg... 
   usage: pututmpid reboot
   usage: pututmpid halt

   c2 is the same entry as in /etc/inittab
   c2:2345:respawn:/sbin/fgetty tty2
   see the end of this file how it works
*/

#include <unistd.h>
#include <fcntl.h>
#include <utmp.h>
#include <time.h>
#include <sys/utsname.h>
#include "ninitfeatures.h"
#include "utmp_defs.h"

int main(int argc, char **argv) {
  struct utmp u;
  char *utid,*argv0,*x;
  char flagwtmp=0;
  int fd; 
  off_t pos=0;

  if (argv[1] && argv[1][0] == '-' && argv[1][1] == 'w') 
    {++argv; --argc; flagwtmp=1;}

  if (argc >=3) {
    char **e, *env[2] = {0,0};
    size_t len = str_len(argv[1]);
    utid = argv[1];
    if (len>4) utid += len-4; 
    argv += 2;

    fd=open(_PATH_UTMP, O_RDWR);
    while (utmp_io(fd, &u, F_RDLCK)) {
      if (!str_diffn(u.ut_id, utid, sizeof(u.ut_id))) {
	if (u.ut_type == USER_PROCESS) {
	  u.ut_tv.tv_sec=time(0);
	  u.ut_type=DEAD_PROCESS;
	  /* byte_zero(&u.ut_user, sizeof(u.ut_user));
	     byte_zero(&u.ut_host, sizeof(u.ut_host)); */
	  do_wtmp(&u);
	}
	break ; 
      }
      pos += sizeof(struct utmp);
    }
    
    byte_zero(&u,sizeof(u));
    str_copyn(u.ut_id, utid, sizeof(u.ut_id)); 
    u.ut_pid=getpid();
    u.ut_tv.tv_sec=time(0);
    u.ut_type=INIT_PROCESS;
    
    if (fd>=0 && lseek(fd,pos,SEEK_SET) == pos)
      utmp_io(fd,&u,F_WRLCK);
    close(fd);
    
    if (flagwtmp) do_wtmp(&u);
    
    argv0 = argv[0];
    x = argv[0];
    x += str_rchr(x,'/');
    if (x[0]) argv[0] = x+1;
    else argv[0] = argv0;

    for (e=environ; *e; e++) 
      if (!str_diffn(*e,"TERM=",5)) {*env=*e; break;}

    execve(argv0, argv, env);
    write(2,"unable to exec: ",16);
    write(2,argv0,str_len(argv0));
    write(2,"\n",1);
    return(100);
  }

  /******************************************************/
  if (argc == 2) {
    struct utsname uname_buf;
    fd = -1;
    switch (argv[1][0]) {
    case 'r':
      fd=open(_PATH_UTMP, O_RDWR);
      while (utmp_io(fd, &u, F_RDLCK)) {
	/* if already exist do nothing */
	if (u.ut_type == BOOT_TIME) { close(fd); return 111; }
	pos += sizeof(struct utmp);
      }
      
      byte_zero(&u,sizeof(u));
      str_copy(u.ut_user,"reboot");
      str_copy(u.ut_line,"~");
      u.ut_type=BOOT_TIME;
      break;
    case 's':
    case 'h':
      str_copy(u.ut_user,"shutdown");
      u.ut_type=RUN_LVL;
      str_copy(u.ut_line,"~~");
      break;
    default:
      goto usage;
    }
    
    str_copy(u.ut_id,"~~");
    u.ut_pid=0;
    u.ut_tv.tv_sec=time(0);
    
    /* Put the OS version in place of the hostname */
    if (uname(&uname_buf) == 0)
      str_copyn(u.ut_host, uname_buf.release, 32); /* XXX overflow ? - Very slim, see uname(2) and utmp(5) */
 
    if (fd>=0 && lseek(fd,pos,SEEK_SET) == pos)
      utmp_io(fd,&u,F_WRLCK);
    close(fd);
    
    do_wtmp(&u);
    return(0);
  }

 usage:
  write(2,"usage: pututmpid [-w] ut_id child arg...\n\
       pututmpid reboot\n\
       pututmpid halt\n",87);
  return(100);
}

/* 
    pututmpid:  
        if exist an entry in utmp with: ut_id=c2 ut_type=USER_PROCESS
          then put: ut_id=c2 ut_type=DEAD_PROCESS  in wtmp

        always put: ut_id=c2 ut_type=INIT_PROCESS  in utmp
    if -w flag put: ut_id=c2 ut_type=INIT_PROCESS  in wtmp 

    getty and login chanegs /var/run/utmp: 
    getty:          ut_id=c2 ut_type=LOGIN_PROCESS
    login:          ut_id=c2 ut_type=USER_PROCESS
*/
