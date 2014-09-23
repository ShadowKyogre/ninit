
#include <stdio.h>
#include <unistd.h>
#include "../initreq.h"

int main() {
  struct init_request req; 
  while (1) {
    if (read(0, &req, sizeof(req)) != (int)sizeof(req)) return 1;

    printf("magic:\t%x\n" "cmd:\t%d\n" "runlevel:\t%d=%c\n"
	   "sleeptime:\t%d\n"
	   "reserved:\t%s\n"
	   "exec_name:\t%s\n"
	   "host:\t%s\n"
	   "term_type:\t%s\n"
	   "tty_id:\t%s\n"
	   "gen_id:\t%s\n"
	   "\n",
	   req.magic, req.cmd, req.runlevel, req.runlevel,
	   req.sleeptime,
	   req.i.bsd.reserved,
	   req.i.bsd.exec_name,
	   req.i.bsd.host,
	   req.i.bsd.term_type,
	   req.i.bsd.tty_id,
	   req.i.bsd.gen_id
	   );
  }
  return 0;
}
