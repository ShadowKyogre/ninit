#include "djb/buffer.h"

buffer b_outfd = BUFFER_INIT(read, -1, 0, BUFFER_TMP_LEN);
static unsigned short maxhistory;
static unsigned short *history;

#ifdef UNKNOWN_PROCESS_SIZE
static unsigned int process_size;
#else
#define process_size PROCESS_SIZE
#endif

#define MAX_SERVICE_NAME 256

/* read from outfd string or constant lenght*/
int read_stringb(char *s,int len) { 
  int count=0;
  while (1) {
    if (buffer_getc(&b_outfd, s+count) <=0 ) break;
    if (len==-1 && s[count]==0) return count;
    ++count;
    if (count==len) return count;
    if (len==-1 && count==MAX_SERVICE_NAME) break;
  }
  die_xx("error getting data");
  return -1;
}

/* buf[??] is defined in the main program */
void get_services() {
  int j;
  char x[MAX_SERVICE_NAME + PROCESS_SIZE], *tmp, *name="";

  if (root) return;
  b_outfd.x = buf;
  b_outfd.fd = outfd;
  x[0] = 'D', x[1]='0';
  if (do_update) x[1]='1';    /* XXX D1 set doupdate */
  write(infd,x,2);

  /* 1. names */
  while (1) {
    j = read_stringb(x,-1);
    if (j==0) break;
    if ((int)mem.r <= j + PROCESS_SIZE) die_xx("Out of memory");
    mem.r -= ++j;
    name = mem.x + mem.r;
    byte_copy(name, j, x);
    ++maxprocess;
  }

  /* 2. history */
  read_stringb(x, sizeof(unsigned short) - 1);
  maxhistory = (unsigned char)x[0];
  j =  sizeof(unsigned short) * maxhistory;

#ifdef UNKNOWN_PROCESS_SIZE
  if (process_size && maxprocess >= 0) {
    process_size -= (mem.a-mem.r) + j + sizeof(unsigned short);
    process_size /= (maxprocess+1);
    byte_zero(mem.x, mem.r);
  } else
    process_size = PROCESS_SIZE;
#endif
  
  mem.r -= (mem.r % 8);
  if (j) {
    if ((int)mem.r < j) die_xx("Out of memory");
    mem.r -= j;
    tmp = mem.x + mem.r;
    read_stringb(tmp,j);
    history = (unsigned short *)tmp;
  }

  /* 3. processes */
  mem.l = (maxprocess+1) * PROCESS_SIZE;
  mem.r -= (mem.r % 16);
  if (mem.l >= mem.r) die_xx("Out of memory");
  tmp = mem.x + mem.r - mem.l;
  root = (struct process *)tmp;

  for (j=0; j<=maxprocess; j++) {
    read_stringb(tmp, process_size);
    tmp += PROCESS_SIZE;
    root[j].pr_circular = 0;
    root[j].name = alloc_name_set(name);
    name += str_len(name) + 1;
  }
}

#undef MAX_SERVICE_NAME
