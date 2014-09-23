void byte_set(const void* dst, unsigned int len, char ch) /*EXTRACT_INCL*/ {
  char *d=(char*)dst;
  while (len) {
    --len;
    d[len] = ch;
  }
}
