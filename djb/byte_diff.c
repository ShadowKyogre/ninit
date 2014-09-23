int byte_diff(const void* a, unsigned int len, const void* b) /*EXTRACT_INCL*/ {
  char *x=(char *)a;
  char *y=(char *)b;
  unsigned int u=0;
  char ch=0;
  for (; u<len; u++) {
    ch = x[u] - y[u];
    if (ch) break;
  }
  return ch;
}
