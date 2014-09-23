/* 0 <= mem.l <= mem.r <= mem.a */

/* add process to data structure, return index or -1 */
static int addprocess(void *p, char *service) {
  unsigned int root_len, serv_len = str_len(service) + 1;
  void *name;
  struct process *pr;

  root_len = mem.l + PROCESS_SIZE;
  if (root_len + serv_len + 32 > mem.r) 
    { write(1,"init: out of memory\n",20);  return -1; }

  mem.r -= serv_len;
  ++maxprocess;

  pr = root;
  root = mem.x + (mem.r & ~15) - root_len;
  byte_copy(root, mem.l, pr);

  pr = root + maxprocess;
  byte_copy(pr, PROCESS_SIZE, p);

  mem.l += PROCESS_SIZE;
  name = mem.x + mem.r;
  byte_copy(name, serv_len, service);

  pr->name = process_name_set;
  next_cron = 0;
  return maxprocess;
}
