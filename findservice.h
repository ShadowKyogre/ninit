#ifdef INIT_PROGRAM
static void circsweep() {
  int i;
  for (i=0; i<=maxprocess; i++)
    root[i].pr_circular=0;
}
#endif


/* return index of service in process data structure or -1 if not found */
static int findservice(char *service) {
  int i;

  for (i=0; i<=maxprocess; ++i) {
#if defined (INIT_PROGRAM) && ! defined (INIT_BLACK)
    char *x=root_name(i), *y=service;
    while (*x == *y && *x) { x++; y++; }
    if (*x==*y) return i;
#else
    if (!str_diff(root_name(i), service)) return i;
#endif
  }
  return -1;
}
