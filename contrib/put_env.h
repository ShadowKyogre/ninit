int put_env(const char *string) {
  unsigned int len, envc, remove=0;
  char **ep;
 
  len=str_chr(string,'=');
  if (string[len]==0) remove=1;
  for (envc=0, ep=environ; *ep; ++ep) {
    if (!byte_diff(string, len, *ep) && (*ep)[len]=='=') {
      if (remove) {
	for (; ep[1]; ++ep) ep[0]=ep[1];
	ep[0]=0;
	++env_free;
	return 0;
      }
      *ep=(char *)string;
      return 0;
    }
    ++envc;
  }
  if (remove==0) {
    if (env_free==0) return -1;
    environ[envc++]=(char*)string;
    environ[envc]=0;
    --env_free;
  }
  return 0;
}
