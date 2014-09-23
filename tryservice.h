
static int tryservice(char *home, char *service, char *type, char *other,
		void *buffer) {
  int r;
  open_inout(home);
  errmsg_puts(-1,0);
  errmsg_puts(-1,type);
  errmsg_put(-1,service,str_len(service)+1);
  if (other) 
    errmsg_puts(-1,other);
  errmsg_puts(infd,0);

  r=read(outfd,buffer, 4*sizeof(struct process));
  close(infd); close(outfd);
  return r;
}
