void byte_copyr(void* dst, unsigned int n, const void* src) /*EXTRACT_INCL*/ {
  char *d=(char*)dst;
  char *s=(char*)src;
  while (n) {
    --n;
    d[n] = s[n];
  }
}
