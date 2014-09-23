unsigned int str_copy(char *out,const char *in) /*EXTRACT_INCL*/ {
  unsigned int len=0;
  while ((out[len]=in[len])) len++;
  return len;
}
