void byte_copy(void *to, unsigned int n, const void *from) /*EXTRACT_INCL*/ {
  char *d=(char*)to;
  char *s=(char*)from;
  unsigned int k=0;
  for (; k<n; k++) d[k] = s[k];
}

