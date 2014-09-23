int str_diffn(const char* a, const char* b, unsigned int limit) /*EXTRACT_INCL*/ {
  unsigned int u=0;
  char ch=0;

  for (; u<limit; u++) {
    ch = a[u] - b[u];
    if (ch) break;
    if (a[u]==0) break;
  }
  return ch;
}
