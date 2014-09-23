unsigned int str_chr(const char *in, char needle) /*EXTRACT_INCL*/ {
  unsigned int u=0;
  char ch;
  while ((ch=in[u]) && ch != needle) u++;
  return u;
}
