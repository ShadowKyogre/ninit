int str_diff(const char* a, const char* b) /*EXTRACT_INCL*/ {
  unsigned int u=0;
  char ch=0;
  for (;; u++) {
    ch = a[u]-b[u];
    if (ch) break;
    if (a[u]==0) break;
  }
  return ch;
}

