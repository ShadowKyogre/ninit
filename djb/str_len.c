unsigned int str_len(const char * s) /*EXTRACT_INCL*/ {
  unsigned int len=0;
  while (s[len]) ++len;
  return len;
}
