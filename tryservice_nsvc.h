#ifdef CLEAN_SERVICE
static char *cleanservice(char* service) {
  char* x;
  size_t len = str_len(initroot);
  if (!byte_diff(service,len,initroot) && service[len] == '/')
    service += len+1;
  x=service+str_len(service);
  while (x>service && x[-1]=='/') --x;
  x[0]=0;
  return service;
}
#endif


/* return nonzero if error */
static int tryservice(char *service, char ch) {
  char *y=buf, *x=service;
  int len;

#ifdef CLEAN_SERVICE
  x = cleanservice(x);
#endif

  *y++ = ch;
  y += str_copyn(y,x,240);
  *y++ = 0;
  if (nsvc_other) y += str_copyn(y, nsvc_other, 15);

  write(infd,buf,y-buf);
  len=read(outfd,buf,BUFFER_TMP_LEN);

  if (ch != 's') return (len != sizeof(struct process));
  return (len!=1 || buf[0]!='1');
}
