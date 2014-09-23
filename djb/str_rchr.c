unsigned int str_rchr(const char *in, char needle) /*EXTRACT_INCL*/ {
  char ch;
  unsigned int u=0, found = (unsigned int)-1;
  for (;; u++) {
    if ((ch=in[u])==0) break; 
    if (ch==needle) found=u;
  }
  return (found != (unsigned int)-1) ? found : u;
}
