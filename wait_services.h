static void wait_services(char **argv, char *home) {
  char *s;
  struct process pr[6];
  int r,count;
  unsigned long ul[2], ug[2] = { RUN_MAXWAIT, 1 };

  read_ulongs("maxwait", ug, 2);
  if (ug[1]==0) ug[1] = 1;

  for (; *argv; argv++) {
    s = *argv;
    if (*s == 0) continue;

    r = str_chr(s,':');
    if (s[r]) {
      s[r] = 0;
      r = scan_ulongs(s+r+1,ul,2, scan_ulong,':',&count);
    } else r=0;

    if (r<1) ul[0] = ug[0];
    if (r<2) ul[1] = ug[1];
    count = ul[0];

    /* we have to ask EACH TIME for PID.   pidfile changes it */
    while (1) {
      r = tryservice(home,s,"p", 0, pr);
      if (r == sizeof(pr[0]))
	if (pr->pid != 1) {
	  nano_sleep(ul[1], 0);
	  if (ul[0]==0) continue;
	  count -= ul[1];
	  if (count > 0) continue;
	}
      break;
    }
  }
}
