unsigned int str_copyn(char *out,const char *in, unsigned int len) /*EXTRACT_INCL*/ {
  unsigned int k=0;
  while (k<len && (out[k]=in[k])) k++;
  if (k<len) out[k] = 0;
  return k;
}
