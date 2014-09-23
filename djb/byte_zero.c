void byte_zero(void *to, unsigned int k) /*EXTRACT_INCL*/ {
  char *d = (char *)to;
  while (k) {
    k--;
    d[k] = 0;
  }
}
