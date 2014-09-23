unsigned int atoulong(char *s) /*EXTRACT_INCL*/ {
  register unsigned int dest=0;
  register unsigned char c;

  while ((c=*s-'0')<10) { ++s; dest=dest*10 + c; }
  return dest;
}
